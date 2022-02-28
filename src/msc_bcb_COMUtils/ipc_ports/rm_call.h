#pragma once
#include <Windows.h>

namespace remote_call {

	struct  rp_IsDebugPresent_t {


		rp_IsDebugPresent_t(DWORD pid) :hrmp(0), paddr(0), hr(0) {


			HMODULE hm = ::GetModuleHandleW(L"Kernel32");

			if (hm) {

				paddr = ::GetProcAddress(hm, "IsDebuggerPresent");

				if (paddr) {
					hrmp = OpenProcess(
						PROCESS_QUERY_INFORMATION |   // Required by Alpha
						PROCESS_CREATE_THREAD |   // For CreateRemoteThread
						PROCESS_VM_OPERATION |   // For VirtualAllocEx/VirtualFreeEx
						PROCESS_VM_READ |
						SYNCHRONIZE |
						PROCESS_VM_WRITE,             // For WriteProcessMemory
						FALSE, pid);


				}
			}
			if (!hrmp) hr = GetLastError();
		};

		inline bool operator ()()
		{

			BOOL  rs = 0;			

			if (!hr) {

				::CheckRemoteDebuggerPresent(hrmp, &rs);
			};

    		return rs;

			/*
			DWORD tid, res = 0;
			if (!hr) {
				HANDLE hrt = ::CreateRemoteThread(hrmp, 0, 0, LPTHREAD_START_ROUTINE(paddr), (void*)0, 0, &tid);
				if (hrt)
				{
					::WaitForSingleObject(hrt, INFINITE);
					::GetExitCodeThread(hrt, &res);
					::CloseHandle(hrt);
				}
				else {
					hr = GetLastError();
				}
			}
			return res;
			*/
		}


		inline bool isterm() {
			/*
			DWORD dw=0;
			if(!h) return false;
			return GetExitCodeProcess(h,&dw), !(STILL_ACTIVE==dw);
			*/
			return (!hrmp) || (WAIT_OBJECT_0 == WaitForSingleObject(hrmp, 0));

		}





		~rp_IsDebugPresent_t() {
			::CloseHandle(hrmp);
		}


		HRESULT hr;
		void* paddr;
		HANDLE hrmp;

	};
};// namespace remote_call