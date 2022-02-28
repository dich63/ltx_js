#pragma once


#define FOURCC(a,b,c,d) (unsigned int)((((unsigned int)a))+(((unsigned int)b)<<8)+(((unsigned int)c)<<16)+(((unsigned int)d)<<24))
struct fourcc_ffmpeg_t
{

	struct PixelFormatTag {
		PixelFormat pix_fmt;
		unsigned int fourcc;
	} ;

inline static const PixelFormatTag* get_PixelFormatTag()
{
	static const PixelFormatTag pixelFormatTags[] = {
		/* Planar formats */
		{ PIX_FMT_YUV420P, 	FOURCC('I', '4', '2', '0') },
		{ PIX_FMT_YUV420P, 	FOURCC('I', 'Y', 'U', 'V') },
		{ PIX_FMT_YUV420P, 	FOURCC('Y', 'V', '1', '2') },
		{ PIX_FMT_YUV410P, 	FOURCC('Y', 'U', 'V', '9') },
		{ PIX_FMT_YUV411P, 	FOURCC('Y', '4', '1', 'B') },
		{ PIX_FMT_YUV422P, 	FOURCC('Y', '4', '2', 'B') },
		{ PIX_FMT_GRAY8,   	FOURCC('Y', '8', '0', '0') },
		{ PIX_FMT_GRAY8,   	FOURCC(' ', ' ', 'Y', '8') },

		//v4l
		{ PIX_FMT_GRAY8,   	FOURCC('G', 'R', 'E', 'Y') },
		//{ PIX_FMT_... 	FOURCC('H', '2', '4', '0') },	//HI420 BT848, seems to not exist in ffmpeg
		{ PIX_FMT_RGB565,  	FOURCC('R', 'G', 'B', '6') },	//RGB565
		{ PIX_FMT_RGB555 , 	FOURCC('R', 'G', 'B', '5') },	//RGB15
		{ PIX_FMT_RGB32,   	FOURCC('R', 'G', 'B', '4') },	//RGB32
		{ PIX_FMT_YUYV422, 	FOURCC('Y', 'U', 'Y', 'V') },
		{ PIX_FMT_UYYVYY411,FOURCC('Y', '4', '1', '1') },	//YUV411 is this correct??
		//FOURCC( 'R', 'A', 'W', ' ' );						//BT848, seems to not exist in ffmpeg
		{ PIX_FMT_YUV422P, 	FOURCC('Y', '4', '2', 'P') },	//YUV422P
		{ PIX_FMT_YUV411P, 	FOURCC('4', '1', '1', 'P') },	//YUV411P
		{ PIX_FMT_YUV420P, 	FOURCC('Y', 'U', '1', '2') },

		//v4l2
		{ PIX_FMT_UYVY422, 	FOURCC('U', 'Y', 'V', 'Y') },
		{ PIX_FMT_YUV411P, 	FOURCC('Y', '4', '1', 'P') },
		//{ PIX_FMT_YVU410P, FOURCC('Y', 'V', 'U', '9') },
		{ PIX_FMT_YUV410P, 	FOURCC('Y', 'U', 'V', '9') }, 	// is this correct or YUV410 non-planar
		{ PIX_FMT_YUV422P, 	FOURCC('4', '2', '2', 'P') },
		{ PIX_FMT_YUV411P, 	FOURCC('4', '1', '1', 'P') },
		{ PIX_FMT_NV12,    	FOURCC('N', 'V', '1', '2') },
		{ PIX_FMT_NV21,    	FOURCC('N', 'V', '2', '1') },
		{  PIX_FMT_NONE,  	FOURCC('Y', 'Y', 'U', 'V') },
		{ PIX_FMT_RGB32_1, 	FOURCC('R', 'G', 'B', '1') },	// is this correct?
		{ PIX_FMT_RGB555,  	FOURCC('R', 'G', 'B', 'O') },
		{ PIX_FMT_RGB565,  	FOURCC('R', 'G', 'B', 'P') },
		//{ PIX_FMT_RGB555X,  FOURCC('R', 'G', 'B', 'Q') },
		//{ PIX_FMT_RGB565X,  FOURCC('R', 'G', 'B', 'R') },
		{ PIX_FMT_BGR32,   	FOURCC('B', 'G', 'R', '4') },

		//vid21394
		{ PIX_FMT_YUYV422, 	FOURCC('Y', 'U', 'Y', '2') },	// is this correct?
		{ PIX_FMT_GRAY8,   	FOURCC('Y', '8', '0', '0') },	// is this correct?

		/* dcam */
		{ PIX_FMT_YUV444P, 	FOURCC('Y', '4', '4', '4') }, 	// is this correct or non-planar?
		{ PIX_FMT_GRAY16,  	FOURCC('Y', '1', '6', '0') }, 	// is this correct?
		{ PIX_FMT_RGB24,   	FOURCC('R', 'G', 'B', ' ') },

		/* Packed formats */
		{ PIX_FMT_YUYV422, 	FOURCC('Y', 'U', 'Y', '2') },
		{ PIX_FMT_YUYV422, 	FOURCC('Y', '4', '2', '2') },
		{ PIX_FMT_UYVY422, 	FOURCC('U', 'Y', 'V', 'Y') },
		{ PIX_FMT_GRAY8,   	FOURCC('G', 'R', 'E', 'Y') },
		{ PIX_FMT_RGB555,  	FOURCC('R', 'G', 'B', 15) },
		{ PIX_FMT_BGR555,  	FOURCC('B', 'G', 'R', 15) },
		{ PIX_FMT_RGB565,  	FOURCC('R', 'G', 'B', 16) },
		{ PIX_FMT_BGR565,  	FOURCC('B', 'G', 'R', 16) },
		{ PIX_FMT_BGR24,   	FOURCC('B', 'G', 'R', '3') },
		{ PIX_FMT_RGB24,   	FOURCC('R', 'G', 'B', '3') },

		/* quicktime */
		{ PIX_FMT_UYVY422, 	FOURCC('2', 'v', 'u', 'y') },
		{ PIX_FMT_UYVY422, 	FOURCC('A', 'V', 'U', 'I') },

		{ PIX_FMT_NONE, 0 },
	};
	return pixelFormatTags;
}

inline static PixelFormat fourcc_to_pix_fmt( unsigned int fourcc)
{
	const PixelFormatTag * tags = get_PixelFormatTag();
	while (tags->pix_fmt >= 0) {
		if (tags->fourcc == fourcc)
			return tags->pix_fmt;
		tags++;
	}
	return PIX_FMT_NONE;
}

inline static unsigned int pix_fmt_to_fourcc(PixelFormat  pix_fmt)
{
	const PixelFormatTag * tags = get_PixelFormatTag();
	while (tags->pix_fmt >= 0) {
		if (tags->pix_fmt == pix_fmt)
			return tags->fourcc;
		tags++;
	}
	return 0;
}

};