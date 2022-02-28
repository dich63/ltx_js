#pragma once
//#include "filters2d.h"

#include "filters2d_base.h"
#include "fast_median2d.h"
#include "math.h"



template <int _BPS1,int _BPS2,int Ver=0>
struct  filter_RGB_RGB_t:filter_base_t<Ver>
{
	enum{
		offsB_S=_BPS1/8,
		offsB_D=_BPS2/8
	};
	
	filter_RGB_RGB_t(int _nthread,int _width,int _height,int _src_byte_width=0,int _dest_byte_width=0)
		:filter_base_t<Ver>(_nthread,_width,_height,_src_byte_width,_dest_byte_width)
	{
		//if(!src_byte_width) 
		src_byte_width=(_src_byte_width)?_src_byte_width:offsB_S*width;
		dest_byte_width=(_dest_byte_width)?_dest_byte_width:offsB_D*width;	


	}

	virtual void make_frame_node(byte_t* psrc, byte_t* pdest,int b_row,int e_row){

		//byte_t* prs=psrc;
		//byte_t* prd=pdest;
		for (int n=b_row;n<=e_row;n++)
		{
			byte_t* ps=psrc+n*src_byte_width;
			byte_t* pd=pdest+n*dest_byte_width;

			for(int m=0;m<width;m++,ps+=offsB_S,pd+=offsB_D)
		 {
			 pd[0]=ps[0];pd[1]=ps[1];pd[2]=ps[2];

		 }

		}



	};
};




template <int _BPS=24 ,int Ver=0>
struct  filter_RGB_grayscale_t:filter_base_t<Ver>
{
	enum{
		 offsB=_BPS/8
	};
	frgba_t rgb;
	filter_RGB_grayscale_t(int _nthread,int _width,int _height,int _src_byte_width=0,int _dest_byte_width=0)
		:filter_base_t(_nthread,_width,_height,_src_byte_width,_dest_byte_width)
	{
		//if(!src_byte_width) 
		src_byte_width=(_src_byte_width)?_src_byte_width:offsB*width;
		dest_byte_width=(_dest_byte_width)?_dest_byte_width:width;

		frgba_t  r={1./3.,1./3.,1./3.};
		 rgb=r;



	}

	virtual void make_frame_node(byte_t* psrc, byte_t* pdest,int b_row,int e_row){

		//byte_t* prs=psrc;
		//byte_t* prd=pdest;
		for (int n=b_row;n<=e_row;n++)
		{
			byte_t* ps=psrc+n*src_byte_width;
			byte_t* pd=pdest+n*dest_byte_width;

			for(int m=0;m<width;m++,ps+=offsB)
		 {
		  //	 uint16_t r=uint16_t(ps[0])+uint16_t(ps[1])+uint16_t(ps[2]);
			// pd[m]=r/3;
			 uint16_t r=rgb.b*uint16_t(ps[0])+rgb.g*uint16_t(ps[1])+rgb.r*uint16_t(ps[2]);
			 pd[m]=r;

		 }


		}



	};
};







template <int _BPS=24 ,int Ver=0>
struct  filter_RGB_grayscale_invert_t:filter_base_t<Ver>
{
	enum{
		offsB=_BPS/8
	};
	filter_RGB_grayscale_invert_t(int _nthread,int _width,int _height,int _src_byte_width=0,int _dest_byte_width=0)
		:filter_base_t(_nthread,_width,_height,_src_byte_width,_dest_byte_width)
	{
		//if(!src_byte_width) 
		src_byte_width=(_src_byte_width)?_src_byte_width:offsB*width;
		dest_byte_width=(_dest_byte_width)?_dest_byte_width:width;

	}

	virtual void make_frame_node(byte_t* psrc, byte_t* pdest,int b_row,int e_row){

		//byte_t* prs=psrc;
		//byte_t* prd=pdest;
		for (int n=b_row;n<=e_row;n++)
		{
			byte_t* ps=psrc+n*src_byte_width;
			byte_t* pd=pdest+n*dest_byte_width;

			for(int m=0;m<width;m++,ps+=offsB)
		 {
			 uint16_t r=uint16_t(ps[0])+uint16_t(ps[1])+uint16_t(ps[2]);
			 byte br=r/3;
			 pd[m]=~br;
		 }


		}



	};
};




template <int _BPS=24 ,int Ver=0>
struct  filter_grayscale_RGB_t:filter_base_t<Ver>
{
	enum{
		offsB=_BPS/8
	};
	
	frgba_t rgba;

	filter_grayscale_RGB_t(int _nthread,int _width,int _height,int _src_byte_width=0,int _dest_byte_width=0)
		:filter_base_t(_nthread,_width,_height,_src_byte_width,_dest_byte_width)
	{
		//if(!src_byte_width) 
		src_byte_width=(_src_byte_width)?_src_byte_width:width;
		dest_byte_width=(_dest_byte_width)?_dest_byte_width:offsB*width;

		frgba_t r={1,1,1,1};
		rgba=r;




	}

	virtual void make_frame_node(byte_t* psrc, byte_t* pdest,int b_row,int e_row){

		//byte_t* prs=psrc;
		//byte_t* prd=pdest;
		for (int n=b_row;n<=e_row;n++)
		{
			byte_t* ps=psrc+n*src_byte_width;
			byte_t* pd=pdest+n*dest_byte_width;

         
			for(int m=0;m<width;m++,pd+=offsB)
		 {
			 byte_t c=ps[m];
			 /*
			 pd[0]=rgba.b*c;
			 pd[1]=rgba.g*c;
			 pd[2]=rgba.r*c;		 			 
			 */
			 //if(n>300)	 c=~c;
			 pd[0]=c;
			 pd[1]=c;
			 pd[2]=c;
			 if(offsB==4) 
				 pd[3]=0;

		 }
		 
		}



	};
};


template <int Ver=0>
struct  filter_median_t:filter_base_t<Ver>
{
	int  rwin,mcache; 
	filter_median_t(int nthread,int _width,int _height,int _src_byte_width=0,int _rwin=7,int _mcache=(8*1024*1024))
		:filter_base_t(nthread,_width,_height,_src_byte_width),rwin(_rwin),mcache(_mcache)
	{		


	}

	virtual void make_frame_node(byte_t* psrc, byte_t* pdest,int b_row,int e_row){

		fast_median2d_t::ctmf(psrc,pdest,width,height,src_byte_width,dest_byte_width,rwin,1,mcache);

	};
};




template <unsigned MUL=4, int Ver=0>
struct  filter_median_delta_t:filter_base_t<Ver>
{
     uint16_t mul;
	double module; 
	filter_median_delta_t(int nthread,int _width,int _height,unsigned _mul=MUL,double _module=0,int _src_byte_width=0)
		:filter_base_t(nthread,_width,_height,_src_byte_width),mul(_mul),module(_module){};

	virtual void make_frame_node(byte_t* psrc, byte_t* pdest,int b_row,int e_row){

		for (int n=b_row;n<=e_row;n++)
		{
			byte_t* ps=psrc+n*src_byte_width;
			byte_t* pd=pdest+n*dest_byte_width;

			if(module)
			{
				for(int m=0;m<width;m++)
				{
					int16_t bs=(int16_t(pd[m])-int16_t(ps[m]))*mul;
					 bs=abs(bs);
					 if(module!=double(1)) 
					 {
                       bs=pow(bs,module);    
					 }
						if(bs>=0){
							if(bs<256)   pd[m]=bs;
							else  pd[m]=255;
						}
						else pd[m]=0;
				}

			}
			else
			for(int m=0;m<width;m++)
			{ 
				int16_t bs=(int16_t(pd[m])-int16_t(ps[m]))*mul;
				if(bs>=0){
					if(bs<256)   pd[m]=bs;
					else  pd[m]=255;
				}
				else pd[m]=0;

				//if(bs<bd)  pd[m]=(pd[m]-ps[m])<<1;
				//	else pd[m]=0;

			}
		}

	};
};



template <int Ver=0>
struct  filter_delta_t:filter_base_t<Ver>
{
	
       int delta;
	   
	   byte_t* pbuffer;
	   std::vector<byte_t> buffer;
	   

	   
	
	filter_delta_t(int nthread,int _width,int _height,int _src_byte_width=0)
		:filter_base_t(nthread,_width,_height,_src_byte_width),delta(-1){ 
			buffer.resize(width*height);
			pbuffer=buffer.size()?&buffer[0]:0;
	 	};



	virtual void make_frame_node(byte_t* psrc, byte_t* pdest,int b_row,int e_row){

		delta=0;
		for (int n=b_row;n<=e_row;n++)
		{
			byte_t* ps=psrc+n*src_byte_width;
			byte_t* pd=pdest+n*dest_byte_width;			
            byte_t* pbuf=pbuffer+n*width;			

			byte_t *ps_end=ps+src_byte_width;	
    
			if(pdest)
				for(;ps<ps_end;ps++,pd++,pbuf++){
					byte_t b=*ps-*pbuf;
					if(b){ delta++;
					*pd=0xFF;}
					else 
					{
						*pd=0;
					}
					*pbuf=*ps;

				}
			else
				for(;ps<ps_end;ps++,pd++,pbuf++)
				{
					byte_t b=*ps-*pbuf;
					if(b) delta++;
					//if(pdest) *pd=b;
					*pbuf=*ps;
				}

		}

	};
};



template <int Ver=0>
struct  filter_median_truncate_t:filter_base_t<Ver>
{
	int  rwin,mcache; 
	filter_median_truncate_t(int nthread,int _width,int _height,int _src_byte_width=0,int _rwin=7,int _mcache=(8*1024*1024))
		:filter_base_t(1,_width,_height,_src_byte_width),rwin(_rwin),mcache(_mcache)
	{		


	}

	virtual void make_frame_node(byte_t* psrc, byte_t* pdest,int b_row,int e_row){

		fast_median2d_t::ctmf(psrc,pdest,width,height,src_byte_width,dest_byte_width,rwin,1,mcache);
		for (int n=b_row;n<=e_row;n++)
		{
			byte_t* ps=psrc+n*src_byte_width;
			byte_t* pd=pdest+n*dest_byte_width;

			for(int m=0;m<width;m++)
          { 
			   int16_t bs=(int16_t(pd[m])-int16_t(ps[m]))<<2;
			   if(bs>=0){
               if(bs<256)   pd[m]=bs;
			     else  pd[m]=255;
			   }
			   else pd[m]=0;

				//if(bs<bd)  pd[m]=(pd[m]-ps[m])<<1;
				//	else pd[m]=0;
					
			}
		}

	};
};






template <int Ver=0>
struct  filter_quantizer_t:filter_base_t<Ver>
{
   inline const byte_t* get_default()
   {
           static byte_t dflt[256]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255};
		   return dflt;
   }

   byte_t filter[256];

   filter_quantizer_t(int nthread,int _width,int _height,int _src_byte_width=0,const byte_t* _filter=0):
   filter_base_t(1,_width,_height,_src_byte_width){

	   filter_load(_filter);
   }

   inline filter_quantizer_t& filter_invert()
   {
	   for(int k=0;k<256;k++) filter[k]=~filter[k];
	   return *this;
   }

   inline filter_quantizer_t& filter_load(const byte_t* _filter=0)
   {
	    memcpy(filter,(_filter)?_filter:get_default(),256);
	   return *this;
   }


   virtual void make_frame_node(byte_t* psrc, byte_t* pdest,int b_row,int e_row){

	   //byte_t* prs=psrc;
	   //byte_t* prd=pdest;
	   for (int n=b_row;n<=e_row;n++)
	   {
		   byte_t* ps=psrc+n*src_byte_width;
		   byte_t* pd=pdest+n*dest_byte_width;

		   for(int m=0;m<width;m++)
		     pd[m]=filter[ps[m]];

	   }



   };


};



template <int Ver=0>
struct  filter_RGBA_t:filter_base_t<Ver>
{
	enum{
		offsB=32/8
	};
	int contour_level,logop;
	
	filter_RGBA_t(int _nthread,int _width,int _height,int _src_byte_width=0,int _dest_byte_width=0)
		:filter_base_t(_nthread,_width,_height,_src_byte_width,_dest_byte_width),contour_level(-1),logop(0)
	{
		//if(!src_byte_width) 
		src_byte_width=(_src_byte_width)?_src_byte_width:offsB*width;
		dest_byte_width=(_dest_byte_width)?_dest_byte_width:offsB*width;



	}

	virtual void make_frame_node(byte_t* psrc, byte_t* pdest,int b_row,int e_row){

		//byte_t* prs=psrc;
		//byte_t* prd=pdest;

		if(logop==-777)
		{
			int cb=src_byte_width*(e_row-b_row+1);
			
			for (int n=b_row;n<=e_row;n++)
			{
				byte_t* ps=psrc+n*src_byte_width;
				byte_t* pd=pdest+n*dest_byte_width;


				for(int m=0;m<width;m++,pd+=offsB,ps+=offsB)
				{
                      pd[3]=ps[3];
				}
			}
			return;

		}
		for (int n=b_row;n<=e_row;n++)
		{
			byte_t* ps=psrc+n*src_byte_width;
			byte_t* pd=pdest+n*dest_byte_width;


			for(int m=0;m<width;m++,pd+=offsB,ps+=offsB)
		 {
			 //*(DWORD*(pd))=*(DWORD*(ps));
			 int16_t a=uint16_t(ps[3]);
			 if(a)		 
			 {
             if(logop==1)
			 {
               pd[0]=~pd[0];
			   pd[1]=~pd[1];
			   pd[2]=~pd[2];

			 }
			 else if(logop==2)
			 {
				 pd[0]=contour_level;
				 pd[1]=contour_level;
				 pd[2]=contour_level;
			 }
			 else  if(logop==16)
			 {
				 if(a<contour_level)
				 {
                   pd[0]=0;
				   pd[1]=0;
				   pd[2]=128;

				 }

			 }
			 else  if(logop==32)
			 {
				 if(a&2)
				 {
					 pd[0]=0;
					 pd[1]=0;
					 pd[2]=128;

				 }else				 
				 {
					 pd[0]=128;
					 pd[1]=0;
					 pd[2]=0;
				 }

			 }
			 else  if(logop==64)
			 {
				 int b;
				 b=-contour_level;
				 if(a&2)
				 {
					 
					 pd[0]=2*uint16_t(pd[0])/b;
					 pd[1]=2*uint16_t(pd[1])/b;
					 pd[2]=1*uint16_t(pd[2])/b;
					 

				 }else				 
				 {
					 pd[0]=1*uint16_t(pd[0])/b;
					 pd[1]=2*uint16_t(pd[1])/b;
					 pd[2]=2*uint16_t(pd[2])/b;
				 }

			 }
			 else{
				 if (contour_level==0)
				 *((DWORD*)pd)=*((DWORD*)ps);
				 else 
				 {
					 if(contour_level<=0)
					 {
						 
						 
						 int b;
							 b=-contour_level;
						 
						 
							 pd[0]=1*uint16_t(pd[0])/b;
							 pd[1]=2*uint16_t(pd[1])/b;
							 pd[2]=2*uint16_t(pd[2])/b;

						 
						 
						

						 
						 
						//  pd[0]=~pd[0];
						  //pd[1]=~pd[1];
						  //pd[2]=~pd[2];
   

					 }
					 else  if(a<contour_level)
					 {
						 /*pd[0]=0;
						 pd[1]=0;
                         pd[2]=255;
						 */
						 pd[0]<<=2;
						 pd[1]<<=2;
						 pd[2]<<=2;

					 }
					 pd[3]=ps[3];
				 }
			 }
				 /*
				 int16_t b=255-a;
				 
				 pd[0]=byte_t( (a*pd[0]+b*ps[0])>>8 );
				 pd[1]=byte_t( (a*pd[1]+b*ps[1])>>8 );
				 pd[2]=byte_t( (a*pd[2]+b*ps[2])>>8 );				 
				 */

			 }

			 
		 }

		}



	};
};


