#pragma once

#include "AviSynth.h"


template <class Render>
struct AviSynth_data_wrapper
{
	 long ref_count;
	 AviSynth_data_wrapper():ref_count(1){};
	 inline long AddRef() {	 return InterlockedIncrement((volatile long*)ref_count); };
	 inline long Release() {
		 long l= InterlockedDecrement((volatile long*)ref_count); 
		 if(l==0)
		 { 
            Render* t=static_cast<Render*>(this)
			 delete t;
		 }
	 };
	 ~AviSynth_data_wrapper(){};
 };
