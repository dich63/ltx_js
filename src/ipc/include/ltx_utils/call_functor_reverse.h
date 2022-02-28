#pragma once
// call_functor_reverse.h

#include "call_functor_macro.h"
#include <vector>
#include <algorithm>


template <class T,class V,class R=V,class VH=V>
struct invoker_base_t
{
	typedef typename V arg_t;
	typedef typename VH argh_t;
	typedef typename R res_t;


	inline 	res_t invoke(std::vector<arg_t>& args){

		int c=args.size();
		arg_t* argv=(c)?&args[0]:0;
		return invoke(c,argv); 				
	}


	inline 	res_t invoke(int argc=0,arg_t* argv=0){


		if(argc>0)
		{
			std::vector<arg_t> rv(argc);
			std::reverse_copy(argv,argv+argc,rv.begin());
			return __invoke_proc(argc,&rv[0]);
		}
		else return __invoke_proc();
		//	return R();
	} 

	inline 	res_t invoke_proc(int argc,argh_t* argv){ return R();}

	inline	res_t __invoke_proc(int argc=0,argh_t* argv=0)
	{
		return  static_cast<T*>(this)->invoke_proc(argc,argv);
	}

#define INLINE_OPERATOR_00001A inline R operator()
	MACRO_CALL_s_n(16,INLINE_OPERATOR_00001A ,__invoke_proc,V,VH);
#undef  INLINE_OPERATOR_00001A


	/*
	inline    R operator()(){
	VH vv[1];
	return  __invoke_proc(0,vv); 
	}

	inline    R operator()(V v0){
	VH vv[1+1]={v0};
	return  __invoke_proc(1,vv); 
	}

	inline	R operator()(V v0,V v1){
	VH vv[2+1]={v1,v0};
	return  __invoke_proc(2,vv); 
	}
	inline	R operator()(V v0,V v1,V v2){
	VH vv[3+1]={v2,v1,v0};
	return  __invoke_proc(3,vv); 
	}

	inline	R operator()(V v0,V v1,V v2,V v3){
	VH vv[4+1]={v3,v2,v1,v0};
	return  __invoke_proc(4,vv); 
	}

	inline	R operator()(V v0,V v1,V v2,V v3,V v4){
	VH vv[5+1]={v4,v3,v2,v1,v0};
	return  __invoke_proc(5,vv); 
	}

	inline	R operator()(V v0,V v1,V v2,V v3,V v4,V v5){
	VH vv[6+1]={v5,v4,v3,v2,v1,v0};
	return  __invoke_proc(6,vv); 
	}

	inline	R operator()(V v0,V v1,V v2,V v3,V v4,V v5,V v6){
	VH vv[7+1]={v6,v5,v4,v3,v2,v1,v0};
	return  __invoke_proc(7,vv); 
	}

	inline	R operator()(V v0,V v1,V v2,V v3,V v4,V v5,V v6,V v7){
	VH vv[8+1]={v7,v6,v5,v4,v3,v2,v1,v0};
	return  __invoke_proc(8,vv); 
	}

	inline	R operator()(V v0,V v1,V v2,V v3,V v4,V v5,V v6,V v7,V v8){
	VH vv[9+1]={v8,v7,v6,v5,v4,v3,v2,v1,v0};
	return  __invoke_proc(9,vv); 
	}

	*/
};


