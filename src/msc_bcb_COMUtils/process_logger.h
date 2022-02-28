#ifndef PROCESS_LOGGER_FMM_H
#define PROCESS_LOGGER_FMM_H


#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
#include <strstream>
#include <iostream>
#include <string>

#include "cpp_name_pair_generator.h"




template<bool open_only=true,int map_siz=0x10000>
struct process_logger
{

 enum
 {
	 server=!open_only
 };
typedef process_logger<open_only,map_siz> logger_type;

	HANDLE m_hmap,m_hmutex;
	char* m_p;


	
	struct locker
	{
		logger_type* m_plog;
		locker(logger_type* p):m_plog(p)
		{
			m_plog->lock();
		}
		~locker()
		{
			m_plog->unlock();
		}
	};

    char __tmp[2];
	 
	char* _ptr()
	{
		return (m_p)?m_p:__tmp;
	}

	int size()
	{
		return (m_p)?map_siz:0;
	}


	struct lockerstream:std::strstream
	{
		  locker lock;
           
		
		lockerstream(logger_type* pl):
			   std::strstream(*pl,pl->size()),lock(pl)
		   {
			   if(!(*pl)) setstate(std::ios::failbit);
		   }
		

		   template<typename N>
		    lockerstream&  operator<<(N const & Val)
			   {
                  *static_cast<std::strstream*>(this)<<Val;
				  return *this;
  			   }

			
			
        ~lockerstream()
		{
            *static_cast<std::strstream*>(this)<<'\0'<<'\0';
		}


	};

	
lockerstream operator ()()
	{
			return lockerstream(this);
 	}



	inline  void lock()
	{
		::WaitForSingleObject(m_hmutex,INFINITE);
	}
	inline  void unlock()
	{
		::ReleaseMutex(m_hmutex);
	}
	inline bool operator !()
	{
		return (m_p==0);
	}
	inline char*  name_prefix(char* prfx,int pid,char* buf)
	{

		sprintf(buf,"%s_F9C00F052F0A41fbAFFF1BB5553DB53C_%x",prfx,pid);
		return buf;
	}

	bool init(int pid=::GetCurrentProcessId())
	{
        m_p=0;
		char buf[128];
		m_hmutex=::CreateMutexA(0,false,name_prefix("mtx",pid,buf));
		if(open_only)
		{
			m_hmap=::OpenFileMappingA(FILE_MAP_ALL_ACCESS,false,name_prefix("fmm",pid,buf));
		}
		else
		{
			m_hmap=::CreateFileMappingA(INVALID_HANDLE_VALUE,0,SEC_COMMIT|PAGE_READWRITE,
				0,map_siz,name_prefix("fmm",pid,buf));
		}
		if(m_hmap)
			m_p=(char*)::MapViewOfFile(m_hmap,FILE_MAP_ALL_ACCESS,0,0,map_siz);
		return m_p;
	}


	process_logger(int pid=::GetCurrentProcessId()):m_hmutex(0),m_hmap(0),m_p(0)
	{
		if(open_only)
			  init(pid);
	}


	~process_logger()
	{
		clear();
	}

	void clear()
	{
		if(m_p) {::UnmapViewOfFile(m_p);	m_p=0;}
		if(m_hmap) {::CloseHandle(m_hmap);m_hmap=0;}
		if(m_hmutex) {::CloseHandle(m_hmutex);m_hmutex=0;}
	}

	inline int printf(char* fmt, ...)
	{
		if(!m_p)		  return 0;
		locker lock(this);

		va_list argptr;
		va_start(argptr, fmt);
		return vsprintf(m_p,fmt,argptr); 
	}


	template <class T>
	T get_str()
	{
		if(!m_p) 
			return T();
		locker lock(this);
		return T(m_p);
	}

	template <class T>
	T get_str_def(char * pdef="")
	{
		if(!m_p) 
			return T();
	if(*m_p)
	{
		locker lock(this);
		return T(m_p);
	}
	  return pdef;
	}


	operator char* ()
	{
		return m_p;
	}


};






#define to_json CPP_NAME_PAIR
#define to_json_o(a,...) make_json_o_p(get_cpp_name_pair_generator(DEL_IF_ONE_PARAM(a,__VA_ARGS__)).setname(#a)(__VA_ARGS__))
#define to_json_o_struct(prfx,a,...) make_json_o_p(get_cpp_name_pair_generator(DEL_IF_ONE_PARAM((prfx.##a),__VA_ARGS__)).setname(#a)(__VA_ARGS__))


template <class Name,class Value>
struct json_o
{
	std::pair<Name,Value> p;
	    json_o(){};
		json_o(Name& name,Value& value)
		{
			p.first=name;
			p.second=value;
		}
		json_o(std::pair<Name,Value> pp)
		{
			p=pp;
		}
};

template <class Name,class Value>
json_o<Name,Value> make_json_o_p(std::pair<Name,Value> pp)
{
	return json_o<Name,Value>(pp);
}
template <class Name,class Value>
json_o<Name,Value> make_json_o(Name n,Value v)
{
	return json_o<Name,Value>(n,v);
}



struct s_json_end{};
struct json_CF{};
struct json_comment
{
	const char* ps;
	json_comment(const char* s):ps(s){};
};

#define json_end s_json_end()







template <class T,class Stream>
struct jsonstreamerT
{
	typedef  typename Stream stream_type;
    bool fnonempty;
	std::string sdelim;

	jsonstreamerT():fnonempty(false){};

inline T& set_delimiter(char* p=0)
{
	if(!p) sdelim="";
	else sdelim=p;
    T* pt=static_cast<T*>(this);
	return *pt;
};

  inline T& end()
  {
	  T* pt=static_cast<T*>(this);
	  if(fnonempty)
	  {
      Stream& s=pt->stream();
	  s<<sdelim;
	  s<<"}"<<'\0'<<'\0';
	  s.seekp(-2, std::ios_base::end);
	  fnonempty=false;
	  };
	  return *pt;
  };
   template<typename C,typename N>
   T& add(std::pair<C,N>&  p,char* qq="")
   {
       T* pt=static_cast<T*>(this);
	   Stream& s=pt->stream();
	   if(fnonempty)  s<<",";
	   else
	   {
		   s<<"{";
		   fnonempty=true;
	   }
       s<<sdelim;  
	   s<<"\""<< p.first<<"\":"<<qq<<p.second<<qq;
	   return *pt;
   };


	template<typename C,typename N>
	T&  operator<<(json_o<C,N>  o)
	{
		return add(o.p);
	}

	template<typename C,typename N>
	T&  operator<<(std::pair<C,N>  p)
	{
		return add(p,"\"");
	}
	
	T&  operator<<(s_json_end p)
	{
		return end();
	}
	T&  operator<<(json_CF p)
	{
		T* pt=static_cast<T*>(this);
		pt->stream()<<"\n";
		return *pt;
	}
	T&  operator<<(json_comment p)
	{
		T* pt=static_cast<T*>(this);
		pt->stream()<<"/*"<<p.ps<<"*/";
		return *pt;
	}
};



	
	




template <class Stream>
struct jsonstreamer:jsonstreamerT<jsonstreamer<Stream>,Stream>
{
	Stream& r_stream;
	inline Stream& stream()
	{
		return r_stream;
	}
	jsonstreamer(Stream& s,char * delimiter=0):r_stream(s)
	{
		set_delimiter(delimiter);
	};  
	~jsonstreamer()
	{
		end();
	}
};




template<bool open_only=true,int map_siz=0x10000>
struct process_json_logger:process_logger<open_only,map_siz>
{

	struct jsonlockerstream:jsonstreamerT<jsonlockerstream,lockerstream>
	{
		lockerstream m_stream;
		inline lockerstream& stream()
		{
			return m_stream;
		}
		jsonlockerstream(logger_type* pl,char * delimiter=0):m_stream(pl)
		{
           set_delimiter(delimiter);
		};  
		~jsonlockerstream()
		{
			end();
		}
	};


 process_json_logger(int pid=::GetCurrentProcessId()):process_logger<open_only,map_siz>(pid)
 {}


 
 jsonlockerstream operator ()(char* pdelimiter=0)
 {
	 return jsonlockerstream(this,pdelimiter);
 }

};

typedef  process_logger<true>  process_log;
typedef  process_json_logger<true>  process_json_log;


template<class ProcessLoger>
struct process_json_logger_test
{

	ProcessLoger jpl;


	process_json_logger_test()
	{
		char buf[4000];
		POINT px={10,121};

		px.y=GetTickCount();

		
		if(jpl.server) 	  jpl.init();

		std::strstream ss(buf,4000);
		{
			//jsonstreamer<std::strstream> jst(ss,"\n");
			jsonstreamer<std::strstream> jst(ss);
			jst<<to_json_o_struct(px,x)<<to_json_o_struct(px,y);
		}
		  
		std::cout<<"json_POINT="<<buf<<std::endl;

		double pi=3.1415;
		jpl()<<to_json(pi)<<to_json(zzz,999)<<to_json_o(buf)<<to_json_o(point,buf);

		std::cout<<"json_log_string="<<jpl.get_str<std::string>() ; 
		

	}


};



#endif

/*
// TEST !!!!
void main()
{
std::cout<<"test as child process"<<std::endl;
process_json_logger_test< process_json_log >();
std::cout<<"test as parent process"<<std::endl;
process_json_logger_test< process_json_logger<0> >();
getchar();
}
*/