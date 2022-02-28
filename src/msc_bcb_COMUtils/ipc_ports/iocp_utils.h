#pragma once

#include <Winsock2.h>
#include <Mswsock.h>
#include <windows.h>

#include "static_constructors_0.h"

#define SET_PROC_ADDRESS(h,a) __set_dll_proc(h,(void**)&a,#a)

namespace iocp_utils {

template<int VERS=0>
struct iocp_t{


	typedef LONG NTSTATUS;

	typedef struct _FILE_COMPLETION_INFORMATION {
		HANDLE Port;
		PVOID  Key;
	} FILE_COMPLETION_INFORMATION, *PFILE_COMPLETION_INFORMATION;

	typedef struct _IO_STATUS_BLOCK {
		union {
			NTSTATUS Status;
			PVOID Pointer;
		};
		ULONG_PTR Information;
	} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

	typedef enum _FILE_INFORMATION_CLASS {
		FileDirectoryInformation = 1,
		FileFullDirectoryInformation,
		FileBothDirectoryInformation,
		FileBasicInformation,
		FileStandardInformation,
		FileInternalInformation,
		FileEaInformation,
		FileAccessInformation,
		FileNameInformation,
		FileRenameInformation,
		FileLinkInformation,
		FileNamesInformation,
		FileDispositionInformation,
		FilePositionInformation,
		FileFullEaInformation,
		FileModeInformation,
		FileAlignmentInformation,
		FileAllInformation,
		FileAllocationInformation,
		FileEndOfFileInformation,
		FileAlternateNameInformation,
		FileStreamInformation,
		FilePipeInformation,
		FilePipeLocalInformation,
		FilePipeRemoteInformation,
		FileMailslotQueryInformation,
		FileMailslotSetInformation,
		FileCompressionInformation,
		FileObjectIdInformation,
		FileCompletionInformation,
		FileMoveClusterInformation,
		FileQuotaInformation,
		FileReparsePointInformation,
		FileNetworkOpenInformation,
		FileAttributeTagInformation,
		FileTrackingInformation,
		FileIdBothDirectoryInformation,
		FileIdFullDirectoryInformation,
		FileValidDataLengthInformation,
		FileShortNameInformation,
		FileIoCompletionNotificationInformation,
		FileIoStatusBlockRangeInformation,
		FileIoPriorityHintInformation,
		FileSfioReserveInformation,
		FileSfioVolumeInformation,
		FileHardLinkInformation,
		FileProcessIdsUsingFileInformation,
		FileNormalizedNameInformation,
		FileNetworkPhysicalNameInformation,
		FileIdGlobalTxDirectoryInformation,
		FileIsRemoteDeviceInformation,
		FileUnusedInformation,
		FileNumaNodeInformation,
		FileStandardLinkInformation,
		FileRemoteProtocolInformation,
		FileRenameInformationBypassAccessCheck,
		FileLinkInformationBypassAccessCheck,
		FileVolumeNameInformation,
		FileIdInformation,
		FileIdExtdDirectoryInformation,
		FileReplaceCompletionInformation,
		FileHardLinkFullIdInformation,
		FileIdExtdBothDirectoryInformation,
		FileMaximumInformation
	} FILE_INFORMATION_CLASS, *PFILE_INFORMATION_CLASS;
/*
	typedef NTSTATUS(NTAPI *SetInformationFile_t)
		(HANDLE FileHandle,
		PIO_STATUS_BLOCK IoStatusBlock,
		PVOID FileInformation,
		ULONG Length,
		FILE_INFORMATION_CLASS FileInformationClass);
*/
	typedef NTSTATUS(NTAPI *SetInformationFile_t)
		(HANDLE FileHandle,
		PIO_STATUS_BLOCK IoStatusBlock,
		PVOID FileInformation,
		ULONG Length,
		FILE_INFORMATION_CLASS FileInformationClass);

	typedef NTSTATUS(NTAPI *QueryInformationFile_t)
		(HANDLE FileHandle,
		PIO_STATUS_BLOCK IoStatusBlock,
		PVOID FileInformation,
		ULONG Length,
		FILE_INFORMATION_CLASS FileInformationClass);




	static HINSTANCE _get_hinstance(const void* p)
	{
		HINSTANCE h = 0;
		GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
			GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)p, &h);
		return h;
	}


	static void* __set_dll_proc(const void*  p, void** fp, char* name)
	{
		HINSTANCE h = _get_hinstance(p);
		return (h) ? (*fp = (void*)GetProcAddress(h, name)) : NULL;
	}


	iocp_t():hr(E_POINTER){

		;
		BOOL f;
		HMODULE hm = GetModuleHandleA("ntdll.dll");
		f=!!hm;
		f&=!!SET_PROC_ADDRESS(hm, NtSetInformationFile);
		f&=!!SET_PROC_ADDRESS(hm, NtQueryInformationFile);
		
		 hr=(f)?S_OK:HRESULT_FROM_WIN32(GetLastError());

	}


	HRESULT clearIOCP(HANDLE h)
	{
		IO_STATUS_BLOCK IoStatusBlock = {};
		FILE_COMPLETION_INFORMATION  fci = {0,0};

		HRESULT st=hr;
		if(!st)
		{
		 st=NtSetInformationFile(h, &IoStatusBlock, &fci, sizeof(fci), FileReplaceCompletionInformation);
		}
		return st;
	}

     HRESULT clearIOCP(SOCKET s)
	 {
		 return clearIOCP(HANDLE(s));
	 }
	
	 inline static iocp_t& get_instance()
	 {
		 return class_initializer_T<iocp_t>().get();
	 }


    HRESULT hr;
	SetInformationFile_t NtSetInformationFile;
	QueryInformationFile_t NtQueryInformationFile;

};



}