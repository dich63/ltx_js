#pragma once

#include "AviSynth.h"
#include <map>
#include <hash_map>
#include <string>
#include <wchar_parsers.h>
#include <math.h>
#include "singleton_utils.h"
#include <limits>

#define in_interval(left,x,right) ((left<=x)&&(x<right))

struct version
{
	enum{
		N=0
	};
};

//void __cdecl *_GMapShutdownFunc(void* user_data, IScriptEnvironment* env);

struct IClipMap:IClip
{
   GUID m_uuid;

virtual AVSValue get_value(const char* name,AVSValue dflv=AVSValue())=0;
virtual void set_value(const char* name,AVSValue dflv=AVSValue())=0;
virtual AVSValue invoke_op(const char* name,int argc,AVSValue args,IScriptEnvironment* env)=0;
virtual long lock()=0;
virtual long unlock()=0;

inline static const GUID& get_uiid()
{
	// {A095B26D-1335-4532-BBE5-B4A813ABD167}
	static const GUID s = { 0xa095b26d, 0x1335, 0x4532, { 0xbb, 0xe5, 0xb4, 0xa8, 0x13, 0xab, 0xd1, 0x67 } };
	return s;
	
};


inline static AVSValue& last_AVSValue_ref()
{
	// {A095B26D-1335-4532-BBE5-B4A813ABD167}
	static AVSValue last;
	return last;
};


inline static AVSValue s_last_AVSValue(AVSValue, void* , IScriptEnvironment* )
{
	// {A095B26D-1335-4532-BBE5-B4A813ABD167}
	 AVSValue& t=last_AVSValue_ref();
      AVSValue l=t;
	  //t=AVSValue();
	  IClip* pp=(IClip*)(void*)l.AsClip();
	return l;
};


inline bool check()
{
  __try
  {
   return (this!=0)&&(get_uiid()==m_uuid);
  }
  __except(EXCEPTION_EXECUTE_HANDLER)
  { }
  return false;
}

inline bool check_raise(IScriptEnvironment* env,const char* msg="Токма наши плагины маппят!!!")
{
	if(!check()) env->ThrowError(msg);
	return true;
}


inline static IClipMap* get_intf(PClip pc,IScriptEnvironment* env)
{
	struct GVF: public IClip {
		PClip child;
		VideoInfo vi;
	};

     IClipMap* pp=(IClipMap*) ((void*)pc);

	 if(!pp->check())
	 {
          GVF* pgf=(GVF*)pp;
		    pp=(IClipMap*) ((void*)pgf->child);
	 }
	 
	 pp->check_raise(env);
	 return pp;

}

static AVSValue s_atan2(AVSValue args, void* user_data, IScriptEnvironment* env)
{
	 AVSValue v=args[0];
	 AVSValue v1=args[1];
	 AVSValue v2=args[2];
     double y=v1.AsFloat(0);
	 v=args[1];
	 double x=1;
		 if( v2.IsFloat()) x=v2.AsFloat(1);
	 double r=atan2(y,x);
	 return AVSValue(r);
}
static AVSValue s_set_value(AVSValue args, void* user_data, IScriptEnvironment* env)
{
	int narg=args.ArraySize();
	//IClipMap* p= (IClipMap*) ((void*) args[0].AsClip());

/*
	const char* pn = args[1].AsString("");
     AVSValue a0=args[0];
	

	if(a0.IsInt())
	{
        int i=a0.AsInt(-1);
		if(i==0)		a0=last_AVSValue_ref();
	}
//	int i=a0.AsInt(-1);
//	
*/
	IClipMap* p=get_intf(args[0].AsClip(),env);


	p->set_value(args[1].AsString(""),args[2]);
	//
	//return args[2];
	//
	return p;
};

static inline bool _get_as_const(const char* name,AVSValue& val )
{
    char* pend=((char*)name)+strlen(name);
    char *p=0; 
    double d=strtod(name,&p);
	if(p!=pend) return false;
     val=AVSValue(d);
	return true;   
}

inline AVSValue _get_val(const char* name,IScriptEnvironment* env){
        
	  bool f;
         if(name[0]=='.')
		 {
			return get_value(name+1);
		 }
		 else
		 {
			 if(StrStrIA(name,"global.")) name+=7;

			  return  env->GetVar(name);
		 }
}

inline bool _set_val(const char* name,IScriptEnvironment* env,AVSValue& val){

	bool f;
	if(name[0]=='.')
	{
		 set_value(name+1,val);
		 return true;
	}
	else
	{
		if(StrStrIA(name,"global."))
		{	
			f = env->SetGlobalVar(name+7,val);
		}
		else f= env->SetVar(name,val);
	}
	return f;
}


static AVSValue s_map_context(AVSValue args, void* user_data, IScriptEnvironment* env)
{
          
       IClipMap* p=get_intf(args[0].AsClip(),env);
	   const char* parse_str=args[1].AsString("");
	   argv_zz_trunc<char> argzt(parse_str,';');

	   locker_t<IClipMap> lock(p); 

	   for(int n=0;n < argzt.argc ; n++ ){
			
		   v_buf<char> buf;
		    char* argname=argzt.names(n,buf);
		    char* argval=argzt[argname];
            AVSValue val;
            if(!p->_get_as_const(argval,val)) 
		             val=p->_get_val(argval,env);
		   p->_set_val(argname,env,val);
		   	   //if((arggname)&&(*arggname)){}
	   }


	   return p;

}

static AVSValue s_get_values(AVSValue args, void* user_data, IScriptEnvironment* env)
{
	bool f;
	int narg=args.ArraySize();

	if((narg<2)&&(!(f=args[0].IsClip())))
	{
		env->ThrowError("Invalid Format GetValues...!!!");
	}

	//IClipMap* p= (IClipMap*) ((void*) args[0].AsClip());
	IClipMap* p=get_intf(args[0].AsClip(),env);

	AVSValue arg_array=args[1];
	narg=arg_array.ArraySize();
	if((narg)&&((narg&1)==0))
	{

		for(int n=0;n<narg;n+=2)
		{
		 AVSValue v=p->get_value(arg_array[n+1].AsString(""));

		 const char* env_name=arg_array[n].AsString("");
		 if(*env_name)
		 {
			 if(StrStrIA(env_name,"global."))
			 {
                  
				  f=env->SetGlobalVar(env_name+7,v);
			 }
			 else  f=env->SetVar(env_name,v);
		 }
		 
		}

	}
	else env->ThrowError("Invalid Format SetValues...Нету Пары!!!");

	//return narg/2;
	return p;
};


static AVSValue s_set_values(AVSValue args, void* user_data, IScriptEnvironment* env)
{
	bool f;
	int narg=args.ArraySize();

	if((narg<2)&&(!(f=args[0].IsClip())))
	{
		env->ThrowError("Invalid Format SetValues...!!!");
	}

	//IClipMap* p= (IClipMap*) ((void*) args[0].AsClip());
	IClipMap* p=get_intf(args[0].AsClip(),env);

	AVSValue arg_array=args[1];
	narg=arg_array.ArraySize();
	if((narg)&&((narg&1)==0))
	{

		for(int n=0;n<narg;n+=2)
		{
		  p->set_value(arg_array[n].AsString(""),arg_array[n+1]);
		}

	}
	else env->ThrowError("Invalid Format SetValues...Нету Пары!!!");

	//return narg/2;
	return p;
};
static AVSValue s_invoke_op(AVSValue args, void* user_data, IScriptEnvironment* env)
{
	bool f;
	int narg=args.ArraySize();

	if((narg<2)&&(!(f=args[0].IsClip())))
	{
		env->ThrowError("Invalid Format SetValues...!!!");
	}

	//IClipMap* p= (IClipMap*) ((void*) args[0].AsClip());
	
	IClipMap* p=get_intf(args[0].AsClip(),env);
    const char* pcmd=args[1].AsString("");
	AVSValue arg_array=args[2];

	narg=arg_array.ArraySize();

	return p->invoke_op(pcmd,narg,arg_array,env);

};

static AVSValue s_get_value(AVSValue args, void* user_data, IScriptEnvironment* env)
{
	//IClipMap* p= (IClipMap*) ((void*) args[0].AsClip());
    IClipMap* p=get_intf(args[0].AsClip(),env);
	AVSValue ret=p->get_value(args[1].AsString(""),args[2]);
	return ret;
};
inline static void register_interface(IScriptEnvironment* env, void* user_data=0);

inline static double reset_fps(VideoInfo& vi,double fps=25,int denominator = 1001)
{
	fps*=denominator;
	vi.fps_numerator=fps;
    vi.fps_denominator = denominator;
	return fps;
}


inline static double safe_back_abs(double v)
{
	return (v>0)? 1./v:std::numeric_limits<double>::infinity();
}


};

template <class Version=version>
struct IClipMap_impl:public IClipMap
{

	//typedef std::map<std::string,AVSValue > AVS_map_t;
	typedef stdext::hash_map<std::string,AVSValue > AVS_map_t;
	volatile long m_init_flag;
	
    AVS_map_t m_map;
	mutex_cs_t m_mutex_cs;

	 long lock()
	 {
		 m_mutex_cs.lock();
		 return -1;
	 };
	 long unlock()
	 {
		 m_mutex_cs.unlock();
		 return -1;
	 };


	template <class T>
	T* ptr_offset(T* pt,int nb)
	{
		BYTE* p=((BYTE*)pt)+nb;
		return (T*)p;
	}

	IClipMap_impl(){
		m_uuid=get_uiid();
		m_init_flag=0;
		//last_AVSValue_ref()=AVSValue(this);
	}
	
static IClipMap* global_list()
{
  return (IClipMap*)(void*)_global_list()->AsClip();
}
static AVSValue*& _global_list()
{
	struct Immm:IClipMap_impl<Version>{
	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env) { return PVideoFrame();}
	void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) {};
	const VideoInfo& __stdcall GetVideoInfo() { return *((VideoInfo*)0);}
	bool __stdcall GetParity(int n) { return 0;}
	void __stdcall SetCacheHints(int cachehints,int frame_range) { } ;
	~Immm()
	{
       m_map.clear();
	}

  };
	static AVSValue* list=0;
	if(!list) list=new AVSValue(new Immm);
	return list;
     
}
static void __cdecl onexit(void* user_data, IScriptEnvironment* env)
{

	try{
	//(*IClipMap_impl::_global_list())=AVSValue();
	
	}catch(...)
	{

	}
   IClipMap_impl::_global_list()=0;
};

    
	AVSValue get_value(const char* name,AVSValue dflv=AVSValue())
	{
    	locker_t<IClipMap> lock(this);

		AVS_map_t::iterator i= m_map.find(name);
		if(i!=m_map.end()) 
		{
			AVSValue& r=i->second;
			if(r.Defined())	return r;
			else return dflv;
		}
		else return dflv;
	}
	void set_value(const char* name,AVSValue newval)
	{
		locker_t<IClipMap> lock(this);
		m_map[name]=newval;
	}
	AVSValue invoke_op(const char* name,int argc,AVSValue args,IScriptEnvironment* env)
	{
		return AVSValue();
	};

inline static	 AVSValue AVSArray_def(AVSValue a, int n,AVSValue dflt=AVSValue())
	{
		int nn=a.ArraySize();
		if(in_interval(0,n,nn))
			return a[n];
		else return dflt;
	}


};


template <class Version=version>
class GenericVideoFilterMap : public IClipMap_impl<Version>{
protected:
	PClip child;
	VideoInfo vi;
public:
	GenericVideoFilterMap(PClip _child) 
		: child(_child) { vi = child->GetVideoInfo(); }
	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env) { return child->GetFrame(n, env); }
	void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) { child->GetAudio(buf, start, count, env); }
	const VideoInfo& __stdcall GetVideoInfo() { return vi; }
	bool __stdcall GetParity(int n) { return child->GetParity(n); }
	void __stdcall SetCacheHints(int cachehints,int frame_range) { } ;  // We do not pass cache requests upwards, only to the next filter.
	 //AVSValue get_value(const char* name,AVSValue dflv=AVSValue()){return AVSValue();};
     //void set_value(const char* name,AVSValue dflv=AVSValue()){};

};


template <class Version=version>
struct avs_base:public GenericVideoFilterMap<Version>
{


	
	avs_base(PClip _clip):GenericVideoFilterMap(_clip){};
/*
	AVS_map_t m_map;
	typedef std::map<std::string,AVSValue> AVS_map_t;
   AVSValue get_value(const char* name,AVSValue dflv=AVSValue())
  {
	  AVS_map_t::iterator i= m_map.find(name);
	  if(i!=m_map.end()) 
		  return i->second;
	  else return dflv;
  }
   void set_value(const char* name,AVSValue newval)
  {
	  m_map[name]=newval;
  }
*/
	class CStopwatch
	{
	public:
		CStopwatch() { QueryPerformanceFrequency(&m_liPerfFreq), Start(); }

		inline	 void Start() { QueryPerformanceCounter(&m_liPerfStart); }

		inline long double Sec()
		{
			LARGE_INTEGER liPerfNow;
			QueryPerformanceCounter(&liPerfNow);
			return (long double)( liPerfNow.QuadPart - m_liPerfStart.QuadPart ) / (long double)(m_liPerfFreq.QuadPart);
		}
		inline LONGLONG Tic()
		{
			LARGE_INTEGER liPerfNow;
			QueryPerformanceCounter(&liPerfNow);
			return (LONGLONG)( liPerfNow.QuadPart - m_liPerfStart.QuadPart );
		}
		inline LONGLONG Frec()
		{
			QueryPerformanceFrequency(&m_liPerfFreq);
			return  m_liPerfStart.QuadPart;
		}
	private:
		LARGE_INTEGER m_liPerfFreq;
		LARGE_INTEGER m_liPerfStart;
	};

inline static long& log_flag()
{    
	   static long s=0;
       return s;
}

static AVSValue s_console_show(AVSValue args, void* user_data, IScriptEnvironment* env)
{
          
	AVSValue v1=args[0];
	if(v1.IsFloat())
	{
		float fl=v1.AsFloat(0);
       HWND hc=GetConsoleWindow();
	   if(hc) ShowWindow(hc,fl);	
	}
     
	return args[0];
}

static AVSValue s_set_log(AVSValue args, void* user_data, IScriptEnvironment* env)
{
    AVSValue v1=args[0];
	if(v1.IsFloat())
	{
		float fl=v1.AsFloat(0);
		long l=(fl!=0)?1:0;
        InterlockedExchange(&log_flag(),l);
	}

	return args[0];
}

static AVSValue s_get_OS_environ(AVSValue args, void* user_data, IScriptEnvironment* env)
{
	AVSValue v1=args[0];

	char* pstr="";
    

	if(v1.IsString())
	{
		char buf[1024];

		 
		 
		 int n=GetEnvironmentVariableA(v1.AsString(""),buf,1024);
		 if(n)
		 {
           pstr=env->SaveString(buf,-1);
		 }
		 else
		 {
 		   if((args.ArraySize()>1)&&(args[1].IsString()))
    		 {
			  return args[1];
	    	 }
		 }
		  
		 
		
	}

	return AVSValue(pstr);
}


inline static void	log_printf(const char* fmt,...){
	   char buf[256];
	   if(InterlockedExchangeAdd(&log_flag(),0))
	   {
		   va_list argptr;
		   va_start(argptr, fmt);
		   _vcprintf(fmt,argptr);
	   }
	};

};

template <class OwnClip>
struct  CurrentGetter: public IClipMap {

	OwnClip* p;
	CurrentGetter(OwnClip* _p):p(_p){m_uuid=get_uiid();};
	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env) 
	{
		return p->current_frame();
	}
	bool __stdcall GetParity(int n) { return false; }
	const VideoInfo& __stdcall GetVideoInfo() { return p->GetVideoInfo();}
	void __stdcall SetCacheHints(int cachehints,int frame_range) { };
	void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env) { };
	AVSValue get_value(const char* name,AVSValue dflv=AVSValue())
	{
		return p->get_value(name,dflv);
	};
	void set_value(const char* name,AVSValue newV=AVSValue())
	{
		p->set_value(name,newV);
	};
	AVSValue invoke_op(const char* name,int argc,AVSValue args, IScriptEnvironment* env)
	{
		return p->invoke_op(name,argc,args,env);
	};
	long lock()	{ return p->lock();};
	long unlock(){return p->unlock();};

};

void IClipMap::register_interface(IScriptEnvironment* env, void* user_data){


	{
     v_buf<wchar_t> buf(4096);
     bufT<wchar_t> tmp=argv_ini<wchar_t>().get_module_name();
	 wchar_t* pp;
	 argv_env<wchar_t> env;
	 if(!env[L"nps_detect_path"].is())
	 if(GetFullPathNameW(tmp,buf.count(),buf,&pp))
	 {
		 *pp=0;
		 pp=buf;
		 SetEnvironmentVariableW(L"nps_detect_path",buf);
		 buf.cat(L";");
		 buf.cat(env[L"PATH"]);
		 pp=buf;
		 SetEnvironmentVariableW(L"PATH",buf);
	 }

	}

    //env->AtExit(&IClipMap_impl<>::onexit,0);  
	env->AddFunction("getMapValue", "c[name].[default].",&IClipMap::s_get_value,0);
	env->AddFunction("setMapValue", ".[name].[value].",&IClipMap::s_set_value,0);
	env->AddFunction("setMapValues", "c.*",&IClipMap::s_set_values,0);
	env->AddFunction("getMapValues", "c.*",&IClipMap::s_get_values,0);
    env->AddFunction("MapContext", "cs",&IClipMap::s_map_context,0);
	env->AddFunction("InvokeOp", "cs.*",&IClipMap::s_invoke_op,0);
	env->AddFunction("LastMap", "",&IClipMap::s_last_AVSValue,0);
	env->AddFunction("NisranAtan","cf[xyina]f",&IClipMap::s_atan2,0);
	env->AddFunction("LogFlag","[flag]f",&avs_base<>:: s_set_log,0);
	env->AddFunction("OSEnviron","[name]s[default]s",&avs_base<>::s_get_OS_environ,0);
	

	
}

inline double get_AVSFloat(const AVSValue& a,double df=0)
{
       if(a.IsFloat())
	   		   return a.AsFloat();
	   if(a.IsString())
	   {
		   return get_double(a.AsString());
	   }
	   return df;
}