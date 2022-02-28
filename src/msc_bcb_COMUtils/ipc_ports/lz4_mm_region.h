#pragma once
//lz4_mm_region.h



#include "ltxjs.h"
#include "ipc_ports/lz4_fast.h"

#define swap32 _byteswap_ulong

#define CHUNKSIZE (8<<20)    // 8 MB
#define CACHELINE 64
#define ARCHIVE_MAGICNUMBER 0x184C2102
#define ARCHIVE_MAGICNUMBER_SIZE 4


//**************************************
// Architecture Macros
//**************************************
static const int one = 1;
#define CPU_LITTLE_ENDIAN (*(char*)(&one))
#define CPU_BIG_ENDIAN (!CPU_LITTLE_ENDIAN)
#define LITTLE_ENDIAN32(i)   (CPU_LITTLE_ENDIAN)? i:swap32(i)
//LZ4_uncompress_unknownOutputSize

typedef i_mm_region_ptr::int64_t int64_t;

#pragma pack(push)
#pragma pack(2)
struct info_frame_t
{
   int magic_info;
   int frame_size;
   int64_t  decompressed_size;
   VARTYPE element_type; 
   int magic;

};
#pragma pack(pop)


extern "C"  HRESULT lz4_compress_mm_region(IN i_mm_region_ptr* in_region,REFIID iid,void** pp_obj,bool fpersist=false)
{
	
	OLE_CHECK_PTR(pp_obj);
	OLE_CHECK_PTR(in_region);

	HRESULT hr;






	
	 int64_t sizeb,maxbuffsize,length;
	 VARTYPE vt;
	 OLE_CHECK_hr(in_region->GetTotalSize(&sizeb));
     OLE_CHECK_hr(in_region->GetElementInfo(&vt));
	 //GetElementInfo(VARTYPE* pvt,int64_t* pCount=0,long* pElementSize=0)


	 int64_t count=sizeb/CHUNKSIZE;
	 int64_t rem=sizeb-count*CHUNKSIZE;

	 maxbuffsize=LZ4_compressBound(CHUNKSIZE)*(count+1);
	 variant_t vsize=double(maxbuffsize);

     ipc_utils::smart_ptr_t<IBindFactory> factory;
 	 OLE_CHECK_hr(factory.CoGetObject(L"callback.factory"));

	 ipc_utils::smart_ptr_t<i_mm_region_ptr> out_region;
	 OLE_CHECK_hr(factory->bind_object_args(L"ltx.bind:mm_buffer:length=#1;type=uint8",__uuidof(i_mm_region_ptr),out_region._ppQI(),1,&vsize));     


	 unsigned long long filesize = 0;
	 unsigned long long compressedfilesize = ARCHIVE_MAGICNUMBER_SIZE;
	 unsigned int u32var;

	 char* in_buff;
	 char* out_buff;
	 int outSize;

	 // Write Archive Header
	 u32var = ARCHIVE_MAGICNUMBER;
	 

	 {
		  i_mm_region_ptr::ptr_t<info_frame_t> info_ptr(out_region,0);
		  OLE_CHECK_hr(out_ptr);
		  info_ptr->magic_info=LITTLE_ENDIAN32( 0x184D2A50);
		  info_ptr->frame_size=LITTLE_ENDIAN32( sizeof(int64_t)+sizeof(VARTYPE));
		  info_ptr->decompressed_size=LITTLE_ENDIAN32(sizeb);
		  info_ptr->element_type=LITTLE_ENDIAN32(vt);
		  info_ptr->magic=LITTLE_ENDIAN32( ARCHIVE_MAGICNUMBER);	  

		  compressedfilesize +=sizeof(info_frame_t);

		  // out_buff=out_ptr;		  
		  //*(unsigned int*)out_buff = LITTLE_ENDIAN32(u32var);	 
		  
	 }

	 
	 int64_t sz_item_out=LZ4_compressBound(CHUNKSIZE+4);

	 for(int64_t c=0;c<=count;c++)
	 {	
		 

		   int64_t sz_item_in=(c<count)?CHUNKSIZE:rem;
		   if(sz_item_in==0) break;


		  i_mm_region_ptr::ptr_t<char> in_ptr(in_region,sz_item_in,c*sz_item_in);
		  OLE_CHECK_hr(in_ptr);


		  i_mm_region_ptr::ptr_t<char> out_ptr(in_region,sz_item_out,compressedfilesize);
		  OLE_CHECK_hr(out_ptr);

		  in_buff=in_ptr;           
		  out_buff=out_ptr;	 
		 

		 // Compress Block
		 outSize = LZ4_compress(in_buff, out_buff+4, sz_item_in);
		 compressedfilesize += outSize+4;

		 // Write Block		 
		 * (unsigned int*) out_buff = LITTLE_ENDIAN32(outSize);
		 		 
	 }
      

	  hr=out_region->DataViewAs(L"uint8",!fpersist,0,compressedfilesize,iid,pp_obj);	 

	  return hr;
      
	

}

/*

extern "C"  HRESULT lz4_decompress_mm_region(IN i_mm_region_ptr* in_region,REFIID iid,void** pp_obj,bool fpersist=false)
{
	typedef i_mm_region_ptr::int64_t int64_t;
	OLE_CHECK_PTR(pp_obj);
	OLE_CHECK_PTR(in_region);

	HRESULT hr;

	int64_t sizeb,maxbuffsize;
	OLE_CHECK_hr(in_region->GetTotalSize(&sizeb));
	return hr;

}
*/