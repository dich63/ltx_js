#pragma once
#include <windows.h>
#include "dummy_dispatch_aggregator.h"
#include "IShared_Image.h"

#include "console_previewer_shared.h"
#include "singleton_utils.h"
#include "console_previewer.h" 
#include "free_com_factory.h" 
#include "yuy2/bmpYUY2.h" 
#include "decay_filter.h" 
#include "ipc_ports/callback_context_arguments.h"


#include <vector>
#include <utility>
#include <algorithm>
#define KB (1024)
#define MB (KB*KB)

#define s_m(s) L#s
#define update_arguments(zz,x) zz[s_m(x)].update(x)
#define  update_args(a) update_arguments(args,a)


template <int _capacity=64*MB,bool freemarshall=1>
struct ISharedImage_impl : public ISharedImage
{

	
	typedef typename DummyIDispatchAggregator_t<ISharedImage_impl,freemarshall> dummy_dispatch_t;


	struct in_t{
     struct crop_t{
		 int enabled;
		 int left,top,width,height;

	 } ;

	 crop_t crop;

	} ;

	typedef typename in_t::crop_t crop_t;

	 in_t in;

	 int leftoff,topoff;


	struct frame_data_t
	{
		byte_t* ptr;
		mutex_ref_t* pmutex;
		int mode;
	};

	struct locker_frame_t
	{
		ISharedImage_impl* p;
		byte_t *ptr;
		locker_frame_t(ISharedImage_impl* _p):p(_p),ptr(0)
		{
			_p->LockFramePtr(&ptr,0);
		}
		~locker_frame_t()
		{
          p->UnlockFramePtr(ptr);
		}

		inline byte* get(){ return ptr;}

	};









    static   HRESULT CreateInstance(ISharedImage** ppObj)
	{        
		if(!ppObj) return E_POINTER;
			*ppObj=new ISharedImage_impl;
	   return S_OK;
	}

	

  	ISharedImage_impl():refcount(1),fmt(0),compression(-1),state(0),flip(0),flipV(0),flipH(0),wfmt(0),fmtout(0),fmtget(0),state_crop(0){
		memset(&in,0,sizeof(in));
		free_com_factory::DLL_AddRef(&CreateInstance);

	};
	~ISharedImage_impl(){
		free_com_factory::DLL_Release(&CreateInstance);
	};



	// IUnknown Interface
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject)
	{ 
		if(!ppvObject) return E_POINTER;
		HRESULT hr=E_NOINTERFACE;

		if (iid == __uuidof(IUnknown))			
		{
			*ppvObject = static_cast< ISharedImage*>(this);
			AddRef();
			hr=S_OK;
		} 
		
		ddisp(this,iid,ppvObject,hr);
		return hr;
	}

	virtual ULONG STDMETHODCALLTYPE AddRef(void) 
	{ 
		return InterlockedIncrement((LONG volatile *)&refcount); 
	}

	virtual ULONG STDMETHODCALLTYPE Release(void) 
	{
         LONG l= InterlockedDecrement((LONG volatile *)&refcount); 
		 if(l==0) delete this;
		return l;
	}

	// ISharedImage Interface
public:

	inline BITMAPFILEHEADER* prepare_fmt_out(BITMAPFILEHEADER* bfh_in)
	{
		int cb,ch;
		if(bfh_in==0) return bfh_in;
          BITMAPINFOHEADER& bih=*((BITMAPINFOHEADER*)(bfh_in+1));

		  int compression=bih.biCompression;


		  bbh=*((BB_t*)bfh_in);

		  cb=BYTESPERLINE(bbh.bi.biWidth,bbh.bi.biBitCount)*bbh.bi.biHeight+sizeof(BB_t);
		  bbh.bf.bfSize=cb;
		  bbh.bf.bfOffBits=0;

		
		if((fmtout==0)||((fmtout==2)&&(*((DWORD*)"YUY2")==compression))||((fmtout==1)&&(0==compression)))
			return bfh_in;
		else
		{

			BB_t BB={{*PWORD("BM")},{sizeof(BITMAPINFOHEADER)}};
			BB.bi.biPlanes=1;

			if(fmtout==1) BB.bi.biBitCount=24;
			else if(fmtout==2) {
				BB.bi.biBitCount=16;
				BB.bi.biCompression=*PDWORD("YUY2");
			}
			BB.bi.biWidth=bih.biWidth;
			BB.bi.biHeight=bih.biHeight;
			
			
			//bbh=BB;

            cb=BYTESPERLINE(BB.bi.biWidth,BB.bi.biBitCount)*BB.bi.biHeight+sizeof(BB);
			BB.bf.bfSize=cb;
			buf2.resize(cb+32);
			ch=sizeof(BB);
			ch=(ch+0xF)&(~0xF);
			BB.bf.bfOffBits=ch;
			char * p=buf2;
			pbbout=(BB_t*)p;
			*pbbout=BB;
			return &(pbbout->bf);
			
		}


	}
	virtual HRESULT Init(const wchar_t* mapkey,void* pbmp_blank_image=0,int nframes=-1,int capacity=-1)
	 {
           BITMAPFILEHEADER* pfblank=prepare_fmt_out((BITMAPFILEHEADER*)pbmp_blank_image);
           BITMAPFILEHEADER* pfh=bmpmap.open_key(mapkey);
		   //inline BYTE*  reset(BITMAPINFO& bi,int _nframes=1,DWORD capacity=(16*1024*1024))
		   //nframes=(nframes<0)?4:nframes;
		   //nframes=(nframes)?nframes:1;
		   nframes=(nframes>0)?nframes:1;
           
		   capacity=(capacity>0)?capacity:_capacity;
		    
			last_fb=0;
		   if(pfblank)
		   {
			   if(pfblank->bfType!=*((WORD*)"BM"))
				     return E_INVALIDARG;
			   BITMAPINFO* pbmi=(BITMAPINFO*)(pfblank+1);
			   void* ptr;
			   ptr=bmpmap.reset(*pbmi,nframes,capacity);
			   if(!ptr) return E_FAIL;
              
		   }
		   else if(!pfh) return E_ACCESSDENIED;

		    compression=bmpmap.header<BITMAPINFOHEADER>().biCompression;

		   //std::vector<mutex_ref_t> vframe_mutexes;
		   //return 

		   for(int n=0;n<bmpmap.nframes;n++)
		   {
              
			   mutex_ref_t* pm=&bmpmap.vframe_mutexes[n];
			   BMPMap::frame_info_t* pfi=bmpmap.pframe_info(n);
			   frame_data_t fd={pfi->pbits,pm};
			   vptr_mutexes.push_back(fd);


		   }
		   state=1;

		   return S_OK;

	};

	virtual HRESULT InitA(const char* mapkey,void* pbmp_blank_image=0,int nframes=-1,int capacity=-1)
	{
		return Init(char_mutator<CP_THREAD_ACP>(mapkey),pbmp_blank_image,nframes,capacity);
	}

#pragma pack(push)
#pragma pack(1)
	struct BB_t
	{
		BITMAPFILEHEADER bf;
		BITMAPINFOHEADER bi;

	} ;
#pragma pack(pop)

   virtual HRESULT Init2A(const char* key, int width=0,int height=0,int fmt=0,int nframes=0,int capacity=-1)
   {
	   return Init2(char_mutator<CP_THREAD_ACP>(key),width,height,fmt,nframes,capacity);
   }
	virtual HRESULT Init2(const wchar_t* key, int width,int height,int _wfmt=0,int nframes=0,int capacity=-1)
	{
		HRESULT hr;
       wfmt=_wfmt;
		nframes_=nframes;
		capacity_=capacity;
		

		if(!key) key=vkey;			 
		else
		{ 
			vkey.printf(L"%s",key);		
		  if((width<=0)||(height<=0)) return S_OK;
		   
	    }
		


		BB_t BB={{*PWORD("BM")},{sizeof(BITMAPINFOHEADER)}};
		BB.bi.biPlanes=1;

		if(wfmt==0) BB.bi.biBitCount=24;
		else if(wfmt==1) BB.bi.biBitCount=32;
		else if(wfmt==2) {
			BB.bi.biBitCount=16;
			BB.bi.biCompression=*PDWORD("YUY2");
		}
		else return E_FAIL;
		BB.bi.biWidth=width;
		BB.bi.biHeight=height;
		return hr=Init(key,&BB,nframes,capacity);


	}


#pragma pack(push)
#pragma pack(1)

	template <int N>
	struct cN_t
	{
		char p[N/8];
	};
#pragma pack(pop)
    
    template <class C> 
	void reverse_copy_images(C* pd,C* ps,int h,int w,int lsb,int ldb)
	{
		typedef char* pchar_t;
		typedef C* pC_t;
		for(int n=0;n<h;n++)
		{
			std::reverse_copy(ps,ps+w,pd);  
			ps=pC_t(pchar_t(ps)+lsb);
			pd=pC_t(pchar_t(pd)+ldb);
            
		}

	}
	template <class C> 
	void reverse_copy_imagesYUY2(C* pd,C* ps,int h,int w,int lsb,int ldb)
	{
		typedef char* pchar_t;
		typedef C* pC_t;
		
		for(int n=0;n<h;n++)
		{
			//std::reverse_copy(ps,ps+w,pd);  
			for(pC_t s=ps,d=pd+w-1;s<ps+w;s++,d--)
			{
				*d=*s;
				std::swap(d->p[0],d->p[2]);

				/*
				register C t=*s;
				register char c;
				c=t.p[0];
				t.p[0]=t.p[1];
				t.p[1]=c;
				

				//std::swap(t.p[0],t.p[2]);
				*d=t;
				*/

				
			}
			ps=pC_t(pchar_t(ps)+lsb);
			pd=pC_t(pchar_t(pd)+ldb);

		}

	}


  HRESULT make_filters(BITMAPFILEHEADER*& pfh,byte_t*& pbits){

	    

         locker_t<filters_t::mutex_t> lock(filters.mutex);
		 if(filters.decay){
			 BITMAPINFOHEADER* psbih=(BITMAPINFOHEADER*)((byte_t*)(pfh)+sizeof(BITMAPFILEHEADER));
			 int lsz=BYTESPERLINE(psbih->biWidth,psbih->biBitCount);
			 pbits=filters.decay.make_mix((psbih->biWidth*psbih->biBitCount)/8,psbih->biHeight,lsz,pbits);
			 }

	  return S_OK;
  }

  HRESULT make_post_filters(BITMAPFILEHEADER*& pfh,byte_t*& pbits){
       locker_t<filters_t::mutex_t> lock(filters.mutex);
	  if(filters.mask)
	  {
		  byte_t invmask=~filters.mask;
		  BITMAPINFOHEADER* psbih=(BITMAPINFOHEADER*)((byte_t*)(pfh)+sizeof(BITMAPFILEHEADER));
		  int lsz=BYTESPERLINE(psbih->biWidth,psbih->biBitCount);
		  int h=psbih->biHeight;
		  int wb=(psbih->biWidth*psbih->biBitCount)/8;
		  byte_t *p=pbits;
		  for(int n=0;n<h;n++,p+=lsz)
		  {
			  
			    for(int m=0;m<wb;m++)
				{
                  p[m]&=invmask;
				}
		  }

	  }
	  if(filters.check)
	  {
		  BITMAPINFOHEADER* psbih=(BITMAPINFOHEADER*)((byte_t*)(pfh)+sizeof(BITMAPFILEHEADER));
          if(psbih->biBitCount!=24) return E_FAIL;
		  int lsz=BYTESPERLINE(psbih->biWidth,psbih->biBitCount);
		  int szim=lsz*psbih->biHeight;
		  if(!bufcheck.size_b())
		  {
           bufcheck.resize(szim);
		   bufcheck2.resize(szim);
		  }
		  int cnz=0;
		  char* pkeep=bufcheck.get();
		  char* pout=bufcheck2.get();
		  int w=psbih->biWidth,h=psbih->biHeight;
		  char* pd=pout,* pb=(char*)pbits,* pk=pkeep;
		  for(int n=0;n<h;n++)
		  {

			  for(int m=0;m<3*w;m+=3)
			  {
                char r=pk[m+0]-pb[m+0];
				char g=pk[m+1]-pb[m+1];
				char b=pk[m+2]-pb[m+2];
				char f=(r|g|b)?255:0;
				if(f) cnz++;
				pd[m+0]=pd[m+1]=pd[m+2]=f;
				
			  }
			  pd+=lsz;
			  pb+=lsz;
			  pk+=lsz;
		  }




		  memcpy(pkeep,pbits,szim);
		  pbits=(byte_t*)pout;
		  double pers=3*double(cnz)/szim;
		  SetConsoleTitleA(v_buf<char>().printf("zeros=%g",pers*100.0));



	  }
	  return S_OK;
  }

	HRESULT make_ptrs(BITMAPFILEHEADER*& pfh,byte_t*& pbits){
		if(pfh)
		{
			if(pfh->bfType!=*((WORD*)"BM")) return E_INVALIDARG;
			if(!pbits)
			{
               BITMAPINFOHEADER* psbih=(BITMAPINFOHEADER*)((byte_t*)(pfh)+sizeof(BITMAPFILEHEADER));
               pbits=(byte_t*)(pfh)+pfh->bfOffBits;
			}
			

		}
		else {
			   
			   if(!pbits) return E_INVALIDARG;
			   //pfh=bmpmap.get_BITMAPFILEHEADER();
			   pfh=&bbh.bf;
		}
	
		return S_OK;		


	}

      HRESULT fmtout_convert(BITMAPFILEHEADER*& pfh ,const byte_t* pbits,byte_t*& ptr,BITMAPINFOHEADER*& pbih)
	 {
		    if(pfh&&(pfh->bfType!=*((WORD*)"BM"))) return E_INVALIDARG;

            BITMAPINFOHEADER* psbih=(BITMAPINFOHEADER*)((byte_t*)(pfh)+sizeof(BITMAPFILEHEADER));
			byte_t* psbits=(pbits)?(byte_t*)pbits:((byte_t*)(pfh)+pfh->bfOffBits);

            if(fmtout==0)
			{
             pbih=psbih;
			 ptr=psbits;
			} 
			else
			{
				
				int cbsz=bbh.bf.bfSize;
				int cbconvert=buf2.size_b();
				//BB.bf.bfSize
				char* pconvert=(char*)pbbout;
				int res;
				pbih=&(pbbout->bi);
				ptr=(byte_t*) (pconvert+pbbout->bf.bfOffBits);
				if(fmtout==1)
					res=BMPYUY2_converter::decompressYUY2(cbsz,(char*)&bbh.bf,&cbconvert,0,0,(char*)psbits,(char*)ptr);					
				else if(fmt==2)
					res=BMPYUY2_converter::compressYUY2(cbsz,(char*)&bbh.bf,&cbconvert,pconvert,0,(char*)psbits);
				else res=0;
				if(!res) return E_FAIL;

				pfh=&pbbout->bf;
				
			}




             return S_OK;
	 }


	virtual HRESULT PushFrame(void* pbmpimage,const byte_t* pcbits=0)
	{
       //  BITMAPINFOHEADER bmpmap.header<BITMAPINFOHEADER>(); 
		if((!pbmpimage)&&(!pcbits)) return E_POINTER;
		BITMAPINFOHEADER* pbmis;
		HRESULT hr=E_FAIL;
		byte_t* ptr=0,*ptrs= (byte_t*) pcbits;
		try
		{
		 BITMAPFILEHEADER* pfh=(BITMAPFILEHEADER*)pbmpimage;
        
        if(state==0)
		{
			if(!pfh) return E_INVALIDARG;
			hr=Init(vkey,pfh,nframes_,capacity_);
			if(FAILED(hr))
				return hr;

		}

           
		if(FAILED(hr=make_ptrs(pfh,ptrs)))			return hr;
         
			 if(hr=make_filters(pfh,ptrs))
				 return hr;
		if(hr=fmtout_convert(pfh,ptrs,ptrs,pbmis))
			 return hr;
		if(hr=make_post_filters(pfh,ptrs))
			return hr;


		/*

		 pbmis=(BITMAPINFOHEADER*)((byte_t*)(pfh)+sizeof(BITMAPFILEHEADER));

		if(pfh&&(pfh->bfType!=*((WORD*)"BM"))) return E_INVALIDARG;

		ptrs=(pbits)?(byte_t*)pbits:((byte_t*)(pfh)+pfh->bfOffBits);
		*/


		

		 
		 frame_data_t* pfd=get_next_and_lock(1);
		 
		 ptr=pfd->ptr;
         byte_t* pd=ptr,*ps=ptrs;
		 int lsb=bmpmap.line_sizeb(),ldb=lsb;

		 BITMAPINFOHEADER& bmid=bmpmap.header<BITMAPINFOHEADER>();

		if(flipV)
		{
           pd+=(bmid.biHeight-1)*lsb;
           ldb=-lsb;   
 
		}
		
		if(flipH)
		{
			int w=bmid.biWidth;
			int h=bmid.biHeight;
			
		      if(1&&(bmid.biCompression==*PDWORD("YUY2")))
			  {
				  if(1)
                 reverse_copy_imagesYUY2( (cN_t<32>*) pd,(cN_t<32>*) ps,h, w/2, lsb, ldb);
				  else{
				 
				 reverse_copy_images( (cN_t<32>*) pd,(cN_t<32>*) ps,h, w/2, lsb, ldb);
			    int sz4=h*w/2;
				cN_t<32>* pd4 =(cN_t<32>*) ptr;
				
				for(cN_t<32>* p=pd4;p<pd4+sz4;p++)						
					std::swap(p->p[0],p->p[2]);
                
					  }
				
			  }
			  else
			  switch(bmid.biBitCount){
				case 32: reverse_copy_images( (cN_t<32>*) pd,(cN_t<32>*) ps,h, w, lsb, ldb);break;
            	case 24: reverse_copy_images( (cN_t<24>*) pd,(cN_t<24>*) ps,h, w, lsb, ldb);break;
			    case 16: reverse_copy_images( (cN_t<16>*) pd,(cN_t<16>*) ps,h, w, lsb, ldb);break;
			    case 8: reverse_copy_images( (cN_t<8>*) pd,(cN_t<8>*) ps,h, w, lsb, ldb);break;
			}
			 
			 
			 

		}
		else  for(int n=0;n<bmid.biHeight;n++)
		 {
           memcpy(pd,ps,lsb);
           pd+=ldb;
		   ps+=lsb;
		 }     
		 hr=0;
			
		}

		catch (...){}
         unlock_by_ptr(ptr);
		return hr;

	};
	virtual HRESULT GetBitmapInfo(BITMAPINFOHEADER* pbmi)
	{
		if(!pbmi) return E_POINTER;
		 *pbmi=bmpmap.header<BITMAPINFOHEADER>();

		return S_OK;
	};

	
inline		frame_data_t* get_next_and_lock(int mode)
	{
			

		    frame_data_t* pfd;

		if(mode==0)
		{
				long n=bmpmap.get_last_index();
				pfd=&vptr_mutexes[n];
				pfd->pmutex->lock();
				pfd->mode=0;
				
				return pfd;
		}
		else 
		{


			int fbcount=vptr_mutexes.size();

			pfd= &vptr_mutexes[last_fb];
			for(int n=0;n<fbcount;n++)
			{	
				last_fb=(++last_fb)%fbcount;
				pfd= &vptr_mutexes[last_fb];
				if(pfd->pmutex->try_lock())
				{
					pfd->mode=1;
					return pfd;
				}
     		}

   			 pfd->pmutex->lock();
			 pfd->mode=1;
			return pfd;
		 }
 	}

inline	HRESULT unlock_by_ptr(byte_t* ptr)
	{
		frame_data_t* pfd;
		int fbcount=vptr_mutexes.size();

       if(ptr)
		for(int n=0;n<fbcount;n++)
		{
			 pfd= &vptr_mutexes[n];
			if(ptr==pfd->ptr) 
			{

              if(make_detach(pfd->mode))	  bmpmap.set_last_index(n);
              pfd->pmutex->unlock();
			  
                
              return S_OK;
			}
		}
		return E_POINTER;

	}

	virtual HRESULT LockFramePtr(byte_t** ppbits,int mode) // 0 -read access, 1- write access
	{
		if(!ppbits) return E_POINTER;
          *ppbits=0;
		  *ppbits=get_next_and_lock(mode)->ptr;
           return S_OK;

	};
	virtual HRESULT UnlockFramePtr(byte_t* pbits)
	{
		return unlock_by_ptr(pbits);
	};

	virtual HRESULT GetFrame(BITMAPFILEHEADER** ppbmpimage,int* pcbimage=0){
		
	         HRESULT hr;
	         if(!ppbmpimage) return E_POINTER;
			 int tmp_0;
			 int& cbimage=(pcbimage)?*pcbimage:tmp_0;
			 BITMAPFILEHEADER*& pbmpimage=*ppbmpimage;


              BITMAPFILEHEADER* pbfh;
			  if(state==0)
			  {
				  if(FAILED(hr=Init(vkey,NULL,nframes_,capacity_)))
					  return hr;
			  }
               
               
			   long cbsz=bmpmap.image_size_b();
			   if(buf0.size_b()<cbsz+16)
				   buf0.resize((cbsz)+16);
			   


			   {
			   	 buf0.resize((cbsz)+16);
			     pbfh=(BITMAPFILEHEADER*)buf0.get();
				 BITMAPFILEHEADER* pdi=bmpmap.get_BITMAPFILEHEADER();
				 if(!pdi) return E_FAIL;
				 int cbh=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
				 memcpy(pbfh,pdi,cbh);
				 ULONG_PTR up=ULONG_PTR((char*)pbfh+cbh);
				 up=(up+0x7)&(~(0x7));
				 up-=ULONG_PTR(pbfh);
 			     pbfh->bfOffBits=up;
				 int dsz=up-cbh;
				 cbsz+=dsz;
				 pbfh->bfSize=cbsz;


//			     pbfh->bfOffBits=cbh;
				 
			   }
			   //else pbfh=(BITMAPFILEHEADER*)buf0.get();   


			   char* pbits_s=(char*)pbfh+pbfh->bfOffBits;
			   long cbb=bmpmap.bytes_sizeb();

			   {
				   locker_frame_t lockptr(this);
				   memcpy(pbits_s,lockptr.get(),cbb);
			   }
                   hr=S_OK;
			   
                

			   if((fmt==0)||((fmt==2)&&(*((DWORD*)"YUY2")==compression))||((fmt==1)&&(0==compression)))
			   {
                pbmpimage=pbfh;
				cbimage=cbsz;
				hr=S_OK;

			   }else 
			   {
				   int cbconvert=(compression)? 3*cbsz:cbsz;
				   if(cbconvert>buf1.size_b()) buf1.resize(cbconvert);
				   BITMAPFILEHEADER* pconvert=(BITMAPFILEHEADER*)buf1.get();   ;
				   //ULONG decompressYUY2(int scb,char *sbuf,int* pdcb,char* dbuf, void * phandle=0,char* s_pbits=0,char* d_pbits=0)
				   int res;
                   if(fmt==1)
				     res=BMPYUY2_converter::decompressYUY2(cbsz,(char*)pbfh,&cbconvert,(char*)pconvert);
				   else if(fmt==2)
					   res=BMPYUY2_converter::compressYUY2(cbsz,(char*)pbfh,&cbconvert,(char*)pconvert);
				   else res=0;
				   
				   if(res)
				   {
					   pbmpimage=pconvert;
					   cbimage=cbconvert;
					   hr= S_OK;
				   }
				   else 
				    hr=E_FAIL;

			   }


			   if(fmtget==1)
			   {
				   hr=reduce_to_24(pbmpimage,cbimage);
			   }
			   hr=frame_cache2(pbmpimage,ppbmpimage,cbimage,cbb);
  			   
			//   if((compresion==0)&&())		
			   if(SUCCEEDED(hr))
				   hr=make_crop(pbmpimage,ppbmpimage,cbimage,cbb);

			   return hr;
	};



   inline HRESULT reduce_to_24(BITMAPFILEHEADER* pbmp,int& cbimage)
   {
	     HRESULT hr;
         BITMAPINFOHEADER* pbih= (BITMAPINFOHEADER*) ((char*)pbmp+sizeof(BITMAPFILEHEADER));
		 
		 
		 if((pbih->biBitCount==32)&&(fmt))
		 {
			 BYTE* pbits=((BYTE*)pbmp)+pbmp->bfOffBits;
			 int w=pbih->biWidth,
			     h=pbih->biHeight,
				 wb=3*(w-1);
			 int line32=BYTESPERLINE(w,32);
			 int line24=BYTESPERLINE(w,24);
			 char *ps=((char*)pbits)+3;
			 char *pd=ps;


			 for(int y=0;y<h;y++)
			 {
				 char * p=ps;

				 for(char * i=pd;i<pd+wb;i+=3) 
				 {
					 *((unsigned*)i)=* ((unsigned*)(++p));
				 }
				 pd+=line24;
				 ps+=line32;
			 }

			 int szimage=(3*line24*h)+pbmp->bfOffBits;
			 pbmp->bfSize=cbimage=szimage;

		 }
         return S_OK;

   };


   crop_t crop_transform(crop_t c,int w,int h)
   {
	   c.enabled=0;
      if(c.width==0)  
		  if((c.width=w-c.left)<=0) 
			  return c;
	  if(c.height==0)  
		  if((c.height=h-c.top)<=0)
			   return c;
		  

      if(c.width<0) c.width=(w-c.left)+c.width;
	  if(c.width<=0) 
	            return c;
	  if(c.height<0) c.height=(w-c.top)+c.height;

	  if(c.height<=0) 
		          return c;


         c.width=min(c.width,w-c.left);
		 c.height=min(c.height,h-c.top);

// flip H
		 if(1){
            int t=c.top;
			int hh=c.height;
			c.top=h-(hh+t-1);
			//c.height=h-t;



		 }


      c.enabled=1;
	  return c;
   }
   


   inline HRESULT make_crop(BITMAPFILEHEADER* pbmp,BITMAPFILEHEADER** ppbmp,int& cbimage,int cbb)
   {
	   if(!in.crop.enabled){ *ppbmp=pbmp; return S_OK;}
	   BITMAPFILEHEADER* pbf_dest;
	   BITMAPINFOHEADER *pbmi_dest;
	   BITMAPINFOHEADER *pbmi_src=(BITMAPINFOHEADER *)( ((char*)pbmp)+sizeof(BITMAPFILEHEADER) );
	   int linewidth_src=BYTESPERLINE(pbmi_src->biWidth,pbmi_src->biBitCount),linewidth_dest;
	   int ws=pbmi_src->biWidth,hs=pbmi_src->biHeight;
	   

	   
	   int wd,hd;

	   

	   if(state_crop==0)
	   {  		   

		   crop_t crop=crop_transform(in.crop,ws,hs);

		   if(!crop.enabled)  return E_INVALIDARG;

		   leftoff=crop.left;
		   topoff=crop.top;

		   wd=crop.width;
		   hd=crop.height;
		   linewidth_dest=BYTESPERLINE(wd,pbmi_src->biBitCount);


		   if(leftoff>=ws) return E_INVALIDARG;  
		   if(topoff>=hs) return E_INVALIDARG;

		   

		  
		   int cbi=linewidth_dest*hd+sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
		   bufcrop.resize(cbi+16);
            
			pbf_dest=(BITMAPFILEHEADER*)bufcrop.get();
			pbmi_dest=(BITMAPINFOHEADER *)( ((char*)pbf_dest)+sizeof(BITMAPFILEHEADER) );

			*pbf_dest=*pbmp;
			*pbmi_dest=*pbmi_src;			

			 pbmi_dest->biWidth=wd;
			 pbmi_dest->biHeight=hd;		   

             pbf_dest->bfOffBits=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
			 pbf_dest->bfSize=cbi;	
			 state_crop=1;
	   }

	     pbf_dest=(BITMAPFILEHEADER*)bufcrop.get();
		 pbmi_dest=(BITMAPINFOHEADER *)( ((char*)pbf_dest)+sizeof(BITMAPFILEHEADER) );
		 
		 wd=pbmi_dest->biWidth;
		 hd=pbmi_dest->biHeight;		   
		 linewidth_dest=BYTESPERLINE(wd,pbmi_dest->biBitCount);        
		 
		 cbimage=pbf_dest->bfSize;
         *ppbmp=pbf_dest;

		 int wdb=(wd*pbmi_dest->biBitCount)/8,offsetb=(leftoff*pbmi_dest->biBitCount)/8;

		 char* pbits_s=((char*)pbmp)+pbmp->bfOffBits;
		 char* pbits_d=((char*)pbf_dest)+pbf_dest->bfOffBits;

		 pbits_s+=(linewidth_src*(topoff))+offsetb;
		 //pbits_d+=linewidth_dest*(hd-1);

		 for(int y=0;y<hd;y++)
		 {
             memcpy(pbits_d,pbits_s,wdb); 
			 pbits_s+=linewidth_src;
			 pbits_d+=linewidth_dest;
		 }





         return S_OK;

   }

	inline HRESULT frame_cache2(BITMAPFILEHEADER* pbmp,BITMAPFILEHEADER** ppbmp,long cbimage,int cbb)
	{
		HRESULT hr=S_OK;
		int nf=filters.frames_cache;

		if(nf<2) return hr;
		int cbfull=nf*cbimage;
		if(vbuf_cache.size()!=nf)
		{
			try
			{
			
			vbuf_cache.resize(nf);
			buf_cache_ptr.resize(nf);

			buf_cache_b=0;
			buf_cache_e=nf-1;
			char* p;

			
			for(int k=0;k<nf;k++) 
			{
                
				vbuf_cache[k].resize(cbimage);
				p=&vbuf_cache[k][0];
				buf_cache_ptr[k]=(BITMAPFILEHEADER*)p;
				memcpy(p,pbmp,cbimage);

				if(filters.frames_cache_effect)
				{
					//char* pbits_s=((char*)pbmp)+pbmp->bfOffBits;
					BITMAPINFOHEADER* bi=(BITMAPINFOHEADER*)(p+sizeof(BITMAPFILEHEADER));
					cbb=BYTESPERLINE(bi->biWidth,bi->biBitCount)*bi->biHeight;

					BYTE* pbits_d=(BYTE*)(p+pbmp->bfOffBits);
					if(filters.frames_cache_effect==1)
					{
						memset(pbits_d,0,cbb);  
					}
					else
					{
						double alpha=double(k)/double(nf);
						for (int n=0;n<cbb;n++) pbits_d[n]*=alpha; 

					}

				}


			}

			}		catch (...)		{      return hr=HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);  	}
		
		 *ppbmp=pbmp; 
         return hr=S_OK;
		    
		}

    	 buf_cache_b= (buf_cache_b+1)%nf;
	     buf_cache_e= (buf_cache_e+1)%nf;
		

		memcpy(buf_cache_ptr[buf_cache_e],pbmp,cbimage);
		pbmp=buf_cache_ptr[buf_cache_b];


		*ppbmp=pbmp;
		return hr=S_OK;

	}


	inline BITMAPFILEHEADER* frame_cache(BITMAPFILEHEADER* pbmp,long cbimage,int cbb)
	{
		  int nf=filters.frames_cache;
        if(nf<2) return pbmp;
		int cbfull=nf*cbimage;
		if(buf_cache.size_b()!=cbfull)
		{
			 buf_cache.v.reserve(cbfull+cbimage/8);
             buf_cache.resize(cbfull);
			 buf_cache_b=0;
			 buf_cache_e=nf-1;
			 char* p=buf_cache.get();
			 buf_cache_ptr.resize(nf);
			 for(int k=0;k<nf;k++,p+=cbimage) 
			 {
				 buf_cache_ptr[k]=(BITMAPFILEHEADER*)p;
				 memcpy(p,pbmp,cbimage);

				 if(filters.frames_cache_effect)
				 {
					 //char* pbits_s=((char*)pbmp)+pbmp->bfOffBits;
					 BITMAPINFOHEADER* bi=(BITMAPINFOHEADER*)(p+sizeof(BITMAPFILEHEADER));
					 cbb=BYTESPERLINE(bi->biWidth,bi->biBitCount)*bi->biHeight;

					 BYTE* pbits_d=(BYTE*)(p+pbmp->bfOffBits);
					 if(filters.frames_cache_effect==1)
					 {
                        memset(pbits_d,0,cbb);  
					 }
					 else
					 {
                         double alpha=double(k)/double(nf);
						 for (int n=0;n<cbb;n++) pbits_d[n]*=alpha; 

					 }

				 }
				 

			 }
			 return pbmp;
		}

		 buf_cache_b= (buf_cache_b+1)%nf;
		 buf_cache_e= (buf_cache_e+1)%nf;

		 memcpy(buf_cache_ptr[buf_cache_e],pbmp,cbimage);
		 pbmp=buf_cache_ptr[buf_cache_b];

		 

		 return pbmp;

	}

	virtual  ULONG Compression(ULONG mode=0)
	{          
			 //if(!bmpmap.get_BITMAPFILEHEADER()) return -1;

			 fmt=mode;

			 return compression;
	};

	virtual HRESULT SetOptionsA(const char* argszz)
	{
		return SetOptions(char_mutator<CP_THREAD_ACP,true>(argszz));
	}


	virtual HRESULT SetOptions(const wchar_t* pargszz)
	{
		HRESULT hr;
		 argv_zz<wchar_t> args(pargszz);
		 flip=args[L"flip"].def(flip);
		 flipV=args[L"flipV"].def(flip&1);
		 flipH=args[L"flipH"].def(flip&2);
		 fmtout=args[L"fmtout"].def(fmtout);
		 fmtget=args[L"fmtget"].def(fmtget);
		 filters.set_opts(args);


		 update_args(in.crop.enabled);
		 update_args(in.crop.left);
		 update_args(in.crop.width);
		 update_args(in.crop.top);
		 update_args(in.crop.height);



		 return S_OK;

	}
	virtual HRESULT SetSingleton(int flag=0)
	{
		singleton_restarter_t<>  sr(vkey.get());
		return S_OK;
	}

public:
	struct filters_t
	{
		
		typedef mutex_cs_t mutex_t;
		 mutex_t mutex;
		 byte_t mask;
		 int check;
		 
		 
		decay_filter_buf_t<> decay;
		  double frames_cache;
		  int frames_cache_effect;

		  filters_t():mask(0),check(0),frames_cache(0),frames_cache_effect(0){};

		template <class Args>
		inline void set_opts(Args& args)
		{
			locker_t<filters_t::mutex_t> lock(mutex);
			std::vector<double> vt(2);
			vt[0]=decay.t;
			vt[1]=decay.t2;
			//double t=args[L"filters.decay.t"].def(decay.t);
			check=args[L"filters.check"].def(check);
			vt=args[L"filters.decay.t"].def(vt);

			frames_cache=args[L"filters.frames.cache"].def(frames_cache);
			frames_cache_effect=args[L"filters.frames.effect"].def(frames_cache_effect);
			

			decay.set_alpha(vt);
			decay.mask=args[L"filters.decay.mask"].def(decay.mask);
			mask=args[L"filters.mask"].def(mask);
			

		}


	};
    filters_t filters;
	std::vector<frame_data_t> vptr_mutexes;
	BMPMap bmpmap;
	BB_t* pbbout,*pbbin;
	BB_t bbh;
	long last_fb;
	v_buf<char> buf0,buf1,buf2,bufcheck,bufcheck2,bufcrop;
	v_buf<wchar_t> vkey;
	v_buf<BITMAPFILEHEADER*> buf_cache_ptr;
	v_buf<char> buf_cache;
	std::vector<std::vector<char> > vbuf_cache;

	int buf_cache_b,buf_cache_e;
	

	int fmt,flip,flipV,flipH,wfmt,fmtout,fmtget;
	DWORD compression;
	int state,state_crop;
	int nframes_,capacity_;
private:
	
	LONG refcount;
	dummy_dispatch_t ddisp;

};





//#define _m(s) char_mutator<CP_X>(s)

extern "C"  HRESULT Create_ISharedImage(ISharedImage** ppImage)
{
	return ISharedImage_impl<>::CreateInstance(ppImage);
}; 

extern "C"  void ltx_Create_ISharedImage(VARIANT* pres,pcallback_lib_arguments_t plibc,pcallback_context_arguments_t pcca)
{
	HRESULT t;
	if(!pres) {*(pcca->phr)= E_POINTER; return;}

	
	
	VARIANT vsi={VT_UNKNOWN};  
	if(SUCCEEDED(*(pcca->phr)=Create_ISharedImage((ISharedImage**)&vsi.byref)))
	{
		int last=pcca->argc-1;
		VARIANT* pv=pcca->argv+last;
		if( (last>=0)&&(pv->vt==VT_BSTR) )
			((ISharedImage*)(vsi.byref))->SetOptions(pv->bstrVal);


		*pres=vsi;
	}
}
