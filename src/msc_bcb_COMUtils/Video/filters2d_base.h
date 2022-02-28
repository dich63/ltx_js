#pragma once
//#include "filters2d_base.h"

#include <vector>
#include <algorithm>


template <int Ver=0>
struct  filter_base_t
{

	enum    
	{
		alpha_denom_dig = 12 ,
		alpha_denom =1<<alpha_denom_dig

	};

	typedef unsigned char byte_t;
	typedef unsigned char uint8_t;
	typedef signed short  int16_t;
	typedef unsigned short uint16_t;
	typedef unsigned int uint32_t;

	struct frgba_t
	{

		float r;
		float g;
		float b;
		float a;

	};

	template <class B>
	inline void blend_byte(const B src,B& dest,const uint32_t a,uint32_t& buffer)
	{  		
		/*if(a==alpha_denom) dest=buffer=src;
		else
		if (a==0) dest=buffer; 
		else */

		{
		uint32_t v;
	    v= ((alpha_denom-a)*buffer)>>alpha_denom_dig;
		v= v+a*(src);
		buffer=v; 
		dest=v>>alpha_denom_dig;
		}

	};





	typedef byte_t* pbyte_t;
	int width;
	int height;
	int src_byte_width;
	int dest_byte_width;
	int nthread;
	//std::vector<HANDLE> mutexes;
	std::vector<int> b_rows;
	std::vector<int> e_rows;




	struct barrier_t
	{
		volatile LONG refcount;
		HANDLE hevent;
		barrier_t(int c=1,HANDLE _hevent=CreateEvent(0,1,0,0)):refcount(c),hevent(_hevent){
			//HANDLE hp=GetCurrentProcess();
			//DuplicateHandle(hp,_hevent,hp,&(hevent=0),0,0,DUPLICATE_SAME_ACCESS);
		};
		~barrier_t()
		{

			if(hevent)
			{
				//SetEvent(hevent);
				CloseHandle(hevent);
			}

		}
		inline LONG AddRef()
		{
			LONG l;
			l=InterlockedIncrement(&refcount);
			return l;
		}
		inline LONG Release()
		{
			LONG l;
			l=InterlockedDecrement(&refcount);
			if(l==0) SetEvent(hevent);
			return l;
		}
		inline LONG RefCount()
		{
			LONG l;
			l=InterlockedCompareExchange(&refcount,0,0);
			return l;
		}

		inline int wait()
		{
			return WaitForSingleObject(hevent,INFINITE);

		}


		inline int wait_fast(int spin=10000)
		{
			LONG l;
			while(((l=RefCount())>1)&&(--spin>0)) {};
			if(l>1) 
			{
				if(Release()>0) return wait();
				
			}

			return -1;						
		}


	};

	/*
	struct barrier_waiter_t
	{

	HANDLE hevent;
	barrier_waiter_t():hevent(CreateEvent(0,1,0,0)){};


	~barrier_waiter_t(){ CloseHandle(hevent);}
	int wait()
	{
	return WaitForSingleObject(hevent,INFINITE);

	}



	};

	*/

	struct thread_node_t{
		filter_base_t *owner;
		int b_row,e_row;
		void* psrc;
		void* pdest;
		int n;
		barrier_t* pbarrier;

		thread_node_t(barrier_t* _pbarrier,int node,filter_base_t* _owner,void* _psrc,void* _pdest)
			:n(node),owner(_owner),psrc(_psrc),pdest(_pdest)
		{
			if(pbarrier=_pbarrier)  pbarrier->AddRef();
		};


		void make_frame() {


			int b_row=owner->b_rows[n],e_row=owner->e_rows[n];
			owner->make_frame_node((byte_t*)psrc,(byte_t*)pdest,b_row,e_row);

		};
		~thread_node_t()
		{
			if(pbarrier)  pbarrier->Release();
		}

	};

	inline int get_NumberOfProcessors() 
	{
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		return si.dwNumberOfProcessors;

	}


	virtual void make_frame_node(byte_t* psrc,byte_t* pdest,int b_row,int e_row)=0;


	static		long __stdcall s_proc( thread_node_t* p)
	{
		p->make_frame();
		delete p;
		return 0;
	}



	filter_base_t(int _nthread,int _width,int _height,int _src_byte_width=0,int _dest_byte_width=0)
		:nthread(_nthread),width(_width),height(_height),src_byte_width( _src_byte_width),dest_byte_width(_dest_byte_width)
	{
		if(nthread<=0) nthread=get_NumberOfProcessors() ;
		if(!src_byte_width) src_byte_width=width;
		if(!dest_byte_width) dest_byte_width=src_byte_width;
		if(nthread!=1)
		{
			/*
			mutexes.resize(nthread);
			for(int k=0;k<nthread;k++)
				mutexes[k]=CreateMutex(0,0,0);
				*/

			b_rows.resize(nthread);
			e_rows.resize(nthread);
			int nh=height/nthread;
			for(int k=0;k<nthread-1;k++)
			{				
				b_rows[k]=k*nh;
				e_rows[k]=(k+1)*nh-1;
			}

			//int hh=height-(nthread-1)*nh;				
			b_rows[nthread-1]=(nthread-1)*nh;
			e_rows[nthread-1]=height-1;




		}

	};

	~filter_base_t(){

		/*
		if(nthread!=1)
			for(int k=0;k<nthread;k++)
				CloseHandle(mutexes[k]);
				*/


	}

	virtual bool make_frame(void* psrc,void* pdest)
	{
		if (nthread==1) 
		{
			make_frame_node((byte_t*)psrc,(byte_t*)pdest,0,height-1);
			return true;
		}
		else
		{
			bool f=true;

			barrier_t barrier;

			for(int k=0;k<nthread;k++)
			{				
				thread_node_t* pt=new thread_node_t(&barrier,k,this,psrc,pdest);
				if(k)
				{
					if(!QueueUserWorkItem((LPTHREAD_START_ROUTINE)&s_proc,pt,WT_EXECUTEDEFAULT)){ 
						delete pt;
						f=false;break;
					}
				}
				else 
				{
                         s_proc(pt);
				}
			}

			int wf=barrier.wait_fast();
			
			return f;

		}


	};


};

template <int Ver=0>
struct  BMP_io_t{

	enum{
		hsize=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)   
	};

#pragma pack(push)
#pragma pack(1)

	struct bmp_file_t
	{
		BITMAPFILEHEADER bf;
		BITMAPINFOHEADER bih;

	};

#pragma pack(pop)



	typedef unsigned char byte_t;
	typedef unsigned short uint16_t;
	typedef byte_t* pbyte_t;

	int byte_width;
	std::vector<byte_t> vbuf;
	BMP_io_t* pimage2;

	inline static int get_byte_width(int Width,int BPP){ return  ((WORD)((((DWORD)(Width) * (DWORD)(BPP) + 31) >> 5)) << 2) ;}
	inline static int get_byte_width(BITMAPINFOHEADER* bih)	{ return  get_byte_width(bih->biWidth,bih->biBitCount);}


	
	inline  int width()
	{
		return get_buf<bmp_file_t>()->bih.biWidth;
	}
	inline  int height()
	{
		return get_buf<bmp_file_t>()->bih.biHeight;
	}

	inline  int bpp()
	{
		return get_buf<bmp_file_t>()->bih.biBitCount;
	}

	inline  int get_byte_width()
	{
		return get_byte_width(&get_buf<bmp_file_t>()->bih);
	}

	template <class N>
	inline N* get_buf(){ return (N*)((vbuf.size())?&vbuf[0]:0);};

	inline long get_buf_size(){ return vbuf.size();};

	inline	byte_t* get_bits(){

		bmp_file_t* pbm=get_buf<bmp_file_t>();
		return ((byte_t*)pbm)+pbm->bf.bfOffBits;
	}

	


	int save(wchar_t* fn)
	{
		bmp_file_t* pbm=get_buf<bmp_file_t>();
		return save(fn,pbm);

	}

	BMP_io_t():pimage2(0){};
	~BMP_io_t(){
		delete pimage2;
	}

	BMP_io_t(int width,int height,int bps=24,int byte_width=0,void* ptr=0):pimage2(0)
	{
		create( width, height,bps,byte_width,ptr);

	}

	bmp_file_t* create(int width,int height,int bps=24,int _byte_width=0,void* ptr=0)
	{

		BITMAPINFOHEADER bi={sizeof(BITMAPINFOHEADER),width, height,1,bps};


		int bw=get_byte_width(&bi);
        byte_width=(_byte_width)?_byte_width:bw;

		int sizim=(bw*height);
		int sizfile=sizim+hsize;
		vbuf.resize(sizfile);
		bmp_file_t* pbm=get_buf<bmp_file_t>();

		BITMAPFILEHEADER bf={*((WORD*)"BM"),sizfile,0,0,hsize};
		/*

		pbm->bf.bfSize=sizfile;
		pbm->bf.bfOffBits=hsize;
		*/
		pbm->bf=bf;
		pbm->bih=bi;

		if(ptr)
		{

			byte_t* pd= ((byte_t*)pbm)+pbm->bf.bfOffBits;
			byte_t* ps=(byte_t*)ptr;
			int szr=(width*bps)/8;
			for(int y=0;y<height;y++)
			{
				memcpy(pd,ps,szr);
				pd+=bw;
				ps+=byte_width;
			}	   
		}

		return pbm;

	};


	static int save(wchar_t* fn,void* ptr,int width,int height,int bps=24,int byte_width=0)
	{
		BMP_io_t b;
		return save(fn, b.create(width,height,bps,byte_width,ptr));       
	}


	static    int save(wchar_t* fn,BITMAPFILEHEADER*pbm)
	{
		return save(fn,(bmp_file_t*)pbm); 
	}
	static	int save(wchar_t* fn,bmp_file_t* pbm,byte_t* pbits=0)
	{
		FILE* hf=_wfopen(fn,L"wb");
		if(!hf) return 0;
		
		int res=save(hf,pbm,pbits);


/*
		int size=pbm->bf.bfSize; 
		if(size==fwrite(pbm,1,size,hf))
			res=size;
			*/

		fclose(hf);

		return res;

	}

	static	int save(FILE* hf,bmp_file_t* pbm,byte_t* pbits=0)
	{
		
		int res=0;


		int size=pbm->bf.bfSize; 
		if(size==fwrite(pbm,1,size,hf))
			res=size;
		return res;

	}




	bmp_file_t* load(wchar_t* fn)
	{
		FILE* hf=_wfopen(fn,L"rb");
		if(!hf) return 0;

		/*
		
		vbuf.resize(hsize);
		bmp_file_t* pbmi=0;

		if(hsize==fread(get_buf<char>(),1,hsize,hf))
		{
			pbmi=get_buf<bmp_file_t>();
			int size=pbmi->bf.bfSize;
			vbuf.resize(size);
			int cb=size-hsize;
			pbmi=(cb==fread(get_buf<char>()+hsize,1,cb,hf))?get_buf<bmp_file_t>():0;
		}
		*/
        bmp_file_t* pbmi=load(hf);
		fclose(hf);
		return pbmi;

	}

	bmp_file_t* load(FILE* hf)
	{	
		
		vbuf.resize(hsize);
		bmp_file_t* pbmi=0;

		if(hsize==fread(get_buf<char>(),1,hsize,hf))
		{
			pbmi=get_buf<bmp_file_t>();
			int size=pbmi->bf.bfSize;
			vbuf.resize(size);
			int cb=size-hsize;
			pbmi=(cb==fread(get_buf<char>()+hsize,1,cb,hf))?get_buf<bmp_file_t>():0;
		}
		if(pimage2)  pimage2->load(hf);
		return pbmi;

	}


};
