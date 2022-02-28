#pragma once

#include "static_constructors_0.h" 



struct Cryptdll
{


	typedef struct {
		ULONG         i[2];
		ULONG         buf[4];
		unsigned char in[64];
		unsigned char digest[16];
	} MD5_CTX;

	MD5_CTX mdctx;

	inline Cryptdll& update(char* input, int inlen=-1)
	{
		return update((unsigned char*)input,inlen);
	}
	inline Cryptdll& update(unsigned char* input,int inlen=-1)
	{
		if(inlen<0) inlen=strlen((char*)input);
		lib().MD5Update(&mdctx,input,inlen);
		return *this;
	};

	inline MD5_CTX& hash()
	{
		lib().MD5Final(&mdctx);
		return mdctx;
	};


	inline operator MD5_CTX&()
	{
		return hash();
	}

	inline operator char* ()
	{
		return (char*)(hash().digest);
	}
    inline Cryptdll& reinit()
	{
       lib().MD5Init(&mdctx);
		return *this;
	}

	inline Cryptdll(unsigned char* input=0,unsigned int inlen=-1)
	{
		reinit();
		if((input)&&(inlen))
		{

			update(input,inlen);
		}
	}

protected:
#define SET_PROC_ADDRESS(h,a) _set_dll_proc(h,(void**)&a,#a)
	struct CryptdllLoader
	{
		HMODULE hlib;
		void (__stdcall *MD5Init)(MD5_CTX* );
		void (__stdcall *MD5Update)(MD5_CTX*,unsigned char* input,unsigned int inlen);
		void (__stdcall *MD5Final)(MD5_CTX* );

 static	inline void _set_dll_proc(HMODULE h,void** pp,char* n){
			*pp=(void*)GetProcAddress(h,n);
		}

		CryptdllLoader(){
			hlib=LoadLibraryA("Cryptdll.dll");
			SET_PROC_ADDRESS(hlib,MD5Init);
			SET_PROC_ADDRESS(hlib,MD5Update);
			SET_PROC_ADDRESS(hlib,MD5Final);
		};
		~CryptdllLoader(){
			FreeLibrary(hlib);
		};
	};

	inline static CryptdllLoader& lib()
	{
		static class_initializer_T<CryptdllLoader> t;
		return t.get();
	}

};


