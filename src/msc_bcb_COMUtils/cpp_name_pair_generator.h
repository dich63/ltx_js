#ifndef CPP_NAME_PAIR_GENERATOR_H
#define CPP_NAME_PAIR_GENERATOR_H

#include <utility>

template <class T>
struct cpp_name_pair_generator
{
	char* pname;
	T  t;
	cpp_name_pair_generator(T& u)
	{
		t=u;
	};
	cpp_name_pair_generator()
	{
		t=0;
	};

	cpp_name_pair_generator<T>& setname(char* p){
		pname=p;
		return *this;
	}
	std::pair<char*,T> operator()()
	{
       std::pair<char*,T> p=std::make_pair(pname,t);  
		return p;
	}
	template<class N>
	std::pair<char*,N> operator()(N n)
	{
		return std::make_pair(pname,n);
	}

};

template <class T>
cpp_name_pair_generator<T> get_cpp_name_pair_generator(T t)
{
	return cpp_name_pair_generator<T>(t);
}



cpp_name_pair_generator<void*> get_cpp_name_pair_generator()
{
	return cpp_name_pair_generator<void*>();
}

#define R_R_R(name) name
#define DEL_IF_ONE_PARAM(a,...) R_R_R(/R_R_R(**__VA_ARGS__)/R_R_R(a)/R_R_R(__VA_ARGS__**)/)
#define CPP_NAME_PAIR(a,...) get_cpp_name_pair_generator(DEL_IF_ONE_PARAM(a,__VA_ARGS__)).setname(#a)(__VA_ARGS__)


#endif
