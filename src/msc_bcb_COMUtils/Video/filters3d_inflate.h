#pragma once
//#include "filters3d.h"
#include "video/filters3d.h"
#include <utility>

template <int Ver=0>
struct  filter_alpha_inflater_t:filter_base_t<Ver>
{
	int radius;
	BMP_io_t<> texture;
	

	inline void texture_init()
	{
		if(!radius) return;
		int siz=2*radius+1;
		texture.create(siz,siz,8);
		byte_t* pbits=texture.get_bits();
		byte_t* p=pbits;
		int wb=texture.byte_width;
		int xc=radius,yc=radius;
		int r2=radius*radius;		
		for(int y=0;y<siz;y++)
		{
			int d2y=(y-yc);
			d2y*=d2y;

			for(int x=0;x<siz;x++)	          
			 {
				 int dx=(x-xc);
				 int d2=d2y+dx*dx;
				 p[x]=(d2<=r2)?1:0;

			 }
			 p+=wb;
		}

		texture.save(L"c:\\temp\\texture.bmp");


	}
	

inline	void set_texture_single_thread(int x,int y,byte_t* pdest)
	{
        int r=radius,siz=2*r+1;
	    int yt=y-r,yb=y+r+1,xl=x-r,xr=x+r+1;
		int txl=0,tyt=0;

		if(yt<0)
		{
			tyt=-yt;
			yt=0;
		}
		if(yb>height)
		{			
			yb=height;
		}

		if(xl<0)
		{
			txl=-xl;
			xl=0;
		}
		if(xr>width)
		{			
			xr=width;
		}

         
		 int twb=texture.byte_width;

         int dbw=dest_byte_width;

          byte_t* pa=pdest+3;
		  byte_t* pt=texture.get_bits()+tyt+twb*txl;

		 for(int y=yt;y<yb;y++)
		 {
			 for(int x=xl;x<xr;x++)
				 pa[x<<2]|=pt[x];

               pa+=dbw;
			   pt+=twb;
		 }
		 

	}

  filter_alpha_inflater_t(int _nthread,int _radius,int _width,int _height,int _src_byte_width=0,int _dest_byte_width=0)
	  :filter_base_t(1,_width,_height,4*_width,4*_width), radius(_radius)
  {
       
		   texture_init();
  }

  virtual void make_frame_node(byte_t* psrc, byte_t* pdest,int b_row,int e_row){

	  memset(pdest,0,dest_byte_width*height);
	  for (int n=0;n<height;n++)
	  {
		  byte_t* psl=psrc+n*src_byte_width;
		  byte_t* pdl=pdest+n*dest_byte_width;

		  for(int m=0;m<src_byte_width;m+=4)
		  {
			  byte_t* ps=psl+m;
			  byte_t* pd=pdl+m;
			  pd[0]=ps[0];
			  pd[1]=ps[1];
			  pd[2]=ps[2];
			  if(ps[3])
			  {
				   pd[3]|=2;
				  if(radius) 
					  set_texture_single_thread(m>>2,n,pdest);				  
			  }
			  
		  }

	  }


  }


};



template <int Ver=0>
struct  filter_alpha_inflater2_t:filter_base_t<Ver>
{

	struct rgba_t
	{
		union{
			struct {
				byte_t c[4];
			} color;
			struct {
				byte_t r,g,b,a;
			};

		};
	};


	int radius;
	BMP_io_t<> texture,alpha;
	byte_t* palpha;
	int nthread2;

	
	


	inline void texture_init()
	{
		if(!radius) return;
		int siz=2*radius+1;
		texture.create(siz,siz,8);
		byte_t* pbits=texture.get_bits();
		byte_t* p=pbits;
		int wb=texture.byte_width;
		int xc=radius,yc=radius;
		int r2=radius*radius;		
		for(int y=0;y<siz;y++)
		{
			int d2y=(y-yc);
			d2y*=d2y;

			for(int x=0;x<siz;x++)	          
			{
				int dx=(x-xc);
				int d2=d2y+dx*dx;
				p[x]=(d2<=r2)?1:0;

			}
			p+=wb;
		}


			//	texture.save(L"c:\\temp\\texture.bmp");


	}


	inline	void set_texture_single_thread(int x,int y,byte_t* pdest,int dbw)
	{
		int r=radius,siz=2*r+1;
		int yt=y-r,yb=y+r+1,xl=x-r,xr=x+r+1;
		int txl=0,tyt=0,txr=siz,tyb=siz;


	
	if(yt<0)
		{
			tyt=-yt;
			yt=0;
		}
		if(yb>height)
		{	
			tyb=siz-yt;
			yb=height;
		}

		if(xl<0)
		{
			txl=-xl;
			xl=0;
		}
		if(xr>width)
		{	
			txr=siz-xl;
			xr=width;
		}
		


		int twb=texture.byte_width;

		

		byte_t* pa=pdest+yt*dbw+xl;
		byte_t* pt=texture.get_bits();

		for(int y=tyt;y<tyb;y++)
		{
			for(int x=txl;x<txr;x++)
				pa[x]=pa[x]|pt[x];

			pa+=dbw;
			pt+=twb;
		}


	}

	filter_alpha_inflater2_t(int _nthread,int _radius,int _width,int _height,int _src_byte_width=0,int _dest_byte_width=0)
		:filter_base_t(1,_width,_height,4*_width,4*_width), radius(_radius),nthread2(_nthread)
	{

		texture_init();
		alpha.create(width,height,8);
		palpha=alpha.get_bits();
		
	}

	virtual void make_frame_node(byte_t* psrc, byte_t* pdest,int b_row,int e_row){

		//memset(pdest,0,dest_byte_width*height);
		int wab=alpha.get_byte_width();
		
		
		
		for (int n=0;n<height;n++)
		{
			rgba_t* ps=(rgba_t* )(psrc+n*src_byte_width);
			rgba_t* pd=(rgba_t* )(pdest+n*dest_byte_width);
			byte_t* pa=palpha+n*wab;

			for(int m=0;m<width;m++)
			{
               rgba_t& s=ps[m];
			   rgba_t& d=pd[m];				
               d.color=s.color;				
				if(s.a)
				{				
					
					pa[m]=0x02;		
					
					 d.a=0x2;
						
				}
				else pa[m]=0x00;		

			}

		}


//		return;

        if(!radius) return;
		int w1=width-1;
		int h1=height-1;
		for (int n=0;n<height;n++)
		{  
		  byte_t* pa=palpha+n*wab;
		  byte_t* pa_up=(n>0)?pa-wab:pa;
		  byte_t* pa_down=(n<h1)?pa+wab:pa;

		   for(int m=0;m<width;m++)
		   {

		   

			   byte_t a=pa[m];
		       byte_t al=(m>0)?pa[m-1]:a;
			   byte_t ar=(m<w1)?pa[m+1]:a;
			   byte_t au=pa_up[m];
			   byte_t ad=pa_down[m];
			   
			   bool f=(!(al&ar&au&ad&0x02))&&(a&0x02);


			//	if(pa[m]&0x02) 
			   if(f)
					set_texture_single_thread(m,n,palpha,wab);
		   }
		}

/*
	     set_texture_single_thread(200,200,palpha,wab);
		 //set_texture_single_thread(20,300,palpha,wab);
		 set_texture_single_thread(500,200,palpha,wab);
		 set_texture_single_thread(width-1,200,palpha,wab);
		 set_texture_single_thread(0,0,palpha,wab);
		 set_texture_single_thread(width-1,height-1,palpha,wab);
		 set_texture_single_thread(width/2,height/2,palpha,wab);
		 */

		for (int n=0;n<height;n++)
		{			
			rgba_t* pd=(rgba_t* )(pdest+n*dest_byte_width);
			byte_t* pa=palpha+n*wab;

			for(int m=0;m<width;m++) pd[m].a=pa[m];
		}
                      



	}


};