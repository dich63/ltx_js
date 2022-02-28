#pragma once
//sparse_file_map.h
#include "sm_region.h"
#include <stdio.h>
#include <tchar.h>
#include <string.h>
#include <psapi.h>
#include <strsafe.h>
#include <string>
#include <WinIoCtl.h>
#include <fcntl.h>
//#include <stat.h>

#pragma comment(lib,"psapi.lib")

namespace spfm
{

	template <int __Vers__=0>
	struct sparse_file_map{

		typedef HANDLE handle_t;
		typedef unsigned long ulong32_t;
		typedef unsigned long long ulong64_t;
		typedef  long long int64_t;


		struct last_error_t
		{
			HRESULT hr;
			last_error_t(HRESULT _hr=0):hr(_hr){}
			~last_error_t(){
				SetLastError(HRESULT_FROM_WIN32(hr));
			}
			inline operator HRESULT(){ return hr;} 
			inline HRESULT operator =(HRESULT h){ 
				return hr=HRESULT_FROM_WIN32(h);
			} 
			inline HRESULT operator()(HRESULT h=GetLastError())
			{
				return hr=HRESULT_FROM_WIN32(h);
			}

		};




		struct pair_mmn_t
		{
			wchar_t *filename;
			wchar_t* mapname;
			int fcompress;
			int ftemp;
			v_buf<wchar_t> buf;

			pair_mmn_t(const wchar_t* fn,const wchar_t* delim=L"::"):mapname(0),filename(L""),buf(fn,0)
			{
				if(!fn) return;
				filename=buf;		
				wchar_t* p;
				if(p=wcsstr(filename,delim))
				{
					*p=0;
					mapname=p+wcslen(delim);			
				}

			};

			inline operator bool()
			{
				return filename[0];
			}

		};


		inline static int64_t file_size(HANDLE hf)
		{
			LARGE_INTEGER ll;
			if(!GetFileSizeEx(hf,&ll)) 
				 return -1;
			/*
			BY_HANDLE_FILE_INFORMATION bhfi;
			
			if(!GetFileInformationByHandle(hf, &bhfi)) return -1;
			
			ll.LowPart=bhfi.nFileSizeLow;
			ll.HighPart=bhfi.nFileSizeHigh;
			return ll.QuadPart;
			*/
			return ll.QuadPart;
		}


		inline static bool is_sparse(HANDLE hf)
		{
			BY_HANDLE_FILE_INFORMATION bhfi;
			if(!GetFileInformationByHandle(hf, &bhfi)) return false;
			return bhfi.dwFileAttributes&FILE_ATTRIBUTE_SPARSE_FILE;
		}
		inline static bool is_compression(HANDLE hf)
		{
			BY_HANDLE_FILE_INFORMATION bhfi;
			if(!GetFileInformationByHandle(hf, &bhfi)) return false;
			return bhfi.dwFileAttributes&FILE_ATTRIBUTE_COMPRESSED;
		}

		inline static HRESULT set_compression(HANDLE hf,unsigned cf=COMPRESSION_FORMAT_DEFAULT)
		{

			unsigned short scf=cf;
			DWORD dw=0;
			bool f=is_compression(hf);
			if(!f)
				f=DeviceIoControl(hf,FSCTL_SET_COMPRESSION,&cf,sizeof(cf),0,0,&dw, NULL);
			return f?S_OK:HRESULT_FROM_WIN32(GetLastError());
		}

		inline static HRESULT set_sparse(HANDLE hf)
		{
			DWORD dw=0;
			bool f=is_sparse(hf);
			if(!f)
				f=DeviceIoControl(hf, FSCTL_SET_SPARSE,NULL, 0, NULL, 0,&dw, NULL);
			return f?S_OK:HRESULT_FROM_WIN32(GetLastError());
		}







		static handle_t create_file(wchar_t* filename,int oflag,int shflag,int pmode,bool shdelete=false)
		{


			handle_t hf,hmap;


			DWORD fileaccess,fileshare,filecreate,fileattrib;
			/*
			* decode the access flags
			*/
			switch( oflag & (_O_RDONLY | _O_WRONLY | _O_RDWR) ) {
		case _O_RDONLY:         /* read access */
			fileaccess = GENERIC_READ;
			break;
		case _O_WRONLY:  

			if ((oflag & _O_APPEND)
				&& (oflag & (_O_WTEXT | _O_U16TEXT | _O_U8TEXT)) != 0)
			{
				fileaccess = GENERIC_READ | GENERIC_WRITE;
			}
			else
			{
				fileaccess = GENERIC_WRITE;
			}
			break;
		case _O_RDWR:           /* read and write access */
			fileaccess = GENERIC_READ | GENERIC_WRITE;
			break;
		default:             

			fileaccess =0;
			}

			/*
			* decode sharing flags
			*/
			switch ( shflag ) {

		case _SH_DENYRW:        /* exclusive access */
			fileshare = 0L;
			break;

		case _SH_DENYWR:        /* share read access */
			fileshare = FILE_SHARE_READ;
			break;

		case _SH_DENYRD:        /* share write access */
			fileshare = FILE_SHARE_WRITE;
			break;

		case _SH_DENYNO:        /* share read and write access */
			fileshare = FILE_SHARE_READ | FILE_SHARE_WRITE;
			break;

		case _SH_SECURE:       /* share read access only if read-only */
			if (fileaccess == GENERIC_READ)
				fileshare = FILE_SHARE_READ;
			else
				fileshare = 0L;
			break;

		default:                /* error, bad shflag */
			fileshare = 0L;
			}

			/*
			* decode open/create method flags
			*/
			switch ( oflag & (_O_CREAT | _O_EXCL | _O_TRUNC) ) {
		case 0:
		case _O_EXCL:                   // ignore EXCL w/o CREAT
			filecreate = OPEN_EXISTING;
			break;

		case _O_CREAT:
			filecreate = OPEN_ALWAYS;
			break;

		case _O_CREAT | _O_EXCL:
		case _O_CREAT | _O_TRUNC | _O_EXCL:
			filecreate = CREATE_NEW;
			break;

		case _O_TRUNC:
		case _O_TRUNC | _O_EXCL:        // ignore EXCL w/o CREAT
			filecreate = TRUNCATE_EXISTING;
			break;

		case _O_CREAT | _O_TRUNC:
			filecreate = CREATE_ALWAYS;
			break;

		default:
			// this can't happen ... all cases are covered
			filecreate =0;
			}

			/*
			* decode file attribute flags if _O_CREAT was specified
			*/
			fileattrib = FILE_ATTRIBUTE_NORMAL;     /* default */

			if ( oflag & _O_CREAT ) {

				if ( !((pmode) & 0x0080) )
					fileattrib = FILE_ATTRIBUTE_READONLY;
			}

			/*
			* Set temporary file (delete-on-close) attribute if requested.
			*/
			if ( oflag & _O_TEMPORARY ) {
				fileattrib |= FILE_FLAG_DELETE_ON_CLOSE;
				fileaccess |= DELETE;
				fileshare |= FILE_SHARE_DELETE;
			}

			/*
			* Set temporary file (delay-flush-to-disk) attribute if requested.
			*/
			if ( oflag & _O_SHORT_LIVED )
				fileattrib |= FILE_ATTRIBUTE_TEMPORARY;

			/*
			* Set sequential or random access attribute if requested.
			*/
			if ( oflag & _O_SEQUENTIAL )
				fileattrib |= FILE_FLAG_SEQUENTIAL_SCAN;
			else if ( oflag & _O_RANDOM )
				fileattrib |= FILE_FLAG_RANDOM_ACCESS;

			SECURITY_ATTRIBUTES sa={sizeof(sa),NULL,BOOL(oflag & _O_NOINHERIT)};  
			//if(INVALID_HANDLE_VALUE==()
			if(shdelete)
				fileshare |= FILE_SHARE_DELETE;

			hf=CreateFileW(filename,fileaccess,fileshare,&sa,filecreate,fileattrib,0);		

			return  hf; 
		}


		static ulong32_t set_zero_block(handle_t hf,ulong64_t offset,ulong64_t size)
		{
			GetLastError();
			FILE_ZERO_DATA_INFORMATION fzdi;
			DWORD dwTemp=0,ff;

			fzdi.FileOffset.QuadPart = offset;
			fzdi.BeyondFinalZero.QuadPart = offset+size;
			ff=::DeviceIoControl(hf, FSCTL_SET_ZERO_DATA,
				&fzdi, sizeof(fzdi), NULL, 0, &dwTemp, NULL);
			return ff;

		}
		static ulong64_t set_file_size(handle_t hf,ulong64_t space_size)
		{

			bool f;
			if(hf!=INVALID_HANDLE_VALUE)
			{
				LARGE_INTEGER& ll=*PLARGE_INTEGER(&space_size);
				LARGE_INTEGER lb;
				if(!GetFileSizeEx(hf,&lb)) return 0;
				//lb.QuadPart=0;
				//DWORD mm=(lb.QuadPart)?FILE_BEGIN:FILE_CURRENT;
				DWORD mm=FILE_BEGIN;
				f=SetFilePointerEx(hf,ll,&lb,mm);
				if(f)
					f=SetEndOfFile(hf);

				if(f) return lb.QuadPart;

			}
			return 0;
		}

		static ulong64_t set_file_size2(handle_t hf, ulong64_t space_size)
		{

			bool f;
			if (hf != INVALID_HANDLE_VALUE)
			{
				
				FILE_END_OF_FILE_INFO feof;

				feof.EndOfFile.QuadPart = space_size;
					

				f = SetFileInformationByHandle(hf, FileEndOfFileInfo,&feof,sizeof(feof));

				if (f) return feof.EndOfFile.QuadPart;
				

			}
			return 0;
		}


		static	handle_t get_enties(HANDLE hf,FILE_ALLOCATED_RANGE_BUFFER* pranges,ulong32_t entries)
		{
			FILE_ALLOCATED_RANGE_BUFFER queryrange;
			FILE_ALLOCATED_RANGE_BUFFER ranges[1];

			ranges[0].FileOffset.QuadPart=-1;
			ranges[0].Length.QuadPart=-1;
			DWORD nbytes, n, i,dr;
			BOOL br;

			queryrange.FileOffset.QuadPart = 0;
			br=GetFileSizeEx(hf,&queryrange.Length);
			if(!pranges) 
			{
				pranges=ranges;
				entries=1;
			}
			;

			br = ::DeviceIoControl(hf, FSCTL_QUERY_ALLOCATED_RANGES, &queryrange, sizeof(queryrange), pranges,entries*sizeof(ranges), &nbytes, NULL);
			if (!br) {
				dr = ::GetLastError();
				if (dr != ERROR_MORE_DATA) return 0;
			}
			return handle_t(nbytes/sizeof(FILE_ALLOCATED_RANGE_BUFFER));
		}

/*
		static		handle_t create_file_map_space(handle_t hf,wchar_t* pmapname,ulong64_t& space_size)
		{
			handle_t hmap;

			if((space_size!=0)&&(set_sparse_file_size(hf,space_size)==0))
				return 0;
			SECURITY_ATTRIBUTES sa={sizeof(SECURITY_ATTRIBUTES),NULL,TRUE};  
			LARGE_INTEGER& ll=*PLARGE_INTEGER(&space_size);
			return hmap=CreateFileMappingW(hf,&sa,PAGE_READWRITE,ll.HighPart,ll.LowPart,pmapname);
		}


		static		inline bool is_more_data()
		{
			return GetLastError()==ERROR_MORE_DATA;
		}

		static			inline bool already_exists()
		{
			return GetLastError()==ERROR_ALREADY_EXISTS;
		}





		handle_t create_sparse_file_map_space(wchar_t* pstr,ulong64_t& space_size,int mode,int share_mode,int pmode,int sparse_compress,void *phfile)
		{


			last_error_t lerr(0);	


			handle_t hmap;
			pair_mmn mmn(pstr);



			if(!mmn)
			{
				return hmap=OpenFileMappingW(FILE_MAP_READ|FILE_MAP_WRITE,true,mmn.mapname);
			}


			//ulong64 null_s=0;
			//DWORD dwa=GENERIC_READ|GENERIC_WRITE,dwsh=FILE_SHARE_WRITE|FILE_SHARE_READ,dwcr=OPEN_ALWAYS,dwatt=FILE_ATTRIBUTE_NORMAL;

			if(mmn.ftemp) mode|=O_TEMPORARY;

			handle_t hf=create_sparse_file(pstr,mode,share_mode,pmode,0);//sparse_compress);
			lerr.set();
			if(hf==INVALID_HANDLE_VALUE) return 0;


			if(mmn.fcompress) sparse_compress|=0x2;


			//if( set_sparse(hf))
			bool fstrict=(sparse_compress&0x10)==0;

			if( (sparse_compress&1)&&(! set_sparse(hf))&&fstrict) { CloseHandle(hf);return 0;};

			{


				LARGE_INTEGER& ll=*PLARGE_INTEGER(&space_size);
				if(space_size==0)
				{
					if(!GetFileSizeEx(hf,&ll)) return close_handle(hf),0;
				}

				space_size=size_align(space_size);

				set_sparse_file_size(hf.m_h,space_size);
				//return wild_handle_state(0,0);

				if( (sparse_compress&2)&&(!set_compression(hf))&&fstrict) {lerr.set(); CloseHandle(hf);return 0;};

				SECURITY_ATTRIBUTES sa={sizeof(SECURITY_ATTRIBUTES),NULL,TRUE};  

				sa=get_SA(TRUE);
				//
				//	hmap=CreateFileMappingW(hf,&sa,PAGE_READWRITE,0,0,mmn.mapname);
				//
				hmap=CreateFileMappingW(hf,&sa,PAGE_READWRITE,ll.HighPart,ll.LowPart,mmn.mapname);
				//hmap=CreateFileMappingW(hf,&sa,PAGE_READWRITE,0,0,mmn.mapname);
			}



			lerr.set();
			//bool fexists=lerr.set()==ERROR_ALREADY_EXISTS;
			bool fexists=0;
			if(phfile)
			{
				* PHANDLE(phfile)=hf;
			}
			else close_handle(hf);
			//return wild_handle_state(hmap,fexists);
			return hmap;
		}
		*/


	};

}

