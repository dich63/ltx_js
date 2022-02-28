// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#include <iostream>
#include "ltxjs.h"

void child() {


	typedef ipc_utils::com_dispcaller_t caller_t;
	ipc_utils::COMInitF_t ci;

	std::cout << "Hell...\n";
	caller_t parent = olestr_t("ltx.bind:external");

	int len = parent["length"];
	std::cout <<"len="<<len<< "\n";
	double r = 0;

	for (auto k = 0; k < len; k++) {
		std::cout << "a[" << k<<"]="<< double(parent[k]) << "\n";
		r += double(parent[k]);
	}
	std::cout << "be r...\n";
	parent["result"] = r;
	std::cout << "end see...\n";
}


void main(int argn, char** argv) {

	

	wchar_t s[128];
	_itow_s(44, s, 128, 10);

	try {		
		child();
		return;
	}
	catch (...) {
	}

	typedef ipc_utils::com_dispcaller_t caller_t;

	//	caller_t process = olestr_t("ltx.bind:process: file=notepad.exe");
	//
	caller_t process = olestr_t("ltx.bind:process: file=")+ olestr_t(argv[0]);// dbg.hook=1");

	variant_t result = process(12, 2, 3, 111.001);

	//std::cout << double(result) << "\n";

	
}
	/*
int main(int argn, char** argv){
	HRESULT hr;
	//caller_t jc("ltx.bind:script:");
	ipc_utils::COMInitF_t ci;
	ipc_utils::smart_ptr_t<i_mm_region_ptr> sm;
	//CoGetObject

	hr = ::CoGetObject(olestr_t("ltx.bind:mm_buffer:len=7777;type=double"), NULL ,__uuidof(i_mm_region_ptr), (void**)&sm.p);
	//hr = ipc_utils::bindObject("ltx.bind:mm_buffer:len=#1;type=#2", sm)(1111,"double");

	caller_t smo(sm);

	int len = smo["length"];
	
	i_mm_region_ptr::ptr_t<double> pm(sm.p, -1);
	int at = ipc_utils::COMInit_t::apartment_type();
	caller_t jj = ipc_utils::interact_js();
    std::cout << "Hell...\n"; 

}
	*/
