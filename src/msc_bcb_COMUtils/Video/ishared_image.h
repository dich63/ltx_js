#pragma  once
#ifndef ISHARED_IMAGE_H
#define ISHARED_IMAGE_H
#endif

#include <windows.h>
//
/*
#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif
//*/
#define ICMDLINE 535
#define FROM_CMDLINE_W ((const wchar_t*)ICMDLINE)
#define FROM_CMDLINE_A ((const char*)ICMDLINE)
struct ICaptureImage:IUnknown
{
#pragma push(pack)
#pragma pack(2)
typedef	struct FILEINFO
	{
       BITMAPFILEHEADER bf;
	   BITMAPINFOHEADER bi;
	} FILEINFO,*PFILEINFO;
#pragma pop(pack)

	virtual HRESULT InitA(const char* options,FILEINFO** ppfi=0,int* pcbimage=0)=0;
	virtual HRESULT Init(const wchar_t* options,FILEINFO** ppfi=0,int* pcbimage=0)=0;
	virtual HRESULT GetFrame(FILEINFO** ppfi,int* pcbimage=0)=0;

	inline	static HRESULT Create(ICaptureImage** pp,const wchar_t* libpathname=L"nps_detect.dll")
	{
		typedef HRESULT (*pCreate_ICaptureImage_t)(ICaptureImage** ppImage);

		HRESULT hr;
		HMODULE hlib=LoadLibraryW(libpathname);

		if(!hlib) return hr=GetLastError();

		pCreate_ICaptureImage_t create_proc=(pCreate_ICaptureImage_t)GetProcAddress(hlib,"Create_ICaptureImage");
		if(create_proc) hr=create_proc(pp);
		else return hr=GetLastError();

		FreeLibrary(hlib);
		return hr;
	}

};
struct ISharedImage:IUnknown
{

	typedef unsigned char byte_t;
	virtual HRESULT InitA(const char* mapkey,void* pbmp_blank_image=0,int nframes=0,int capacity=-1)=0;
	virtual HRESULT Init(const wchar_t* mapkey,void* pbmp_blank_image=0,int nframes=0,int capacity=-1)=0;
	virtual HRESULT PushFrame(void* pbmpimage,const byte_t* pbits=0,LARGE_INTEGER* pTimeStamp=0)=0;
	virtual HRESULT GetBitmapInfo(BITMAPINFOHEADER* pbmi)=0;
	virtual HRESULT LockFramePtr(byte_t** ppbits,int mode=0)=0; // 0 -read access, 1- write access
	virtual HRESULT UnlockFramePtr(byte_t* pbits)=0;
	virtual HRESULT GetFrame(BITMAPFILEHEADER** pbmpimage,int* pcbimage=0, LARGE_INTEGER* pTimeStamp=0)=0;
	virtual  ULONG Compression(ULONG mode=0)=0;
        virtual HRESULT Init2(const wchar_t* key, int width=0,int height=0,int fmt=0,int nframes=0,int capacity=-1)=0;
        virtual HRESULT Init2A(const char* key, int width=0,int height=0,int fmt=0,int nframes=0,int capacity=-1)=0;
		virtual HRESULT SetOptions(const wchar_t* argszz)=0;
		virtual HRESULT SetOptionsA(const char* argszz)=0;
		virtual HRESULT SetSingleton(int flag=0)=0;
		virtual HRESULT GetAdditionDataPtrSize(byte_t** ppbits,long* psize=0)=0;


inline	static HRESULT Create(ISharedImage** pp,const wchar_t* libpathname=L"nps_detect.dll")
	{
		typedef HRESULT (*pCreate_ISharedImage_t)(ISharedImage** ppImage);
		HRESULT hr;
		wchar_t* lpn;
                HMODULE hlib=LoadLibraryW(libpathname);
				hr=GetLastError();
                if(!hlib&&(lpn=_wgetenv(L"SharedImage.lib")))
				{
					hlib=LoadLibraryW(lpn);
					hr=GetLastError();
				}

				

		if(!hlib) return hr;

        pCreate_ISharedImage_t create_proc=(pCreate_ISharedImage_t)GetProcAddress(hlib,"Create_ISharedImage");
		if(create_proc) hr=create_proc(pp);
			else return hr=GetLastError();
	
		FreeLibrary(hlib);
		return hr;
	}

};


#define BYTESPERLINE(Width, BPP) ((WORD)((((DWORD)(Width) * (DWORD)(BPP) + 31) >> 5)) << 2)

struct BMP_helper_t
{
    HRESULT hr;
    BITMAPFILEHEADER* pbfh;	
    int cbsz;
    BMP_helper_t():pbfh(0),cbsz(0),hr(E_POINTER){}

    BMP_helper_t(ISharedImage* p,LARGE_INTEGER* pTimeStamp=0):pbfh(0),cbsz(0)
    {
        hr=p->GetFrame(&pbfh,&cbsz,pTimeStamp);
    }

inline    BITMAPINFOHEADER* hdr()
{
    return (BITMAPINFOHEADER*)(((char*)pbfh)+sizeof(BITMAPFILEHEADER));
}

inline    BITMAPINFO* hdr_info()
{
	return (BITMAPINFO*)(((char*)pbfh)+sizeof(BITMAPFILEHEADER));
}

inline int width(){ return hdr()->biWidth;}
inline int height(){ return hdr()->biHeight;}

 template <class T>
        T* bits(){
     return (T*)(((char*)pbfh)+pbfh->bfOffBits);
 }
 inline int image_size(){ return cbsz;}

 inline int linesize_b()
 {
     BITMAPINFOHEADER* pbih=hdr();
     return BYTESPERLINE(pbih->biWidth,pbih->biBitCount);
 }
    inline int bits_size_b()
    {
        BITMAPINFOHEADER* pbih=hdr();
        return pbih->biHeight*BYTESPERLINE(pbih->biWidth,pbih->biBitCount);
    }
};

template <class Intf>
struct   image_helper_base_t
{
	HRESULT hr;
	Intf* p;


	template <class T, class T2>
	inline static T make_detach(T& dst, T2 n)
	{  
		T t=dst;
		dst=(T)n;
		return t;
	};

	template <class T>
	inline static T make_detach(T& dst)
	{  
		T t=dst;
		dst=T(); 
		return t;
	};

	inline Intf* reset(Intf* np=0,bool faddref=true)
	{
		if(np!=p)
		{
			Intf* t=make_detach(p,np);
			if(p&&faddref) p->AddRef();
			if(t)
				t->Release();
		}
		return p;
	}

	

	~image_helper_base_t(){
		reset();
	}
	image_helper_base_t(bool finstall=true):p(0)
	{
		if(finstall) hr= Intf::Create(&p,L"nps_detect.dll");
		else hr=E_POINTER;
	}

	inline  Intf* operator->()
	{
		return p;
	}
	inline   operator Intf*()
	{
		return p;
	}

	Intf** address()
	{
		reset();
		return &p;
	}


	long refcount()
	{
		if(p)
		{
			p->AddRef();
			return p->Release();
		}
		else return 0;
	}

	operator bool(){ return hr==0;}
	bool operator !(){ return hr;}	



};
struct   capture_image_helper_t:image_helper_base_t<ICaptureImage>
{
	capture_image_helper_t():image_helper_base_t<ICaptureImage>(){}; 
};


struct   shared_image_helper_t:image_helper_base_t<ISharedImage>
{
	shared_image_helper_t():image_helper_base_t<ISharedImage>(){};

	shared_image_helper_t(const char* key,const wchar_t* libpathname=L"nps_detect.dll")
		:image_helper_base_t<ISharedImage>()
	{
		
		if(hr==S_OK) hr=p->Init2A(key);
	}

	shared_image_helper_t(const wchar_t* key,const wchar_t* libpathname=L"nps_detect.dll")
		:image_helper_base_t<ISharedImage>()
	{
		hr= ISharedImage::Create(&p,libpathname);
		if(hr==S_OK) hr=p->Init2(key);
	}

	shared_image_helper_t(const shared_image_helper_t& sp):image_helper_base_t<ISharedImage>(false)
	{
		reset(sp.p);
		hr=sp.hr;
	}

	shared_image_helper_t& operator =(const shared_image_helper_t& sp)
	{
		reset(sp.p);
		hr=sp.hr;
		return *this;
	}
	template <class Args>
    inline void set_options(Args& a) 
	{
		 const wchar_t* pzz=a.flat_str();
         if(hr==S_OK) 
			 hr=p->SetOptions(pzz);
	}
	

};

template <class F>
bool loop_fps(F functor,double fps=25)
{
	if(fps<0.0001) return false;
	DWORD tio=DWORD(1000/fps);
	ULONGLONG t0=GetTickCount64(),t;
	int dt;
	while(functor())
	{
       t=GetTickCount64();
	   dt=int(t-t0);
       if(dt>10) Sleep(dt);   
	   t0=GetTickCount64();
	}

	return true;
}

/*
struct   shared_image_helper_t
{
    HRESULT hr;
   ISharedImage*p;


   template <class T, class T2>
  inline static T make_detach(T& dst, T2 n)
   {  
	   T t=dst;
	   dst=(T)n;
	   return t;
   };

   template <class T>
   inline static T make_detach(T& dst)
   {  
	   T t=dst;
	   dst=T(); 
	   return t;
   };

   ~shared_image_helper_t(){
	   reset();
   }
   shared_image_helper_t():p(0)
   {
           hr= ISharedImage::Create(&p,L"nps_detect.dll");
   }


    shared_image_helper_t(const char* key,const wchar_t* libpathname=L"nps_detect.dll"):p(0)
	{
		hr= ISharedImage::Create(&p,libpathname);
		if(hr==S_OK) hr=p->Init2A(key);
	}

   shared_image_helper_t(const wchar_t* key,const wchar_t* libpathname=L"nps_detect.dll"):p(0)
   {
	   hr= ISharedImage::Create(&p,libpathname);
	   if(hr==S_OK) hr=p->Init2(key);
   }

	shared_image_helper_t(const shared_image_helper_t& sp):p(0)
	{
		reset(sp.p);
		hr=sp.hr;
	}

	shared_image_helper_t& operator =(const shared_image_helper_t& sp)
	{
		return reset(sp.p);
		hr=sp.hr;
	}

	inline shared_image_helper_t& reset(ISharedImage* np=0,bool faddref=true)
	{
		if(np!=p)
		{
			ISharedImage* t=make_detach(p,np);
			if(p&&faddref) p->AddRef();
			if(t)
				t->Release();
		}
		return *this;
	}
	inline  ISharedImage* operator->()
	{
		return p;
	}
	inline   operator ISharedImage*()
	{
		return p;
	}

	ISharedImage** address()
	{
		reset();
		return &p;
	}
	

	long refcount()
	{
		if(p)
		{
			p->AddRef();
			return p->Release();
		}
		else return 0;
	}

	operator bool(){ return hr==0;}
	bool operator !(){ return hr;}	

};
*/