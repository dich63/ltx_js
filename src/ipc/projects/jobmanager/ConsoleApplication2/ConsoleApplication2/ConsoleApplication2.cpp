// ConsoleApplication2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "ipc_ports/pipe_port2.h"
#include <windows.h> 
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>
#include "wchar_parsers.h"
#include "singleton_utils.h"
#include "ipc_ports/job_manager.h"


#define CONNECTING_STATE 0 
#define READING_STATE 1 
#define WRITING_STATE 2 
#define INSTANCES 4 
#define PIPE_TIMEOUT 5000
#define BUFSIZE 4096




//std::recursive_mutex mutex;
JOBOBJECT_NOTIFICATION_LIMIT_INFORMATION klkjkj;

typedef struct
{
	OVERLAPPED oOverlap;
	HANDLE hPipeInst;
	TCHAR chRequest[BUFSIZE];
	DWORD cbRead;
	TCHAR chReply[BUFSIZE];
	DWORD cbToWrite;
	DWORD dwState;
	BOOL fPendingIO;
	ULONG client_pid;
} PIPEINST, *LPPIPEINST;


VOID DisconnectAndReconnect(DWORD);
BOOL ConnectToNewClient(HANDLE, LPOVERLAPPED);
VOID GetAnswerToRequest(LPPIPEINST);

PIPEINST Pipe[INSTANCES];
HANDLE hEvents[INSTANCES];


//typedef HRESULT (*recv_send_proc_t) (void* context,);


static void clear_buf(PIPEINST& pipe){
	memset(pipe.chRequest, 0, BUFSIZE*sizeof(TCHAR));
}

struct asios_t{
	HANDLE hiocp;
	HRESULT hr;

	asios_t(HANDLE h) :hiocp(h){};
	void operator()(){
		BOOL f;
		DWORD nb;
		ULONG_PTR u;
		OVERLAPPED* povl;
		char* sst[] = { "connecting", "reading", "writing" };
		printf("start loop\n");
		while (1)
		{
			f = GetQueuedCompletionStatus(hiocp, &nb, &u, &povl, INFINITE);
			hr = f ? S_OK : GetLastError();
			PIPEINST* pi = (PIPEINST*)povl;
			if (pi)
			{
				printf("\n\ [%d]{%s} nb=%d; err=%x\n\n", pi->hPipeInst, sst[pi->dwState], nb, hr);
			}
			else printf("\n\ERROR nb=%d; err=%x\n\n",  nb, hr);
		}
	}
};

struct tete_t;

typedef std::map<std::wstring, tete_t> mtete_t;
struct tete_t
{
	mtete_t::iterator i;
	mtete_t* pmm;


	void remove(){
		//auto t=i;
		//auto p = pmm;
		pmm->erase(i);
	}
	static bool create(mtete_t* pm, const std::wstring& name, tete_t** ppt )
	{
		bool f;
		auto i = pm->insert(std::make_pair(name, tete_t()));
		auto j = i.first;
		tete_t& t = j->second;
		if (f = i.second)
		{			
			t.i = j;
			t.pmm = pm;
		}

		*ppt = &t;
		return f;
	}
};


typedef std::map<std::wstring, int> mm_t;
typedef  mm_t::iterator imm_t,*pimm_t;
PERFORMANCE_INFORMATION pi;

//int on_send_recv(unsigned long id, char* bufin, int cbr, char* bufout, int* pcbw, HANDLE hAbortEvent)
int cdecl  on_send_recv(void* pcontext, unsigned long id, char* bufin, int cbr, char* bufout, int* pcbw, HANDLE hAbortEvent)
{
	
	if (cbr) memcpy(bufout, bufin, *pcbw = cbr);
	return 1;
}


HRESULT job_manager_main(wchar_t* pipename = L"\\\\.\\pipe\\jobmanager"){

	HRESULT hr=0;
	
	HANDLE hmutex = CreateMutexW(0, 0,L"D0BF1FF31A094DE4A79DEB4425A3A5A0");
	if (!hmutex) return hr = GetLastError();

	if ((hr = GetLastError())==ERROR_ALREADY_EXISTS)
		return hr;

	

	typedef pipe_port<0, 2, 4 * 1024>  pipe_port_t;

	job_manager_t<>::log_proc_t pp= job_manager_t<>::log_proc();
	job_manager_t<> job_manager;
	pipe_port_t::thread_pair tp = pipe_port_t::create_thread_instance(pipename, &job_manager_t<>::s_on_send_recv, &job_manager);
	job_manager.loop_forever();
	return hr;
}



int _tmain(VOID)
{
	HRESULT hr;
	VARIANT v = { VT_R8 | VT_BYREF }, vr = { VT_R8 | VT_BYREF };
	double r = -111.2223;
	long long ir;
	v.byref = &r;
	vr.byref = &ir;
	hr = VariantChangeType(&vr, &v, 0, VT_I8 );

	job_manager_main();
	return 0 ;

	
	BOOL f;
	job_manager_t<> job_manager;

	argv_zzs<wchar_t> argv,argout;
	event_local_t ev;

		argv[L"fulltime"]=2;
		argv[L"memlimit"] = 10000;
		argv[L"handle"]=UINT_PTR(ev.hko);
		//argv[L"pid"] = GetCurrentProcessId();
		argv[L"cmd"] = L"quota-set";
		argv[L"pcount"] =3;

		
		hr = job_manager.job_create(argv, argout, GetCurrentProcessId());

		const wchar_t* jobname = argout[L"jobname"];

		job_manager_t<>::string_t sjn = jobname;

/*		auto pjob = job_manager[jobname];

		f = AssignProcessToJobObject(pjob->hjob, GetCurrentProcess());

		hr = GetLastError();


		job_manager.loop_forever();

		///hr = job_manager.job_create(argv, argout, GetCurrentProcessId());
		///hr = job_manager.job_create(argv, argout, GetCurrentProcessId());


		//delete pjob;
		Sleep(4000);
		ev.signal(1);
		Sleep(1000);

		*/

	
	bool fff;
	fff = ::GetPerformanceInfo(&pi, sizeof(pi));

	typedef std::list<int> processor_list_t;




	int ii[4], ii0[4] ;
	CoCreateGuid((GUID*)ii);
	CoCreateGuid((GUID*)ii0);
	/*
	processor_list_t pl,pl2;

	pl.push_back(1);
	pl.push_back(2);
	pl.push_back(3);
	pl.push_back(4);

	processor_list_t::iterator ip=pl.begin(),jp;

	bool ff = jp == ip;
	//pl.erase(jp);

	//std::next
	int k = pl.size();
	jp = std::next(ip, 3);



	*/

	//<bool fremoted = false, int INSTANCES = 2, size_t _BUFSIZE
	typedef pipe_port<0,2,4*1024>  pipe_port_t;

	pipe_port_t::thread_pair tp = pipe_port_t::create_thread_instance("\\\\.\\pipe\\mynamedpipe", &on_send_recv, 0);


	WaitForSingleObject(tp,INFINITE);



	 fff = JOB_OBJECT_MSG_ABNORMAL_EXIT_PROCESS;
	


	int ccc=sizeof(JOBOBJECT_NOTIFICATION_LIMIT_INFORMATION);
	int ccfo = offsetof(JOBOBJECT_NOTIFICATION_LIMIT_INFORMATION, LimitFlags);
	SIZE_T miws, maws;
	fff = GetProcessWorkingSetSize(GetCurrentProcess(), &miws, &maws);




	mtete_t mtt;

	//tete_t& tttt=mtt[L"ss"];

	auto itt = mtt.insert(std::make_pair(L"ss", tete_t()));
	auto jtt = itt.first;
	tete_t&yy = jtt->second;
	yy.i = jtt;
	yy.pmm = &mtt;
	


	tete_t& uuu = mtt[L"ss"];


	tete_t* pt;
	 fff = tete_t::create(&mtt,L"UIUIU",&pt);

	pt->remove();
	int hhhh = sizeof(tete_t);



	uuu.remove();

	//tete_t&yy = *(itt.first.second);
		//yy.i = itt;

	mm_t mm;
	mm[L"ss"] = 10;
	mm[L"aa"] = 12;

	auto imm = mm.begin();


	void * ppp = &(*imm);

	mm.erase(imm);

	//void * ppp = (void *)(imm);
	

	HANDLE hiocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	DWORD i, dwWait, cbRet;
	HRESULT dwErr;
	BOOL fSuccess;
	LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\mynamedpipe");

	asyn_call(new asios_t(hiocp));
	Sleep(3000);

	// The initial loop creates several instances of a named pipe 
	// along with an event object for each instance.  An 
	// overlapped ConnectNamedPipe operation is started for 
	// each instance. 

	for (i = 0; i < INSTANCES; i++)
	{

		// Create an event object for this instance. 

		hEvents[i] = CreateEvent(
			NULL,    // default security attribute 
			TRUE,    // manual-reset event 
			TRUE,    // initial state = signaled 
			NULL);   // unnamed event object 

		if (hEvents[i] == NULL)
		{
			printf("CreateEvent failed with %d.\n", GetLastError());
			return 0;
		}
		Pipe[i].client_pid = 0;
		Pipe[i].oOverlap.hEvent = hEvents[i];

		Pipe[i].hPipeInst = CreateNamedPipe(
			lpszPipename,            // pipe name 
			PIPE_ACCESS_DUPLEX |     // read/write access 
			FILE_FLAG_OVERLAPPED,    // overlapped mode 
			PIPE_TYPE_MESSAGE |      // message-type pipe 
			PIPE_READMODE_MESSAGE |  // message-read mode 
			PIPE_WAIT,               // blocking mode 
			INSTANCES,               // number of instances 
			BUFSIZE*sizeof(TCHAR),   // output buffer size 
			BUFSIZE*sizeof(TCHAR),   // input buffer size 
			PIPE_TIMEOUT,            // client time-out 
			NULL);                   // default security attributes 

		if (Pipe[i].hPipeInst == INVALID_HANDLE_VALUE)
		{
			printf("CreateNamedPipe failed with %d.\n", GetLastError());
			return 0;
		}

		
		hr = CreateIoCompletionPort(Pipe[i].hPipeInst, hiocp, 1000+i, 0) ? S_OK : GetLastError();

		// Call the subroutine to connect to the new client

		Pipe[i].fPendingIO = ConnectToNewClient(
			Pipe[i].hPipeInst,
			&Pipe[i].oOverlap);

		Pipe[i].dwState = Pipe[i].fPendingIO ?
		CONNECTING_STATE : // still connecting 
						   READING_STATE;     // ready to read 
	}

	while (1)
	{
		// Wait for the event object to be signaled, indicating 
		// completion of an overlapped read, write, or 
		// connect operation. 

		dwWait = WaitForMultipleObjects(
			INSTANCES,    // number of event objects 
			hEvents,      // array of event objects 
			FALSE,        // does not wait for all 
			INFINITE);    // waits indefinitely 

		// dwWait shows which pipe completed the operation. 

		i = dwWait - WAIT_OBJECT_0;  // determines which pipe 
		if (i < 0 || i >(INSTANCES - 1))
		{
			printf("Index out of range.\n");
			return 0;
		}

		// Get the result if the operation was pending. 

		if (Pipe[i].fPendingIO)
		{
			fSuccess = GetOverlappedResult(
				Pipe[i].hPipeInst, // handle to pipe 
				&Pipe[i].oOverlap, // OVERLAPPED structure 
				&cbRet,            // bytes transferred 
				FALSE);            // do not wait 

			switch (Pipe[i].dwState)
			{
				// Pending connect operation 
			case CONNECTING_STATE:
				if (!fSuccess)
				{
					printf("Error %d.\n", GetLastError());
					return 0;
				}
				Pipe[i].dwState = READING_STATE;
				break;

				// Pending read operation 
			case READING_STATE:
				if (!fSuccess || cbRet == 0)
				{
					DisconnectAndReconnect(i);
					continue;
				}
				Pipe[i].dwState = WRITING_STATE;
				break;

				// Pending write operation 
			case WRITING_STATE:
				if (!fSuccess || cbRet != Pipe[i].cbToWrite)
				{
					DisconnectAndReconnect(i);
					continue;
				}
				Pipe[i].dwState = READING_STATE;
				break;

			default:
			{
				printf("Invalid pipe state.\n");
				return 0;
			}
			}
		}

		// The pipe state determines which operation to do next. 

		switch (Pipe[i].dwState)
		{
			// READING_STATE: 
			// The pipe instance is connected to the client 
			// and is ready to read a request from the client. 

		case READING_STATE:
			//clear_buf(Pipe[i]);
			Pipe[i].client_pid;
			fSuccess = GetNamedPipeClientProcessId(Pipe[i].hPipeInst, &Pipe[i].client_pid);

			fSuccess = ReadFile(
				Pipe[i].hPipeInst,
				Pipe[i].chRequest,
				BUFSIZE*sizeof(TCHAR),
				&Pipe[i].cbRead,
				&Pipe[i].oOverlap);

			// The read operation completed successfully. 
			//...

			if (fSuccess && Pipe[i].cbRead != 0)
			{
				Pipe[i].fPendingIO = FALSE;
				Pipe[i].dwState = WRITING_STATE;
				continue;
			}
			else {
				hr = 01;
			};

			// The read operation is still pending. 

			dwErr = GetLastError();
			if (!fSuccess && (dwErr == ERROR_IO_PENDING))
			{
				Pipe[i].fPendingIO = TRUE;
				continue;
			}

			// An error occurred; disconnect from the client. 

			DisconnectAndReconnect(i);
			break;

			// WRITING_STATE: 
			// The request was successfully read from the client. 
			// Get the reply data and write it to the client. 

		case WRITING_STATE:
			GetAnswerToRequest(&Pipe[i]);

			fSuccess = WriteFile(
				Pipe[i].hPipeInst,
				Pipe[i].chReply,
				Pipe[i].cbToWrite,
				&cbRet,
				&Pipe[i].oOverlap);

			// The write operation completed successfully. 

			if (fSuccess && cbRet == Pipe[i].cbToWrite)
			{
				Pipe[i].fPendingIO = FALSE;
				Pipe[i].dwState = READING_STATE;
				continue;
			}

			// The write operation is still pending. 

			dwErr = GetLastError();
			if (!fSuccess && (dwErr == ERROR_IO_PENDING))
			{
				Pipe[i].fPendingIO = TRUE;
				continue;
			}

			// An error occurred; disconnect from the client. 

			DisconnectAndReconnect(i);
			break;

		default:
		{
			printf("Invalid pipe state.\n");
			return 0;
		}
		}
	}

	return 0;
}


// DisconnectAndReconnect(DWORD) 
// This function is called when an error occurs or when the client 
// closes its handle to the pipe. Disconnect from this client, then 
// call ConnectNamedPipe to wait for another client to connect. 

VOID DisconnectAndReconnect(DWORD i)
{
	// Disconnect the pipe instance. 

	if (!DisconnectNamedPipe(Pipe[i].hPipeInst))
	{
		printf("DisconnectNamedPipe failed with %d.\n", GetLastError());
	}

	// Call a subroutine to connect to the new client. 

	Pipe[i].fPendingIO = ConnectToNewClient(
		Pipe[i].hPipeInst,
		&Pipe[i].oOverlap);

	Pipe[i].dwState = Pipe[i].fPendingIO ?
	CONNECTING_STATE : // still connecting 
					   READING_STATE;     // ready to read 
}

// ConnectToNewClient(HANDLE, LPOVERLAPPED) 
// This function is called to start an overlapped connect operation. 
// It returns TRUE if an operation is pending or FALSE if the 
// connection has been completed. 

BOOL ConnectToNewClient(HANDLE hPipe, LPOVERLAPPED lpo)
{
	BOOL fConnected, fPendingIO = FALSE;

	// Start an overlapped connection for this pipe instance. 
	fConnected = ConnectNamedPipe(hPipe, lpo);

	// Overlapped ConnectNamedPipe should return zero. 
	if (fConnected)
	{
		printf("ConnectNamedPipe failed with %d.\n", GetLastError());
		return 0;
	}

	switch (GetLastError())
	{
		// The overlapped connection in progress. 
	case ERROR_IO_PENDING:
		fPendingIO = TRUE;
		break;

		// Client is already connected, so signal an event. 

	case ERROR_PIPE_CONNECTED:
		if (SetEvent(lpo->hEvent))
			break;

		// If an error occurs during the connect operation... 
	default:
	{
		printf("ConnectNamedPipe failed with %d.\n", GetLastError());
		return 0;
	}
	}

	return fPendingIO;
}

VOID GetAnswerToRequest(LPPIPEINST pipe)
{

	//pipe->chRequest[pipe->cRead] = 0;
	argv_zzs<char> argin(pipe->cbRead,pipe->chRequest);
	argin[char_mutator<CP_UTF8>(L"Õåð")] = char_mutator<CP_UTF8>(L"Ï¸ð");
	int cb;
   const	char* p = argin.flat_str(false, "\n",&cb);
   char_mutator<CP_UTF8> cm(p);

   //wprintf(L"[pid=%d][cb=%d] %s\n", pipe->client_pid, pipe->cbRead, (wchar_t*)cm);
	//StringCchCopy(pipe->chReply, BUFSIZE, TEXT("Default answer from server"));
	memcpy(pipe->chReply, p, cb);
	pipe->cbToWrite = cb;
	//	pipe->cbToWrite = (lstrlen(pipe->chReply) + 1)*sizeof(TCHAR);
}


