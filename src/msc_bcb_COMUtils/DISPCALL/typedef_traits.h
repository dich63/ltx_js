#pragma once





template<class T>
struct is_typedef_base {
	typedef char (&yes)[1];
	typedef char (&no)[2];
	static T* get();
};

template<class V,class T>
struct is_convertible:is_typedef_base<T>
{
	static yes use(T);
	static no use(...);
  enum { result=sizeof( use(get()) )==sizeof(yes) };
};

#define DECL_IS_TYPEDEFER(name,type_name) \
	template<class T>\
struct name:is_typedef_base<T>{\
	template<class V> static no use(void*);\
	template<class V> static yes use(V*, typename V::##type_name* = 0);\
	enum { result=sizeof( use<T>(get()) )==sizeof(yes) };\
};

#define DECL_IS_TYPEDEFER_PRFX(name_base,type_name) DECL_IS_TYPEDEFER(name_base##_##type_name,type_name)


#define DECL_SELECT_TYPEDEFER(name,type_name)\
	template <bool flag, typename T, typename U>\
struct name\
{	typedef typename T::##type_name Result;};\
	template <typename T, typename U>\
struct name<false, T, U>\
{	typedef typename U Result;};





namespace name_s
{
	DECL_IS_TYPEDEFER(AS,VHolder)
		DECL_SELECT_TYPEDEFER(TAS,VHolder)
		template <typename CC,typename AA>
	struct typedef_select
	{
		typedef typename TAS<AS<CC>::result,CC,AA>::Result result;
	};
}

#define DECL_TYPE_SELECTOR_(name_s,type_name,addn)\
	namespace name_s\
{	DECL_IS_TYPEDEFER(__AS##addn,type_name)\
	DECL_SELECT_TYPEDEFER(__TAS##addn,type_name)\
	template <typename CC,typename AA>\
struct typedef_select##addn\
{		typedef typename __TAS##addn<__AS##addn<CC>::result,CC,AA>::Result result;};\
};

#define DECL_TYPE_SELECTOR_IN(name_s,type_name) DECL_TYPE_SELECTOR_(name_s,type_name,type_name) 
#define DECL_TYPE_SELECTOR(name_s,type_name) DECL_TYPE_SELECTOR_(name_s,type_name,)
#define DECL_TYPE_SELECTOR_PRFX(name_base,type_name) DECL_TYPE_SELECTOR(name_base##_##type_name,type_name)

#define  TYPE_SELECT_IF(name_ss,__tn) name_ss##_##__tn::typedef_select  
//typedef typename    name_ss_VHolder::typedef_select<CC,float>::result VHolder;



template <bool flag, typename T, typename U>
struct typedef_if
{
	typedef typename T Result;
};
template <typename T, typename U>
struct typedef_if<false, T, U>
{
	typedef typename U Result;
};

template <class U> struct PtrSelect
{
	enum { result = false };
	typedef void* PtrType;
    typedef     U ValueType;
	inline	PtrType operator =(U const& )
	{		return PtrType(NULL);	}
};

template <class U> struct PtrSelect<U*>
{
	enum { result = true };
	typedef U* PtrType;
	typedef  U ValueType;
	inline PtrType operator =(U * t)
	{		return t;	}
};

