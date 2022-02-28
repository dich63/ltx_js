#pragma once
#include "video/webcam_capture.h"
#include "video/lasercalibrate.h"
#include "ipc_ports/hssh.h"
//#include "AviSynth.h"
#include "AviSynth_templates.h"



extern HANDLE hGlobalEventAbort;

template <class Version=version>
class Avisyth_hss_map : public IClipMap_impl<Version> {

	
	VideoInfo vi;
	PVideoFrame frame;
	typedef Avisyth_hss_map owner_t;
	hssh_t hssh;
	void* hport,*pclosure;
	v_buf<char> callback_name;
	IScriptEnvironment* m_env;

public:


	struct sockgetter
	{
		SOCKET s;
		sockgetter(SOCKET _s=INVALID_SOCKET):s(_s){};
		inline int operator()(void* p,int count,int flags=0) const
		{
			return ::recv(s,(char*)p,count,flags);
		}
	private:
		sockgetter(sockgetter& a){};
		sockgetter(const sockgetter& a){};
	};


	static void __stdcall s_on_accept(owner_t* p, HSS_INFO* hovl)
	{
		p->on_accept(hovl);
	}







    inline void set_map_values(argv_zz<char>& args,const char* ra=0)
	{
		

		if(args.argc<=0) return;

        v_buf<char> buf;
		
		locker_t<Avisyth_hss_map> lock(this);
		avs_base<Version>::log_printf("set %d values from %s\n",args.argc,(ra)?ra:"??");
		
		for(int n=0;n<args.argc;++n)
		{

			const char* name=args.names(n,buf);
			double val=args[name];
			set_value(name,val);
			avs_base<Version>::log_printf("%s=%g\n",name,val);
		}

		if(callback_name)
		{
			AVSValue args[2] = {this};
			AVSValue  res=m_env->Invoke(callback_name.get(), AVSValue(args,1));
						
		}


	}


	inline void  on_accept(HSS_INFO* hovl)
	{
		if(!hovl) return;

		long lh;
		lh=hssh.heap_count();

		char* sn= hssh.reset_SOCKET_DATA(hovl,HSD_LOCAL_INFO,HSF_GET,0,0);
		char* pn= hssh.reset_SOCKET_DATA(hovl,HSD_REMOTE_INFO,HSF_GET,0,0);
		avs_base<Version>::log_printf("on_accept:: sockname=(%s)  peername=(%s)\n",sn,pn);
		ULONG st=(ULONG)hssh.reset_SOCKET_DATA(hovl,HSD_STATE,HSF_GET,0,0);
		sockgetter sget(hovl->s);
		{
            char* prm=hssh.reset_SOCKET_DATA(hovl,HSD_PARAMS,HSF_GET,0,0); 
			argv_zz<char> args_connect(prm);
			set_map_values(args_connect,pn);
			
				
		}
		
		//socksetter sset(hovl->s);
	if(st&(1<<HSD_REPLY)) 
		while(1)
		{

			std::strstream stream_in,stream_out;
			int cb;

			if((cb=get_wbs_string(stream_in,sget))<=0)
				break;

			   argv_zz<char> args_in(stream_in.str());
			   set_map_values(args_in,pn);
		   
		}

     avs_base<Version>::log_printf("disconnect:: sockname=(%s)  peername=(%s)\n",sn,pn);
		
	}


inline PVideoFrame&	  current_frame()
{
   return frame;
}

	~Avisyth_hss_map() {
           
		long lh;
		lh=hssh.heap_count();
		hssh.release(hport);
		lh=hssh.heap_count();
		hssh.release(pclosure);
		lh=hssh.heap_count();
		

	}

	Avisyth_hss_map(const char* portname,const char* _callbackname,IScriptEnvironment* env)
	{

   if(1)
   {
		if(safe_len(_callbackname))
			callback_name.cat(_callbackname);
		if(!hssh.fok)
		           env->ThrowError("hs_socket.dll not found");

		hssh.lock_forever();

         m_env=env;
		 pclosure=hssh.create_stdcall_closure(&s_on_accept,this);
		if(pclosure)
		{
			hport=hssh.create_socket_port((char*)portname,HSF_SRD_AUTO,pclosure,HSD_SAME_PTR);
			//
			hssh.release(pclosure);
		}
		if((pclosure==0)||(hport==0))
		{
			HRESULT he=hssh.error();
			env->ThrowError("hss_map error=%s",(char*)char_mutator<CP_THREAD_ACP>(error_msg(he)));
		}
		
   }
		memset(&vi, 0, sizeof(VideoInfo));
		vi.width = 32;
		vi.height = 32;
		vi.fps_numerator = 25*1000;
		vi.fps_denominator = 1001;
		vi.num_frames = 3*107892;   // 1 hour
		vi.pixel_type = VideoInfo::CS_BGR32;


		frame = env->NewVideoFrame(vi);
		
	}

	
	
	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env) { return frame; }

	
	bool __stdcall GetParity(int n) { return false; }
	const VideoInfo& __stdcall GetVideoInfo() { return vi; }
	void __stdcall SetCacheHints(int cachehints,int frame_range) { };

	void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) { };

	static AVSValue __cdecl Create(AVSValue args, void*, IScriptEnvironment* env) {
		return new Avisyth_hss_map(args[0].AsString(""),args[1].AsString(""), env);
	}
};
