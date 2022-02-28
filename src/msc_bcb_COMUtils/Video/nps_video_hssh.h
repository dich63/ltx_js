#pragma once

#include "ipc_ports/hssh.h"
#include "ipc_ports/lz4_fast.h"
#include "video/IShared_Image.h"
#include "video/yuv4526.h"
#include "video/filters2d_base.h"
#include "video/capture_base.h"

#define s_m(s) L#s
#define update_arguments(zz,x) zz[s_m(x)].update(x)
#define  update_args(a) update_arguments(args,a)
//#define CHECK_FAIL_hr(a)  CHECK_FAIL_(hr,a)

template <int Ver=0>
struct nps_video_base_t
{
	enum{
		multe=1,

		kbyte=1024,
		mbyte=kbyte*kbyte,
		buf_size=16*mbyte
	};
	typedef unsigned char byte_t;
	typedef signed char int8_t;
	typedef unsigned int uint32_t;
    typedef uint32_t* puint32_t;
	
	typedef long long int64_t;





	struct region_t{
		uint32_t size;
		void* ptr;
	};


	

	template <class Args>
	nps_video_base_t(Args& _args):hrg(E_FAIL)
	{
		void  (*cs_printfW)(DWORD attr,wchar_t* fmt,...);
		HMODULE hlib=LoadLibraryW(L"nps_detect.dll");
		SET_PROC_ADDRESS(hlib,cs_printfW);
		logmsg_attr=cs_printfW;
		//SET_PROC_ADDRESS(GetModuleHandleA("ntdll.dll"),RtlTimeToSecondsSince1970);

		{		
			argv_ini<wchar_t> ini;
			v_buf<wchar_t> mp;
			mp=ini.get_module_path();
			lz4=mp.cat(L"lz4_fast.dll").get();

		}





		args=_args;




		hsslib_pfn=args[L"hssh.lib"];
		if(hsslib_pfn)
		{
			hsslib.hlib=LoadLibraryW(hsslib_pfn);
			hsslib.init();
		}

		if(hrg=hsslib.hr)
		{
			logmsg_attr(M_BC|0x04,L"module \"hss_socket.dll\" not found \n");
			return;
		}
		logmsg_attr(M_BC|0x02,L"module \"hss_socket.dll\" loaded -ok\n");	

		nsport=args[L"port"].def(L"::nps.rtv");



	}

	

template <class A>
static void* console_previewer(A& args,const wchar_t* key,bool fasyn=1)
{
	HMODULE hlib=LoadLibraryW(L"nps_detect.dll");
	void*  (*start_console_previewer_shared_imageW)(const wchar_t* argszz,const wchar_t* ,bool fasyn);

	if(SET_PROC_ADDRESS(hlib,start_console_previewer_shared_imageW))
	{ 
		return start_console_previewer_shared_imageW(args.flat_str(),key,fasyn);
	}
	return 0;
 
}



	HRESULT hrg;
	argv_zzs<wchar_t> args;
	const wchar_t* nsport;	
	const wchar_t* hsslib_pfn;
	hssh_t hsslib;
	lz4_helper_t lz4;	
	void  (*logmsg_attr)(DWORD attr,wchar_t* fmt,...);
};

template <int Ver=0>
struct nps_video_client_t:nps_video_base_t<Ver>{
	typedef nps_video_client_t<Ver> client_t;
	typedef client_t *pclient_t ;


    

	template <class Args>
	nps_video_client_t(Args& _args):nps_video_base_t<Ver>(_args),hovl(0)
	{



		shared_key=args[L"output.key"].def(L"yuv4526out");
		shared_image=shared_key;
		/*
		if(hrg=BMP_helper_t(shared_image).hr)
		{
			logmsg_attr(M_BC|0x04,L"video output.key \"%s\" open fail \n",shared_key);
			return;
		}

		logmsg_attr(M_BC|0x02,L"video output.key \"%s\" open \n",shared_key);
		*/

		host_port=args[L"host_port"].def(L"localhost:7777");

		buffer1.resize(buf_size);
	    buffer2.resize(buf_size);
		testbuf.resize(buf_size/2);
		
	}

	HSS_INFO* connect()
	{
		
		hsslib;
		frame_count=0;		
		totalbytes=0;
		packsize=0;
		return hovl=hsslib.websocket_connect2W((wchar_t*)host_port,(wchar_t*)nsport,0);		
		
	}

	 HRESULT loop_once()
	 {
		 HRESULT hr;
		 region_t packet={};
		 region_t* pregions;
		 uint32_t count=0;

		 OLE_CHECK_hr(get_next_packet(packet,!(frame_count)));
		 //if(frame_count) 
		    packsize=int(packet.size);
			 totalbytes+=packsize;
		 pregions=unmarshal_data(packet,count);
		 if(count<2) return S_OK;
		 OLE_CHECK_hr(image_processing(count,pregions));
		 //BMP_io_t<>::bmp_file_t* pbf= image_yuv4526.get_buf<BMP_io_t<>::bmp_file_t>();
		 BMP_io_t<>::bmp_file_t* pbf= image_rgb.get_buf<BMP_io_t<>::bmp_file_t>();
		 OLE_CHECK_hr(shared_image->PushFrame(pbf));
		 frame_count++;

		 return S_OK;

	 }

	HRESULT get_next_packet(region_t& packet,bool as_key_frame=0)
	{   
		wbs_frame_base_t wbs_frame;
      
		
		const wchar_t sfk0[]=L"frame.iskey=0\0\0";
		const wchar_t sfk1[]=L"frame.iskey=1\0\0";

		hssh_t &hssh=hsslib;

		HRESULT hr;
		 wchar_t* psend=(wchar_t*) (as_key_frame?sfk1:sfk0);
		int cb=sizeof(sfk0);

		
		if(FAILED(hr=hssh.write_wbs_frame(hovl,psend,&cb,WSF_FIN|WSF_BIN,0)))
			return hr;

		 


		 
		if(FAILED(hr=hssh.read_wbs_frame(hovl,1,0,0,&wbs_frame)))			
			return hr;

		   cb=wbs_frame.len;
		   byte_t* p=get_alter_buffer<byte_t>(); 

		   if(FAILED(hr=hssh.read_wbs_frame(hovl,0,p,&cb,0)))			
			   return hr;

		   packet.size=cb;
		   packet.ptr=p;

		return S_OK;

	}


	region_t* unmarshal_data(region_t& packet,uint32_t& count)
	{
		char* ps=(char*)packet.ptr;
		//const int cbs=buf_size,cbd=buf_size;
		char* pd=get_alter_buffer<char>(ps);
		

		int cb;

		cb= lz4.decompress(ps,pd,packet.size,buf_size);
		char* p=pd;
		count=*puint32_t(p);		
		p+=sizeof(uint32_t);
		regions.resize(count);
		
		region_t* pregions=&regions[0];
		//argv_zz<wchar_t> test((wchar_t*)regions[0].ptr);
		if(count>1)
		for(int k=0;k<count;k++)
		{
			region_t& r=pregions[k];
			uint32_t c=*puint32_t(p);
			r.size=c;
			r.ptr=(p+=sizeof(uint32_t));
			p+=c;			
		}

		return pregions;
		

		
		


/*
		std::strstream stream(pd,cb);
		count=0;
		stream.read((char*)&count,sizeof(uint32_t));
		if(!count) return 0;
		regions.resize(count);
		int cbout=0;
		for(int k=0;k<count;k++)
		{
            region_t &r=regions[k];
			stream.read((char*)&r.size,sizeof(uint32_t));
			cbout+=sizeof(uint32_t);
			stream.read((char*)r.ptr,r.size);
			cbout+=r.size;
		};
		
		
		return &regions[0];
		*/

	}

	HRESULT image_processing(int count, region_t* regions)
	{
		struct {
			int crc32;
			int iskey;
			int width;
			int height;

		} frame={};

		
		
		
		HRESULT hr;
		if(count<1) return hr=E_INVALIDARG;
		argv_zz<wchar_t> opts((wchar_t*)regions[0].ptr);

		int mask_index=opts[s_m(region.mask)].def<int>(0x7fffffff);
		int pixel_index=opts[s_m(region.pixels)].def<int>(0x7fffffff);
		if(pixel_index>=count) return hr=E_INVALIDARG;

        region_t& mask_region=regions[mask_index]; 		
		region_t& pixel_region=regions[pixel_index]; 
		

		update_arguments(opts,frame.crc32);
		update_arguments(opts,frame.iskey);
		if(frame.iskey)
		{
			update_arguments(opts,frame.width);
			update_arguments(opts,frame.height);	
			
			if((frame.height*frame.width*4)!=pixel_region.size)
				return hr=E_INVALIDARG;
			image_yuv4526.create(frame.width,frame.height,32,frame.width*4,pixel_region.ptr);			
			image_rgb.create(2*frame.width,2*frame.height,24);			
			return S_OK;
		};

		 if(mask_index>=count) return hr=E_INVALIDARG;
		 uint32_t size=image_yuv4526.get_byte_width()*image_yuv4526.height();
		 byte_t* pbits=image_yuv4526.get_bits();

		 yuv_comparator_t<>::update_frame(size,pbits,mask_region.ptr,pixel_region.ptr);	
		 crc32=Crc32(pbits,size/multe);
		 /*
		 if(crc32!=frame.crc32)
			 logmsg_attr(M_BC|0x04,L"[%d] crc32 incorrect\n",int(frame_count));
		 else logmsg_attr(M_BC|0x02,L"[%d] crc32 ok\n",int(frame_count));
		 */


		 //static uint32_t set_delta_bitmask(uint32_t sizeb, void* last_frame,void* curr_frame,void* bitmasks_buff,void* diff_frame)
		 

		 //convert_YUV4526_to_RGB_bits<yuv_packer_4526_t<> >(w_in/2,h_in/2,pyuv_out,32,lszb,pRGB_out);
		 int width=image_yuv4526.width();
		 int height=image_yuv4526.height();

		 convert_YUV32_to_RGB_bits<yuv_packer_4526_t<> >(width,height,pbits,24,image_rgb.get_byte_width(),image_rgb.get_bits());

		 

         //rg.size=yuv_comparator_t<>::set_delta_bitmask(size,plf,pcf,pmask,pdiff)*sizeof(uint16_t);
		 //static uint32_t update_frame(uint32_t size, void* frame_buff,void* bitmasks_buff,void* diff_frame_buff)
		 //

		return S_OK;
	}

	template <class N>
	inline N* get_alter_buffer(void* pbuf=0)
	{      
		
		char* pb=buffer1.get(),*pe=pb+buf_size,*p=(char*)pbuf;

		
		if((pb<=p)&&(p<pe))
			return (N*)buffer2.get();

		else 
			return (N*)pb;		
	}


	HSS_INFO *hovl; 
	shared_image_helper_t shared_image;
	const wchar_t* shared_key;
	const wchar_t* host_port;

	BMP_io_t<> image_yuv4526,image_rgb;
	int64_t frame_count;
	v_buf<char> buffer1,buffer2;

	std::vector<region_t> regions;
	double totalbytes,packsize;
	int crc32;

    v_buf<char> testbuf;



};


template <int Ver=0>
struct nps_video_server_t:nps_video_base_t<Ver>{

	typedef nps_video_server_t<Ver> server_t;
	typedef server_t *pserver_t ;


	struct connection_t
	{
		connection_t(HSS_INFO* _hovl,pserver_t _owner):hovl(_hovl)
			,owner(_owner),hssh(_owner->hsslib),buffer1(16*mbyte),buffer2(16*mbyte){

				shared_image=owner->shared_image;
				///width=shared_image.width()
				BMP_helper_t bmp(shared_image);			
				set_key_frame();         
		}



		HRESULT read_ws_header()
		{
			HRESULT hr;
			hr=hssh.read_wbs_frame(hovl,1,0,&(message_size=0),&wbs_frame);
			return hr;

		}
		const wchar_t* data_type()
		{
			wchar_t* bt[]={L"unknown",L"string",L"binary"};
			int optcode=wbs_frame.opcode&(WSF_TXT|WSF_BIN);
			return bt[optcode];
		}


		HRESULT read_command(argv_zzs<wchar_t>& args)
		{
			HRESULT hr;
			char* p=(char*)buffer1.get();
			int optcode=wbs_frame.opcode&(WSF_TXT|WSF_BIN);			
			int c=wbs_frame.len;
			hr=hssh.read_wbs_frame(hovl,0,p,&c,0);
			*((DWORD*)(p+c))=0;
			
			if(optcode==1)
				args.init(char_mutator<>(p,false,true,CP_UTF8),true);
			else if(optcode==2) 
				args.init((wchar_t*)p,true);
			else return E_FAIL;

			return S_OK;
		}



		region_t marshal_data(int count,region_t* rs)
		{
			char* ps=buffer1.get();
			int cbs=buffer1.size_b();
			char* pd=buffer2.get();
			int cbd=buffer2.size_b();
			std::strstream stream(ps,cbs);
			int cb=0,cbpack;

			stream.write((char*)&count,sizeof(uint32_t));
			cb+=sizeof(uint32_t);

			for(int k=0;k<count;k++)
			{
				stream.write((char*)&rs[k].size,sizeof(uint32_t));
				cb+=sizeof(uint32_t);
				stream.write((char*)rs[k].ptr,rs[k].size);
				cb+=rs[k].size;
			};
			cbpack= owner->lz4.compress(ps,pd,cb,16*mbyte);
			region_t r={cbpack,pd};
			return r;

		}


		HRESULT image_processing( region_t& rg )
		{
			BMP_helper_t bmp(shared_image);
			if(bmp.hr) return bmp.hr;
			int size=bmp.bits_size_b();
			
			byte_t* pcf=bmp.bits<byte_t>();
			crc32=Crc32(pcf,size/multe);
			byte_t* pmask=mask_buffer.get();
			byte_t* plf=image_buffer.get();
			byte_t* pdiff=diff_buffer.get();
			rg.ptr=pdiff;
			 clear_mask();
			  
			  rg.size=yuv_comparator_t<>::set_delta_bitmask(size,plf,pcf,pmask,pdiff)*sizeof(uint32_t);

			  char* ptest=testbuf;
			  int mcc;
			  if(rg.size)
			  {

			  

			  
			  memcpy(ptest,plf,size);      
			  yuv_comparator_t<>::update_frame(size,ptest,pmask,pdiff);	
			  mcc=memcmp(ptest,pcf,size);
			  if(mcc) FatalAppExitW(0,L" yuv_comparator_t<> error");
			  }



			  memcpy(plf,pcf,size);      
              
				  //memcpy(pbuffer,bmp.bits<byte_t>(),bufsize);                
                


             return 0;
		}


		int loop()
		{
			HRESULT hr;
			argv_zzs<wchar_t> cmd,header;
			if(FAILED(hr=read_ws_header()))
			{

				owner->logmsg_attr(M_BC|0x04,L"error reading header=%s\n",error_msg(hr));

				return 0;
			}
			const wchar_t* mt=data_type();
			//owner->logmsg_attr(M_BC|0x02,L"message_type=%s\n",mt);
			if(FAILED(hr=read_command(cmd)))
			{
				owner->logmsg_attr(M_BC|0x04,L"error reading command=%s\n",error_msg(hr));
			}

			int fk=cmd[L"frame.iskey"].def<int>(0);

            header[L"frame.iskey"]=fk;
			header[L"timestamp"]=owner->getTime();
			
			



			region_t regions[3]={{0,0},{mask_buffer.size_b(),mask_buffer.get()},{image_buffer.size_b(),image_buffer.get()}};
			int reg_count=3;
			if(fk)
			{
                
				set_key_frame();
				header[L"frame.width"]=width;
				header[L"frame.height"]=height;
				header[L"region.mask"]=L"1";
				header[L"region.pixels"]=L"2";

			}
			else {

                 image_processing(regions[2]);
				 
				 if(regions[2].size)
				 {
					 
					 header[L"region.mask"]=L"1";
					 header[L"region.pixels"]=L"2";
					 
				 }
				 else reg_count=1;
				 				 
			}
			int cszz=0;
			  header[L"frame.crc32"]=int(crc32);

			

			wchar_t* ph=(wchar_t*)header.flat_str(0,0,&cszz);
			region_t r0={(cszz+1)*sizeof(wchar_t),ph};
			regions[0]=r0;			


			region_t rout=marshal_data(reg_count,regions);
			
//extern "C" HRESULT hss_write_wbs_frame(void* hsinfo,void* buf,int* pcb,int flags=WSF_FIN|WSF_TXT,int mask=0);

			int c=rout.size;
            if(FAILED(hr=hssh.write_wbs_frame(hovl,rout.ptr,&c,WSF_FIN|WSF_BIN,0)))
				return 0;


			return 1;
		}

		double set_key_frame()
		{			
			BMP_helper_t bmp(shared_image);
			width=bmp.width();
			height=bmp.height();
			bufsize=bmp.bits_size_b();
			pbuffer=image_buffer.resize(bufsize).get();
			diff_buffer.resize(bufsize);
			pmask=mask_buffer.resize(mask_size=(width*height)/8+8).get();
			init_mask();
			testbuf.resize(buf_size/2);
			memcpy(pbuffer,bmp.bits<byte_t>(),bufsize);
			crc32=Crc32(pbuffer,bufsize/multe);
			return timestamp_key=timestamp=owner->getTime();
		}


		void init_mask() {
			clear_mask(0xff);
		}
		void clear_mask(int b=0){
			memset(pmask,b,mask_size);
		}
		int width,height,bpp,bufsize,mask_size;
		v_buf<byte_t> mask_buffer,image_buffer,diff_buffer;
		v_buf<char> buffer1,buffer2;
		char *pbuf1,*pbuf2;    
		v_buf<wchar_t> msgbufffer;



		byte_t *pmask,*pbuffer; 
		double timestamp,timestamp_key;

		shared_image_helper_t shared_image;
		pserver_t owner;	
		hssh_t& hssh;
		HSS_INFO* hovl;
		int message_size;
		wbs_frame_base_t wbs_frame;


		int crc32;

		v_buf<char> testbuf;
	};


	static  void  __stdcall s_on_hss_accept(HSS_INFO* hovl)
	{
		pserver_t(hovl->user_data)->on_hss_accept(hovl);  

	}
	void on_hss_accept(HSS_INFO* hovl)
	{

		hssh_t& hssh=hsslib; 
		ULONG st;
		HRESULT hr;

		st=hssh.HTTP_request_headers_chain(hovl);
		st=hssh.websocket_handshake_reply(hovl);
		if(!(hovl->reply_state&1)) return ;


		char* ra=hssh.reset_SOCKET_DATA(hovl,HSD_REMOTE_INFO,0,0,0);
		logmsg_attr(M_BC|0x0e,L"connect[%s]\n",(wchar_t*)char_mutator<>(ra));

		//		rpc_connect_t rpc(hovl);

		int fv=hssh.wbs_version(hovl);
		connection_t connection(hovl,this);
		while(connection.loop()){};



		/*
		if(fv<=0) 
		while(rpc.loop_once());
		else while(rpc.loop_once_new());
		*/

		//logmsg_attr(M_BC|0x0e,"disconnect[%s]\n",ra);

		//hovl->user_data=ULONG_PTR((void*)this);

	}


	template <class Args>
	nps_video_server_t(Args& _args):nps_video_base_t<Ver>(_args),hport(0)
	{
			

		
		shared_key=args[L"input.key"].def(L"yuv4526");
		shared_image=shared_key;
		if(hrg=BMP_helper_t(shared_image).hr)
		{
			logmsg_attr(M_BC|0x04,L"video input.key \"%s\" not found \n",shared_key);
			return;
		}
		logmsg_attr(M_BC|0x02,L"video input.key \"%s\" open \n",shared_key);

		hport=hsslib.create_socket_portW((wchar_t*)nsport,HSF_SRD_AUTO,&s_on_hss_accept,(void*)this);
		if(!hport) {
			logmsg_attr(M_BC|0x04,L"nsport \"%s\" fail \n",nsport);
			hrg=hsslib.error();
			return ;
		}
		logmsg_attr(M_BC|0x02,L"nsport \"%s\" created\n",nsport);


	}
	int wait(int timeout=-1)
	{
		return hsslib.wait_signaled(hport,timeout);

		return 0;
	}


	double getTime()
	{
		ULONG64 ft=0;
		ULONG tics=0;
		GetSystemTimeAsFileTime((FILETIME*)&ft);
		//RtlTimeToSecondsSince1970&&RtlTimeToSecondsSince1970(&ft,&tics);
		return ft;

	}


	
	void * hport;
	shared_image_helper_t shared_image;
	const wchar_t* shared_key;
	///BOOL (__stdcall *RtlTimeToSecondsSince1970)(ULONG64* ,ULONG*);



};