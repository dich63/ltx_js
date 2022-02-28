#pragma once

#include "windows.h"
#include "inttypes.h"
#include "stdint.h"
#ifndef UINT64_C 
#define UINT64_C(a) UINT64(a)
#endif

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/pixfmt.h"



#include "wchar_parsers.h"
#include "singleton_utils.h"


namespace ffmpeg_bind
{

	void (* av_close_input_file)(AVFormatContext *s);

	int (*av_read_play)(AVFormatContext *s);
	int (*av_read_pause)(AVFormatContext *s);
	void (*av_close_input_stream)(AVFormatContext *s);

	int (*av_dup_packet)(AVPacket *pkt);
	void (*av_register_all)();
	int (*av_open_input_file)(AVFormatContext **ic_ptr, const char *filename,AVInputFormat *fmt,int buf_size,AVFormatParameters *ap);
	int (*av_read_frame)(AVFormatContext *s, AVPacket *pkt);


	void (*av_log_set_callback)(void (*)(void*, int, const char*, va_list));
	void (*av_free)(void *ptr);


	int (*avcodec_close)(AVCodecContext *avctx);
	enum PixelFormat (*avcodec_get_pix_fmt)(const char* name);
	const char * (*avcodec_get_pix_fmt_name)(int pix_fmt);
	AVCodec * (*avcodec_find_decoder)(enum CodecID id);
	int (*avcodec_open)(AVCodecContext *avctx, AVCodec *codec);
	AVFrame* (*avcodec_alloc_frame)(void);
	int  (*avcodec_decode_video2)(AVCodecContext *avctx, AVFrame *picture,int *got_picture_ptr, AVPacket *avpkt);
	int (*avcodec_decode_video)(AVCodecContext *avctx, AVFrame *picture,int *got_picture_ptr,const uint8_t *buf, int buf_size);

	int (*sws_scale)(struct SwsContext *context, uint8_t* srcSlice[], int srcStride[], int srcSliceY,int srcSliceH, uint8_t* dst[], int dstStride[]);
	void (*sws_freeContext)(struct SwsContext *swsContext);
	struct SwsContext* (*sws_getContext)(int srcW, int srcH, enum PixelFormat srcFormat, int dstW, int dstH, enum PixelFormat dstFormat, int flags,SwsFilter *srcFilter, SwsFilter *dstFilter, double *param);


	int  (*av_find_stream_info)(struct AVFormatContext *);
	int (*avpicture_fill)(struct AVPicture *,unsigned char *,enum PixelFormat,int,int);
	void (*av_free_packet)(struct AVPacket *);
	AVInputFormat * (*av_find_input_format)(const char *short_name);


	bool initialize(wchar_t* libspath=0)
	{

		
		path_saver_t path_saver(libspath);




		bool fok=1;
		HMODULE hlib;
		hlib=LoadLibraryW(L"avformat-52.dll");

	
		fok=fok&&hlib;
		fok=fok&&(SET_PROC_ADDRESS(hlib,av_close_input_file)); 
        fok=fok&&(SET_PROC_ADDRESS(hlib,av_close_input_stream)); 
		fok=fok&&(SET_PROC_ADDRESS(hlib,av_read_play)); 
		fok=fok&&(SET_PROC_ADDRESS(hlib,av_read_pause)); 

		

		
		fok=fok&&(SET_PROC_ADDRESS(hlib,av_register_all)); 

		
		fok=fok&&(hlib=LoadLibraryW(L"avutil-50.dll"));

		fok=fok&&(SET_PROC_ADDRESS(hlib,av_free)); 
		fok=fok&&(SET_PROC_ADDRESS(hlib,av_log_set_callback)); 


		
		
        fok=fok&&(hlib=LoadLibraryW(L"avformat-52.dll"));

		fok=fok&&(SET_PROC_ADDRESS(hlib,av_open_input_file)); 
		fok=fok&&(SET_PROC_ADDRESS(hlib,av_read_frame)); 
		fok=fok&&(SET_PROC_ADDRESS(hlib,av_find_stream_info)); 
		fok=fok&&(SET_PROC_ADDRESS(hlib,av_find_input_format)); 

		


		
		fok=fok&&(hlib=LoadLibraryW(L"avcodec-52.dll"));

		fok=fok&&(SET_PROC_ADDRESS(hlib,av_dup_packet)); 
        fok=fok&&(SET_PROC_ADDRESS(hlib,avcodec_close)); 
		fok=fok&&(SET_PROC_ADDRESS(hlib,avcodec_open)); 
		fok=fok&&(SET_PROC_ADDRESS(hlib,av_free_packet)); 
		fok=fok&&(SET_PROC_ADDRESS(hlib,avpicture_fill)); 
		

		
		
		
		fok=fok&&(SET_PROC_ADDRESS(hlib,avcodec_get_pix_fmt)); 
		fok=fok&&(SET_PROC_ADDRESS(hlib,avcodec_get_pix_fmt_name)); 

		fok=fok&&(SET_PROC_ADDRESS(hlib,avcodec_find_decoder)); 
		fok=fok&&(SET_PROC_ADDRESS(hlib,avcodec_open)); 
		fok=fok&&(SET_PROC_ADDRESS(hlib,avcodec_alloc_frame)); 
        fok=fok&&(SET_PROC_ADDRESS(hlib,avcodec_decode_video)); 
		fok=fok&&(SET_PROC_ADDRESS(hlib,avcodec_decode_video2)); 


		fok=fok&&(hlib=LoadLibraryW(L"swscale-0.dll"));

		fok=fok&&(SET_PROC_ADDRESS(hlib,sws_scale)); 
		fok=fok&&(SET_PROC_ADDRESS(hlib,sws_freeContext)); 
		fok=fok&&(SET_PROC_ADDRESS(hlib,sws_getContext)); 
		
		





		/*
		int (*sws_scale)(struct SwsContext *context, uint8_t* srcSlice[], int srcStride[], int srcSliceY,int srcSliceH, uint8_t* dst[], int dstStride[]);
		void (*sws_freeContext)(struct SwsContext *swsContext);

		struct SwsContext* (*sws_getContext)

		int (*avcodec_close)(AVCodecContext *avctx);

		enum PixelFormat (*avcodec_get_pix_fmt)(const c
		har* name);
		const char * (*avcodec_get_pix_fmt_name)(int pix_fmt);
		AVCodec * (*avcodec_find_decoder)(enum CodecID id);
		int (*avcodec_open)(AVCodecContext *avctx, AVCodec *codec);
		AVFrame* (*avcodec_alloc_frame)(void);
		int  (*avcodec_decode_video2)(AVCodecContext *avctx, AVFrame *picture,int *got_picture_ptr, AVPacket *avpkt);
		int (*avcodec_decode_video)(AVCodecContext *avctx, AVFrame *picture,int *got_picture_ptr,const uint8_t *buf, int buf_size);

		*/

		return fok;
	}



};