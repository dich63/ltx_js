#pragma once

#include "windows.h"

#pragma comment(lib, "Rpcrt4.lib") 

namespace local_port {


struct nuid_t
{
	unsigned long long ul;
	unsigned long long uh;

};
struct lp_message
{
  nuid_t sng;
  long pid;
  long cb;

};


inline bool operator ==(nuid_t& l,nuid_t& r)
{
	return (l.ul==r.ul)&&(l.uh==r.uh);
}
inline bool operator <(nuid_t& l,nuid_t& r)
{
	return (l.ul<r.ul)||((l.ul==r.ul)&&(l.uh<r.uh));
}


inline UUID shared_uuid(DWORD to=INFINITE)
{

	static  GUID  unpossible= { 0xd4c6ef52, 0xabb0, 0x44fe, { 0xbd, 0x14, 0x97, 0x9f, 0x60, 0x90, 0xf2, 0x61 } };


	char * pmp="{D4C6EF52-ABB0-44fe-BD14-979F6090F261}_map";

	static volatile LONG  l;
	static UUID uid;
	HANDLE hmap;
	LONG ll=l;
	if(InterlockedExchangeAdd(&l,0))
		return uid;

	hmap=::CreateFileMappingA(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,4*1024,pmp);
	bool fexists=(GetLastError()==ERROR_ALREADY_EXISTS);
	if(hmap==NULL) 
	{
		uid=unpossible;
		InterlockedExchangeAdd(&l,1);
		return uid;
	}

	struct s_gc
	{
		HANDLE h;
		s_gc(DWORD to)
		{ 
			char * pmx="{D4C6EF52-ABB0-44fe-BD14-979F6090F261}_mtx";
			h=::CreateMutexA(0,false,pmx);
			WaitForSingleObject(h,to);
		};
		~s_gc()
		{
			::ReleaseMutex(h);
			::CloseHandle(h);
		}
	} 
	gc(to);




	DWORD faccess=(fexists)?FILE_MAP_READ:FILE_MAP_ALL_ACCESS;
	UUID* puid= (UUID*)MapViewOfFile(hmap,faccess,0,0,4*1024);
	if(puid)
	{
		if(!fexists) UuidCreateSequential(puid);
		uid=*puid;
		UnmapViewOfFile(puid);
	}

	InterlockedExchangeAdd(&l,1);

	return uid;
	//UuidCreateSequential(&uid);

}

inline void shared_uuid(UUID* puid, DWORD to=INFINITE)
{
	*puid=shared_uuid(to);
}

inline  nuid_t& host_id(nuid_t& n,int to=INFINITE)
{
 	 shared_uuid((UUID*)&n,to);
	 return n;
}

inline  nuid_t host_id(int to=INFINITE)
{
      nuid_t n;
	  return host_id(n,to);
}

};