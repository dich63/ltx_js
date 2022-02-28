#pragma once

#include <windows.h>
#include <restartmanager.h>
#include<vector>

#pragma comment(lib, "rstrtmgr.lib")

#include <string>
#include <sstream>


namespace rm_tools {

	template<int V=0>
	struct rm_t {


		struct up_t {
			up_t(HANDLE hp = INVALID_HANDLE_VALUE) {

				FILETIME ct, t;
				r.dwProcessId = ::GetProcessId(hp);
				if (::GetProcessTimes(hp, &ct, &t, &t, &t)) {
					r.ProcessStartTime = ct;
				}

			}

			operator RM_UNIQUE_PROCESS* () {
				return &r;
			}

			RM_UNIQUE_PROCESS r = {};

		};


		inline std::wstring  getfullname(DWORD pid) {
			wchar_t buf[4096];
			DWORD sz = 4096;
			HANDLE hp = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, 0, pid);

			if (hp) {
				if (::QueryFullProcessImageNameW(hp, 0, buf, &sz)) {
					return  (wchar_t*)buf;
				}

				::CloseHandle(hp);
			}

			return L"";
		}


		inline  bool rmgetlist() {

			UINT nNeeded = 0, nAffected = 0;

			if ((S_OK == hr) && (ERROR_MORE_DATA == (hr = ::RmGetList(dwSessionHandle, &nNeeded, &nAffected, nullptr, (LPDWORD)&dwRebootReasons)))) {
				vpi.resize(nAffected = nNeeded);
				hr = ::RmGetList(dwSessionHandle, &nNeeded, &nAffected, vpi.data(), (LPDWORD)&dwRebootReasons);
			}
			return S_OK == hr;
		}



		rm_t(const wchar_t** rgsFiles, size_t len) {
			bool f;
			f = (S_OK == (hr = ::RmStartSession(&dwSessionHandle, 0, sessKey)));
			f = f && (S_OK == (hr = ::RmRegisterResources(dwSessionHandle, len, rgsFiles, 0, NULL, 0, NULL)));
			f = f && (S_OK == (hr = ::RmAddFilter(dwSessionHandle, NULL, rm_t::up_t(), NULL, RmNoShutdown)));
			f = f && rmgetlist();
		}

		~rm_t() {

			(dwSessionHandle == 0xFFFFFFFF) || ::RmEndSession(dwSessionHandle);

		}

		std::wstring log(const wchar_t* fmt = L"[%d]  %s : \"%s\"%s", const wchar_t delim = L'\n') {
			wchar_t buf[4096];


			if (fmt) {

				const wchar_t* cm[2] = { L" -",L" +" };
				std::wostringstream stream;

				for (auto n = 0; n < vpi.size(); n++) {

					RM_PROCESS_INFO& pi = vpi[n];
					DWORD pid = pi.Process.dwProcessId;
					auto fn = getfullname(pid);
					int i = (pi.AppStatus & RmStatusShutdownMasked) ? 0 : 1;
					swprintf_s(buf, fmt, pid, pi.strAppName, fn.c_str(), cm[i]);
					if (n) {
						stream << delim;
					}
					stream << (wchar_t*)buf;
				}
				return stream.str();
			}
			return L"";

		}

		HRESULT shutdown(RM_WRITE_STATUS_CALLBACK rmcb = nullptr) {
			(S_OK == hr) && (hr = ::RmShutdown(dwSessionHandle, RmForceShutdown, rmcb));
			return hr;
		}

		HRESULT terminate(DWORD excode = 0) {

			HRESULT hrc = S_OK;
			auto cpi = ::GetCurrentProcessId();
			HANDLE hp;
			for (auto n = 0; n < vpi.size(); n++) {
				DWORD i = vpi[n].Process.dwProcessId;
				if ((i != cpi) && (hp = ::OpenProcess(PROCESS_TERMINATE, 0, i))) {
					if (!::TerminateProcess(hp, excode)) {
						hrc = GetLastError();
					}
					::CloseHandle(hp);
				}
			}

			return hrc;

		}


		DWORD dwRebootReasons = RmRebootReasonNone;
		WCHAR sessKey[CCH_RM_SESSION_KEY + 1];
		DWORD dwSessionHandle = 0xFFFFFFFF;
		//DWORD pid_mask;
		HRESULT hr = E_FAIL;
		std::vector<RM_PROCESS_INFO> vpi;
	};



}; //namespace rm_tools