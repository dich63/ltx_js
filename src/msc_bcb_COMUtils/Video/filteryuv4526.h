#pragma once
//#include "filteryuv4526.h"
#include "npstat_filters.h"
#include "yuv4526.h"

template <class yuv_packer_T=yuv_packer_4526_t<>,int _QColor=0 ,int Ver=0 >
struct  filter_npstat_decay_yuv_t:filter_npstat_decay_base_t<Ver>
{
	enum{
		qcolor=_QColor,
		bpp_y=yuv_packer_T::bpp_y,		
		bpp_uv=yuv_packer_T::bpp_uv-qcolor,
		maxc_y=(1<<bpp_y)-1,
		maxc_uv=(1<<bpp_uv)-1,
		npstat_count_y=(1<<bpp_y),		
		npstat_count_uv=(1<<bpp_uv)

	};

    typedef unsigned char byte_t;
	typedef unsigned int uint32_t;

	typedef filter_alpha_dynamic_base_t<Ver> alpha_dynamic_t;
	typedef typename alpha_dynamic_t::alpha_t alpha_t;
	

	typedef yuv_packer_T yuv_packer_t;



	//	typedef float float_pd_t;
	//
	//
	typedef float_fix_t<14,uint16_t> float_pd_t;
	//	typedef float_fix_t<16,uint32_t> float_pd_t;
	//	typedef float_fix_t<8,uint8_t> float_pd_t;

	typedef  PD_base_t<npstat_count_y,float_pd_t> npstat_base_y_t;
	typedef  PD_t<npstat_count_y,float_pd_t> npstat_y_t;
	typedef  PD_base_t<npstat_count_uv,float_pd_t> npstat_base_uv_t;
	typedef  PD_t<npstat_count_uv,float_pd_t> npstat_uv_t;




	npstat_base_uv_t *pbuffer_u,*pbuffer_v;
	npstat_base_y_t *pbuffer_y00,*pbuffer_y01,*pbuffer_y10,*pbuffer_y11;
	std::vector<npstat_base_uv_t> buffer_u,buffer_v;
	std::vector<npstat_base_y_t> buffer_y00,buffer_y01,buffer_y10,buffer_y11;

	//int buffer_width_u, buffer_width_y;

	alpha_dynamic_t* pfad;





/*
inline static  void blend_value_y(const byte_t src,byte_t* dest,const float_pd_t& b,npstat_base_y_t* nsp)
	{
		
		byte_t s=*src;
		if(s>maxcol) s=maxcol;
		b=1-a;
		//
		//if(nsp->mix_distribution_and_check(s>>bpsh,b))		  *dest=*src;
		//     
		//*dest=nsp->mix_distribution2(s>>bpsh,b)<<bpsh;

		//
		*dest=nsp->fast_mix_distribution(s>>bpsh,(*dest)>>bpsh,b)<<bpsh;
	}


	//inline 

	static  void blend_value(const byte_t* src,byte_t* dest,const double a,npstat_base_t* nsp,byte maxcol=255)
	{
		float_pd_t b;
		//
		//__asm {			int 3  		};
		byte_t s=*src;
		if(s>maxcol) s=maxcol;
		b=1-a;
		//
		//if(nsp->mix_distribution_and_check(s>>bpsh,b))		  *dest=*src;
		//     
		//*dest=nsp->mix_distribution2(s>>bpsh,b)<<bpsh;

		//
		*dest=nsp->fast_mix_distribution(s>>bpsh,(*dest)>>bpsh,b)<<bpsh;
	}

*/

	virtual    filter_npstat_decay_base_t& reset_buffer(double l=25,double r=75,int frandv=0)
	{
		npstat_y_t npsy(l/100.,r/100.);
		npstat_uv_t npsuv(l/100.,r/100.);

		int cb=width*height;  
		buffer_u.resize(cb,npsuv);
		buffer_v.resize(cb,npsuv);
		buffer_y00.resize(cb,npsy);
		buffer_y01.resize(cb,npsy);
		buffer_y10.resize(cb,npsy);
		buffer_y11.resize(cb,npsy);     

		pbuffer_u=buffer_u.size()?&buffer_u[0]:0;
		pbuffer_v=buffer_v.size()?&buffer_v[0]:0;

		pbuffer_y00=buffer_y00.size()?&buffer_y00[0]:0;
		pbuffer_y01=buffer_y01.size()?&buffer_y01[0]:0;
		pbuffer_y10=buffer_y10.size()?&buffer_y10[0]:0;
		pbuffer_y11=buffer_y11.size()?&buffer_y11[0]:0;

		return *this;
	}

	filter_npstat_decay_yuv_t(int _nthread,filter_alpha_dynamic_base_t<Ver>* _pfad, int _width,int _height,int _src_byte_width=0,int _dest_byte_width=0)
		:filter_npstat_decay_base_t<Ver>(_nthread,_width,_height,_src_byte_width,_dest_byte_width),pfad(_pfad)
	{	
		width=_width;
		height=_height;
		src_byte_width=(_src_byte_width)?_src_byte_width:4*width;
		dest_byte_width=(_dest_byte_width)?_dest_byte_width:4*width;
	}

	virtual void make_frame_node(byte_t* psrc, byte_t* pdest,int b_row,int e_row){

		//byte_t* prs=psrc;
		//byte_t* prd=pdest;
		
		for (int n=b_row;n<=e_row;n++)
		{
			int nw=width*n;

			uint32_t* ps=((uint32_t*)psrc)+nw;
			uint32_t* pd=((uint32_t*)pdest)+nw;			

			

			npstat_base_y_t * py00=pbuffer_y00+nw;
			npstat_base_y_t * py01=pbuffer_y01+nw;
			npstat_base_y_t * py10=pbuffer_y10+nw;
			npstat_base_y_t * py11=pbuffer_y11+nw;

			npstat_base_uv_t * pu=pbuffer_u+nw;
			npstat_base_uv_t * pv=pbuffer_v+nw;			

			alpha_t* palpha=pfad->palpha_row(n<<1);
			

				for(int l=0;l<width;l++)				
				{
					double a=pfad->alpha_ref(palpha,l<<1).decay;
					float_pd_t b=double(1-a);
					
					uint32_t pos=ps[l];
					uint32_t& rd=pd[l];
					uint32_t y,uv;
					if(1){
                    byte_t p; 
					p=yuv_packer_t::get_y<0,0>(pos);
					uint32_t y00=py00[l].mix(p,b);
					p=yuv_packer_t::get_y<0,1>(pos);
					uint32_t y01=py01[l].mix(p,b);
					p=yuv_packer_t::get_y<1,0>(pos);
					uint32_t y10=py10[l].mix(p,b);
					p=yuv_packer_t::get_y<1,1>(pos);
					uint32_t y11=py11[l].mix(p,b);	


                     
					y=yuv_packer_t::y_pack(y00,y01,y10,y11);

//					/*
					p=yuv_packer_t::get_u(pos);
					uint32_t u=pu[l].mix(p>>qcolor,b)<<qcolor;				
					p=yuv_packer_t::get_v(pos);
					uint32_t v=pv[l].mix(p>>qcolor,b)<<qcolor;				
					uv=yuv_packer_t::uv_pack(u,v);
					//*/
					//uv=yuv_packer_t::yuvrgb_t::rgb4_to_uv_pack(0,0,0);
					//uv=0;
/*
					if(1)
					{
						uint32_t u=yuv_packer_t::get_u(pos>>qcolor)<<qcolor;
						uint32_t v=yuv_packer_t::get_v(pos>>qcolor)<<qcolor;
						uv=yuv_packer_t::uv_pack(u,v);

					}					
					else uv=yuv_packer_t::yuvrgb_t::rgb4_to_uv_pack(0,0,0);
					*/

					rd=y|uv;}
					else{

						byte_t p; 
						p=yuv_packer_t::get_y<0,0>(pos);
						uint32_t y00=p;
						p=yuv_packer_t::get_y<0,1>(pos);
						uint32_t y01=p;
						p=yuv_packer_t::get_y<1,0>(pos);
						uint32_t y10=p;
						p=yuv_packer_t::get_y<1,1>(pos);
						uint32_t y11=p;	

						y=yuv_packer_t::y_pack(y00,y01,y10,y11);

						uint32_t u=yuv_packer_t::get_u(pos);
						uint32_t v=yuv_packer_t::get_v(pos);
						uv=yuv_packer_t::uv_pack(u,v);
						//uv=yuv_packer_t::yuvrgb_t::rgb4_to_uv_pack(0,255,0);

						rd=y|uv;


					}


				}
				//	blend_pt(ps,pd,a,b,pbuf);
		}
	}

	static filter_npstat_decay_base_t<Ver>* create(int QColor,int _nthread,filter_alpha_dynamic_base_t<Ver>* _pfad, int _width,int _height,int _src_byte_width=0,int _dest_byte_width=0)
	{

		//filter_npstat_decay_yuv_t(int _nthread,filter_alpha_dynamic_base_t<Ver>* _pfad, int _width,int _height,int _src_byte_width=0,int _dest_byte_width=0)
		switch(QColor)
		{
		case 0:  return new filter_npstat_decay_yuv_t<yuv_packer_T,0,Ver>( _nthread, _pfad, _width,_height,_src_byte_width, _dest_byte_width);
		case 1:  return new filter_npstat_decay_yuv_t<yuv_packer_T,1,Ver>( _nthread, _pfad, _width,_height,_src_byte_width, _dest_byte_width);
		case 2:  return new filter_npstat_decay_yuv_t<yuv_packer_T,2,Ver>( _nthread, _pfad, _width,_height,_src_byte_width, _dest_byte_width);
     	case 3:  return new filter_npstat_decay_yuv_t<yuv_packer_T,3,Ver>( _nthread, _pfad, _width,_height,_src_byte_width, _dest_byte_width);
        case 4:  return new filter_npstat_decay_yuv_t<yuv_packer_T,4,Ver>( _nthread, _pfad, _width,_height,_src_byte_width, _dest_byte_width);
		
		}

		return 0;

	}


};
