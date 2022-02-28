#pragma once 

#include "ipc_marshal.h"

#include <map>
#include <list>
#include "video/singleton_utils.h"

#include <string>
struct ipc_marshal_map_t
{

	typedef ipc_marshal_data_t::region_t region_t;
	typedef std::map<std::string,region_t*>  map_t;
	typedef std::list<region_t>  list_t;
	//typedef mutex_cs_t mutex_t; 
	map_t rmap;
	list_t rlist;


	region_t* reset_region(char* name,region_t *pr=0)
	{
		std::pair<map_t::iterator,bool> pm;
       if(pr)
	   {
		   pm=rmap.insert(std::make_pair(name,pr));
		   return (pm.second)? pm.first->second:NULL; 
	   }
	   else
	   {
		   map_t::iterator i=rmap.find(name);
		   return (i!=rmap.end())?  rmap.erase(i)->second:NULL;
	   }
		
	}

   	region_t* get_region(char* name)
	{
		map_t::iterator i;
		i=rmap.find(name);
		return (i!=rmap.end())?i->second:NULL;
	}

	 region_t* clone(region_t& d, region_t& s)	
	 {
		 //if(attr==0)
		 {
			 
		 }
		 return &d;
	 };

	region_t* set_region(char* name,int attr,int sizeb,void* buf)
	{
		list_t::iterator il;
		region_t r;
		r.attr=attr;
		r.ptr=(char*)buf;
		r.sizeb=sizeb;
		il=rlist.insert(rlist.begin(),region_t());
		region_t* pr=clone(*il,r);
		reset_region(name,pr);
		return pr;
	}



	//inline attach_region();
};


struct ipc_marshal_data_impl_t:ipc_marshal_data_t
{

	template <class iterator>
       ipc_marshal_data_impl_t(iterator b,iterator e){
           int count = std::distance(b,e);
		   init(b,count);
	   }
  template <class iterator>
	   ipc_marshal_data_impl_t(iterator b,int count){
		   		   init(b,count);
	   }

template <class iterator>
ipc_marshal_data_impl_t& init(iterator b,int count)  {
		   

           
	     return *this;

	   }



};


