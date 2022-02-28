#ifndef __EXPORT_TOOL_H 
#define __EXPORT_TOOL_H

#include <windows.h>
#include <stdio.h>

inline int get_export_section_RVA(const void* baseAdrress=GetModuleHandle(NULL))
{
	PIMAGE_DOS_HEADER pdos_header =PIMAGE_DOS_HEADER(baseAdrress);
	PIMAGE_NT_HEADERS ppe_header= PIMAGE_NT_HEADERS( PBYTE(baseAdrress)+pdos_header->e_lfanew);
	return ppe_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
};

inline  PIMAGE_EXPORT_DIRECTORY get_export_directory(const void* baseAdrress=GetModuleHandle(NULL))
{
	int offset=get_export_section_RVA(baseAdrress);
	return PIMAGE_EXPORT_DIRECTORY( PBYTE(baseAdrress)+offset);
};

inline DWORD symbol_count(PIMAGE_EXPORT_DIRECTORY pied)
{
	return pied->NumberOfNames;
}
inline const char* symbol_name(int i,const void* baseAdrress,const PIMAGE_EXPORT_DIRECTORY pied)
{
	PDWORD pnameRVA=PDWORD(PBYTE(baseAdrress)+pied->AddressOfNames)+i; 
	return PCHAR(baseAdrress)+(*pnameRVA);
}


inline  WORD symbol_index(int i,const void* baseAdrress,const PIMAGE_EXPORT_DIRECTORY pied)
{
	PWORD pfuncindex=PWORD(PBYTE(baseAdrress)+pied->AddressOfNameOrdinals)+i; 
	return *pfuncindex;
}

inline  WORD symbol_ordinal(int i,const void* baseAdrress,const PIMAGE_EXPORT_DIRECTORY pied)
{
	PWORD pfuncindex=PWORD(PBYTE(baseAdrress)+pied->AddressOfNameOrdinals)+i; 
	return *pfuncindex+pied->Base;
}


inline const void* symbol_address(int i,const void* baseAdrress,const PIMAGE_EXPORT_DIRECTORY pied)
{
	i=symbol_index(i,baseAdrress,pied);
	PDWORD pfuncRVA=PDWORD(PBYTE(baseAdrress)+pied->AddressOfFunctions)+i; 
	return PCHAR(baseAdrress)+(*pfuncRVA);
}


#define HINSTANCE_IN_CODE ( __getHinstance(address_next(0)))

template <typename T>
T* __stdcall address_next(T i)
{
	//return (T*)(*(&p-1));
     return (T*) (*(((void**)&i)-1));
}


template <typename T>
HMODULE __stdcall __getHinstance(T* p)
{
	HMODULE h=0;
	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|
		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)p,&h);
	return h;
}


class export_table
{
public:
	PIMAGE_EXPORT_DIRECTORY pied;
	void* baseAdrress;
/*
	export_table(HMODULE hm=HINSTANCE_IN_CODE)
	{
		pied=get_export_directory(baseAdrress=hm);
	};
*/
	export_table(void* hm=HINSTANCE_IN_CODE)
	{
		hm=__getHinstance(hm);
		pied=get_export_directory(baseAdrress=hm);
	};

	inline int count() 
	{
		return symbol_count(pied);
	};

	inline const char* name(int i)
	{
		return  symbol_name(i,baseAdrress,pied);
	};

	inline const void* address(int i)
	{
		return  symbol_address(i,baseAdrress,pied);
	};

	inline const WORD ordinal(int i)
	{
		return  symbol_ordinal(i,baseAdrress,pied);
	};
   
	inline const void* address(const char* n,bool fcs=true)
	{
	   int ( *pcmp)(const char *,const char *)=(fcs)?&strcmp:&strcmpi;
	   for(int i=0;i<count();i++)
		   if(pcmp(n,name(i))==0)
			   return address(i);
        return NULL;
	};
   
};

struct chbuf
{
	char buf[MAX_PATH];
inline	operator char*()
	{
     return buf;
	}
};

inline const char* proc_name(void* paddr)
{
	  //export_table et=__getHinstance(paddr);
	  export_table et=paddr;
	  for(int n=0;n<et.count();n++)
	  {
		  if(et.address(n)==paddr)
			  return et.name(n);
	  }
  return NULL;
}
inline short proc_ordinal(void* paddr)
{
	//export_table et=__getHinstance(paddr);
	export_table et=paddr;
	for(int n=0;n<et.count();n++)
	{
		if(et.address(n)==paddr)
			return et.ordinal(n);
	}
	return NULL;
}

inline  chbuf lib_name(void* paddr)
{
	  chbuf buf; buf[0]=0;
	GetModuleFileNameA(__getHinstance(paddr),buf,sizeof(buf));
	return buf;
}


#endif