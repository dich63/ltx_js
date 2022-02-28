#pragma once

#define PIPE_TIMEOUT 5000
#define BUFSIZE 4096

typedef int (*SEND_RECIEVED_PROC_TYPE)(void* pcntx,int err,int cbread,char *read_buf,int cbwritesize,int* pcbwrite,char *write_buf);
typedef int (*PROCESS_MESSAGE_TYPE)(void* pcntx);

struct SEND_RECIEVED
{

	int cbRead;
	int cbToWrite; 
	char chRequest[BUFSIZE]; 
	char chReply[BUFSIZE]; 
};

struct named_pipe_server_base{};
typedef named_pipe_server_base* hnpsrv;


extern "C" hnpsrv named_pipe_server_initialize
	(char* pipename,
	bool remoted_share,
	SEND_RECIEVED_PROC_TYPE srp,
	void* pcntx_srp,
	PROCESS_MESSAGE_TYPE pmp,
	void* pcntx_pmp); 

extern "C" long named_pipe_server_run(hnpsrv h);
extern "C" long named_pipe_server_stop(hnpsrv h,long* pifzero);
extern "C" long named_pipe_server_delete(hnpsrv h);
extern "C" long named_pipe_send_recieved(char* pipename,int cbread,void* prbuf,int* cbwrite,void* pwbuf);