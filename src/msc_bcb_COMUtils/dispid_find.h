#pragma once
typedef struct _PAIR_NAME
{
	const wchar_t* name;
	const DISPID id;
	const size_t cb;
	const int len;
} PAIR_NAME;

#define _MLL_ int(256) 
#define BEGIN_STRUCT_NAMES(pn)  PAIR_NAME pn[]={
#define PAIR_SHIFT(b,s) {L#b,offsetof(s,b),sizeof(((s *)0)->b),_MLL_},
#define PAIR_SHIFT_2(b,s,off) {L#b,offsetof(s,b)+off,sizeof(((s *)0)->b),_MLL_},
#define DISP_PAIR(b,id)  {L#b,id,-1,_MLL_},
#define DISP_PAIR_IMPLC_LEN(b,id,l)  {L#b,id,-1,l},
#define DISP_PAIR_IMPLC(b,id)  {L#b,id,-1,wcslen(L#b)},
#define END_STRUCT_NAMES {NULL,-1,-1,_MLL_} };
#define DISP_ONE(b) DISP_PAIR(b,b)

 inline const wchar_t* find_name_def(PAIR_NAME* pn,DISPID id,const wchar_t* pdef=0)
 {
     while(pn->name)
	 {
           if(pn->id==id) return pn->name; 
		 ++pn;
	 }

     return pdef;    
 }
 inline HRESULT find_id(PAIR_NAME* pn,const wchar_t* name,DISPID* pid,DISPID log_add=0)
 {

	 if(!pid) return E_POINTER;
	 if(!name) return DISP_E_UNKNOWNNAME;
	 int ln0=(name)?wcslen(name):0;

	 for(int i=0;pn[i].name;i++)
	 {    	 
		 int  l=pn[i].len;
		 if(StrCmpNIW(pn[i].name,name,l)==0) return (*pid=(pn[i].id)|log_add),S_OK;     	 
	 }

	 return DISP_E_UNKNOWNNAME;
 }

 template<int VERS=0>
 struct VARTYPE_finder_t
 {


	 static const wchar_t* jsname(VARTYPE id,const wchar_t* pdef=0)
	 {
		 HRESULT hr;
		 static  BEGIN_STRUCT_NAMES(pn)
			 DISP_PAIR(Float32Array,VT_R4)
			 DISP_PAIR(Float64Array,VT_R8)
			 DISP_PAIR(Int8Array,VT_I1)
			 DISP_PAIR(Int16Array,VT_I2)
			 DISP_PAIR(Int32Array,VT_I4)
			 DISP_PAIR(Int64Array,VT_I8)
			 DISP_PAIR(Uint8Array,VT_UI1)
			 DISP_PAIR(Uint16Array,VT_UI2)			 
			 DISP_PAIR(Uint32Array,VT_UI4)
			 DISP_PAIR(Uint64Array,VT_UI8)
			 DISP_PAIR(Int16Array,VT_BOOL)
			 END_STRUCT_NAMES;
		 return find_name_def(pn,id,pdef);
	 };
	static const wchar_t* name(VARTYPE id,const wchar_t* pdef=0)
	 {
		 HRESULT hr;
		 static  BEGIN_STRUCT_NAMES(pn)
			 DISP_PAIR(int16(VT_I2),VT_I2)
			 DISP_PAIR(int32(VT_I4),VT_I4)
			 DISP_PAIR(float32(VT_R4),VT_R4)
			 DISP_PAIR(float64(VT_R8),VT_R8)
			 DISP_PAIR(float(VT_R4),VT_R4)
			 DISP_PAIR(double(VT_R8),VT_R8)
			 DISP_PAIR(char(VT_I1),VT_I1)
			 DISP_PAIR(byte(VT_UI1),VT_UI1)
			 DISP_PAIR(uint16(VT_UI2),VT_UI2)
			 DISP_PAIR(uint32(VT_UI4),VT_UI4)
			 DISP_PAIR(int64(VT_I8),VT_I8)
			 DISP_PAIR(uint64(VT_UI8),VT_UI8)
			 DISP_PAIR(variant(VT_VARIANT),VT_VARIANT)
			 DISP_PAIR(error(VT_ERROR),VT_ERROR)
			 DISP_PAIR(date(VT_DATE),VT_DATE)
			 DISP_PAIR(string(VT_BSTR),VT_BSTR)
			 DISP_PAIR(bool(VT_BOOL),VT_BOOL)
			 END_STRUCT_NAMES;
		 return find_name_def(pn,id,pdef);
	 };
	static HRESULT type_by_name(const wchar_t* pstr,VARTYPE *pvt )
	{
		HRESULT hr;
		static  BEGIN_STRUCT_NAMES(pn)
			DISP_ONE(2)
			DISP_ONE(3)
			DISP_ONE(4)
			DISP_ONE(5)
			DISP_ONE(11)
			DISP_ONE(16)
			DISP_ONE(17)
			DISP_ONE(18)
			DISP_ONE(19)
			DISP_ONE(20)
			DISP_ONE(21)
			DISP_ONE(VT_I2)
			DISP_ONE(VT_I4)
			DISP_ONE(VT_R4)
			DISP_ONE(VT_R8)
			DISP_ONE(VT_I1)
			DISP_ONE(VT_UI1)
			DISP_ONE(VT_UI2)
			DISP_ONE(VT_UI4)
			DISP_ONE(VT_I8)
			DISP_ONE(VT_UI8)
			DISP_ONE(VT_VARIANT)
			DISP_ONE(VT_ERROR)
			DISP_ONE(VT_DATE)
			DISP_ONE(VT_BSTR)
			DISP_ONE(VT_BOOL)
			DISP_PAIR(int16,VT_I2)
			DISP_PAIR(int32,VT_I4)
			DISP_PAIR(float32,VT_R4)
			DISP_PAIR(float64,VT_R8)
			DISP_PAIR(float,VT_R4)
			DISP_PAIR(single, VT_R4)
			DISP_PAIR(double,VT_R8)
			DISP_PAIR(char,VT_I1)
			DISP_PAIR(byte,VT_UI1)
			DISP_PAIR(int8,VT_I1)
			DISP_PAIR(uint8,VT_UI1)
			DISP_PAIR(uchar,VT_UI1)
			DISP_PAIR(uint16,VT_UI2)
			DISP_PAIR(uint32,VT_UI4)
			DISP_PAIR(int64,VT_I8)
			DISP_PAIR(uint64,VT_UI8)
			DISP_PAIR(variant,VT_VARIANT)
			DISP_PAIR(error,VT_ERROR)
			DISP_PAIR(date,VT_DATE)
			DISP_PAIR_IMPLC(str,VT_BSTR)
			DISP_PAIR_IMPLC(bool,VT_BOOL)
			DISP_PAIR_IMPLC(logic,VT_BOOL)
			DISP_PAIR(float32array,VT_R4)
			DISP_PAIR(float64array,VT_R8)
			DISP_PAIR(int8array,VT_I1)
			DISP_PAIR(int16array,VT_I2)
			DISP_PAIR(int32array,VT_I4)
			DISP_PAIR(uint8array,VT_UI1)
			DISP_PAIR(uint16array,VT_UI2)
			DISP_PAIR(uint32array,VT_UI4)
			END_STRUCT_NAMES
			DISPID di;
		if(SUCCEEDED( hr=find_id(pn,pstr,&di)))
			*pvt=di;
		return hr;
	};


	static long element_size(VARTYPE vt)
	{
		SAFEARRAY* psa;
		SAFEARRAYBOUND sb={1,0};
		if(!(psa=SafeArrayCreate(vt,1,&sb) ) ) return 0;
		long e=psa->cbElements;
		SafeArrayDestroy(psa);
		return e;
	}

	static  HRESULT element_size(VARTYPE vt,long* psize)
	{
		HRESULT hr;
		long e=element_size(vt);
		if(!e) return hr=E_INVALIDARG;
		*psize=e;
		return S_OK;
	}

	static  HRESULT element_size(wchar_t* vtype,long* psize,VARTYPE* pvt=0)
	{
		HRESULT hr;
		VARTYPE vt;
		long t;
		if(!psize) psize=&t;
		OLE_CHECK_hr(type_by_name(vtype,&vt));
		OLE_CHECK_hr(element_size(vt, psize));
		if(pvt) *pvt=vt;
		return hr;
		/*
		long e=element_size(vt);
		if(!e) return hr=E_INVALIDARG;
		*psize=e;
		return S_OK;
		*/
	}


	template <typename T> static VARTYPE VarType(T*);

	template<> static VARTYPE VarType(LONG*) { return VT_I4; }
	template<> static VARTYPE VarType(BYTE*) { return VT_UI1; }
	template<> static VARTYPE VarType(SHORT*) { return VT_I2; }
	template<> static VARTYPE VarType(FLOAT*) { return VT_R4; }
	template<> static VARTYPE VarType(DOUBLE*) { return VT_R8; }
	//template<> VARTYPE VarType(VARIANT_BOOL*) { return VT_BOOL; }
	
	template<> static VARTYPE VarType(CY*) { return VT_CY; }
	
	template<> static VARTYPE VarType(BSTR*) { return VT_BSTR; }
	template<> static VARTYPE VarType(IUnknown **) { return VT_UNKNOWN; }
	template<> static VARTYPE VarType(IDispatch **) { return VT_DISPATCH; }
	template<> static VARTYPE VarType(SAFEARRAY **) { return VT_ARRAY; }
	template<> static VARTYPE VarType(CHAR*) { return VT_I1; }
	template<> static VARTYPE VarType(USHORT*) { return VT_UI2; }
	template<> static VARTYPE VarType(ULONG*) { return VT_UI4; }
	template<> static VARTYPE VarType(INT*) { return VT_I4; }
	template<> static VARTYPE VarType(UINT*) { return VT_UI4; }
	template<> static VARTYPE VarType(INT64*) { return VT_I8; }
	template<> static VARTYPE VarType(UINT64*) { return VT_UI8; }
	template<> static VARTYPE VarType(VARIANT*) { return VT_VARIANT; }



 };