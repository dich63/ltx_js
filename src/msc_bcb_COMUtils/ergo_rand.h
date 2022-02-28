#pragma once

#include <intrin.h>

#pragma intrinsic(_BitScanForward)

template <typename  RT=unsigned int>
struct small_rand_base
{
  typedef  typename RT rand_type;
  typedef rand_type (__stdcall *ergo_step_proc_type)(rand_type ir,rand_type n);
  ergo_step_proc_type ergo_step;
  int modulo;
  int ma;
  int mc;

};

typedef small_rand_base<unsigned int> small_rand_baseUI;

template <int dig>
struct small_rand:small_rand_baseUI
{
  //typedef unsigned int rand_type;
   enum
   {       mask=(1<<dig)-1,
	       dig_h2=dig-2,
		   dig_h4=dig-4,
		   dig_h8=dig-8,
		   dig_a00=(dig_h8>0)?dig_h8:dig_h4,
		   dig_a=(dig_a00>0)?dig_a00:dig_h2,
		   dig_c=(dig_h4>0)?dig_h4:dig_h2,
		   a2i=(1<<(dig_a)),
		   af=1,
		   a=a2i+af,
		   c=(1<<dig_c)+1
   };


static inline rand_type& step(rand_type& ir) 
{
   //
	
	ir=( a2i*ir+af*ir +c)&mask;
	//ir=( (ir<<dig_a)+af*ir +c)&mask;
   return ir;
};

static  rand_type  __stdcall _ergo_step(rand_type ir,rand_type n) 
{
	while(step(ir)>=n);
	return ir;
}

small_rand()
{
	ergo_step=&_ergo_step;
	modulo=1<<dig;
	ma=a;
	mc=c;
};


};

template <typename  RT=unsigned int>
struct small_rand_list
{
  small_rand_baseUI m_rb[32]; //* = ::new(calloc(32,sizeof(small_rand_baseUI))) small_rand_baseUI;
  small_rand_list()
  {
          
	  m_rb[0]= small_rand<0>();
	  m_rb[1]= small_rand<1>();
	  m_rb[2]= small_rand<2>();
	  m_rb[3]= small_rand<3>();
	  m_rb[4]= small_rand<4>();
	  m_rb[5]= small_rand<5>();
	  m_rb[6]= small_rand<6>();
	  m_rb[7]= small_rand<7>();
	  m_rb[8]= small_rand<8>();
	  m_rb[9]= small_rand<9>();

	  m_rb[10]= small_rand<10>();
	  m_rb[11]= small_rand<11>();
	  m_rb[12]= small_rand<12>();
	  m_rb[13]= small_rand<13>();
	  m_rb[14]= small_rand<14>();
	  m_rb[15]= small_rand<15>();
	  m_rb[16]= small_rand<16>();
	  m_rb[17]= small_rand<17>();
	  m_rb[18]= small_rand<18>();
	  m_rb[19]= small_rand<19>();

	  m_rb[20]= small_rand<20>();
	  m_rb[21]= small_rand<21>();
	  m_rb[22]= small_rand<22>();
	  m_rb[23]= small_rand<23>();
	  m_rb[24]= small_rand<24>();
	  m_rb[25]= small_rand<25>();
	  m_rb[26]= small_rand<26>();
	  m_rb[27]= small_rand<27>();
	  m_rb[28]= small_rand<28>();
	  m_rb[29]= small_rand<29>();

	  m_rb[30]= small_rand<30>();
	  m_rb[31]= small_rand<31>();

  }

  //isNonzero = _BitScanReverse(&index, mask);
inline small_rand_baseUI& operator[](int n)
{
    unsigned long index;
   BitScanReverse(&index,n);
   return m_rb[index+1];
}

};


inline small_rand_baseUI* get_ergo_rand(small_rand_baseUI::rand_type NMax)
{
	//static small_rand_baseUI* = ::new(calloc(32,sizeof(small_rand_baseUI))) small_rand_baseUI;
    static  small_rand_list<small_rand_baseUI::rand_type> ssrb;
	return &ssrb[NMax];
}


unsigned long xor128()
{
	static unsigned long x=123456789,
		y=362436069,
		z=521288629,
		w=88675123;
	unsigned long t;
	t=(x^(x<<11));x=y;y=z;z=w;
	return(w=(w^(w>>19))^(t^(t>>8)));
}