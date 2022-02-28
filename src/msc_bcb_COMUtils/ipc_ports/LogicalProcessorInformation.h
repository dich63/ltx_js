#pragma once

#include <Winsock2.h>
#include <Mswsock.h>
#include <windows.h>

struct LogicalProcessorInformation_t{

	typedef BOOL (WINAPI *LPFN_GLPI)(
		PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, 
		PDWORD);


	// Helper function to count set bits in the processor mask.
	static	DWORD CountSetBits(ULONG_PTR bitMask)
	{
		DWORD LSHIFT = sizeof(ULONG_PTR)*8 - 1;
		DWORD bitSetCount = 0;
		ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;    
		DWORD i;

		for (i = 0; i <= LSHIFT; ++i)
		{
			bitSetCount += ((bitMask & bitTest)?1:0);
			bitTest/=2;
		}

		return bitSetCount;
	}



	DWORD logicalProcessorCount ;
	DWORD numaNodeCount ;
	DWORD processorCoreCount ;
	DWORD processorL1CacheCount ;
	DWORD processorL2CacheCount ;
	DWORD processorL3CacheCount ;
	DWORD processorPackageCount ;     
	HRESULT err;


	LogicalProcessorInformation_t(){
		PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
		err=HRESULT_FROM_WIN32(init(buffer));
		free(buffer);

	}

inline	HRESULT init(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer){
		logicalProcessorCount = 0;
		numaNodeCount = 0;
		processorCoreCount = 0;
		processorL1CacheCount = 0;
		processorL2CacheCount = 0;
		processorL3CacheCount = 0;
		processorPackageCount = 0;     

		LPFN_GLPI glpi;
		BOOL done = FALSE;		
		PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = NULL;
		DWORD returnLength = 0;

		DWORD byteOffset = 0;
		PCACHE_DESCRIPTOR Cache;

		glpi = (LPFN_GLPI) GetProcAddress(
			GetModuleHandle(TEXT("kernel32")),
			"GetLogicalProcessorInformation");
		if (NULL == glpi) 
		{		
			return (1);
		}

		while (!done)
		{
			DWORD rc = glpi(buffer, &returnLength);

			if (FALSE == rc) 
			{
				if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) 
				{
					if (buffer) 
						free(buffer);

					buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(
						returnLength);

					if (NULL == buffer) 
					{

						return (2);
					}
				} 
				else 
				{

					return (3);
				}
			} 
			else
			{
				done = TRUE;
			}
		}

		ptr = buffer;

		while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength) 
		{
			switch (ptr->Relationship) 
			{
			case RelationNumaNode:
				// Non-NUMA systems report a single record of this type.
				numaNodeCount++;
				break;

			case RelationProcessorCore:
				processorCoreCount++;

				// A hyperthreaded core supplies more than one logical processor.
				logicalProcessorCount += CountSetBits(ptr->ProcessorMask);
				break;

			case RelationCache:
				// Cache data is in ptr->Cache, one CACHE_DESCRIPTOR structure for each cache. 
				Cache = &ptr->Cache;
				if (Cache->Level == 1)
				{
					processorL1CacheCount++;
				}
				else if (Cache->Level == 2)
				{
					processorL2CacheCount++;
				}
				else if (Cache->Level == 3)
				{
					processorL3CacheCount++;
				}
				break;

			case RelationProcessorPackage:
				// Logical processors share a physical package.
				processorPackageCount++;
				break;

			default:
				return -1;
				break;
			}
			byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
			ptr++;
		}

		return 0;


	}

	inline bool is_ht(){
		return processorCoreCount<logicalProcessorCount;
	}
	inline bool is_hyperthreading(){
       return is_ht();
	}
	inline	operator bool(){
		return err==S_OK;
	}

};
