#pragma once
#ifdef _WIN32
# define _CRT_SECURE_NO_WARNINGS
typedef __int8 int8_t;
typedef __int16 int16_t;
typedef __int32 int32_t;
typedef __int64 int64_t;
typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#else
# include <stdint.h>
#endif
 
#ifndef BI_RGB
# define BI_RGB 0
#endif
#ifndef BI_BITFIELDS
# define BI_BITFIELDS 3
#endif
 
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

 
// Setting struct alignment to 1 byte, you might have to use something else on linux to do this...
// Newer gcc version support pragma pack. see: http://gcc.gnu.org/onlinedocs/gcc/Structure_002dPacking-Pragmas.html
#pragma pack(push, 1)
typedef struct tagBITMAPFILEHEADER {
	uint16_t	bfType;
	uint32_t	bfSize;
	uint16_t	bfReserved1;
	uint16_t	bfReserved2;
	uint32_t	bfOffBits;
} BITMAPFILEHEADER;
 
typedef struct tagBITMAPINFOHEADER{
	uint32_t	biSize;
	int32_t		biWidth;
	int32_t		biHeight;
	uint16_t	biPlanes;
	uint16_t	biBitCount;
	uint32_t	biCompression;
	uint32_t	biSizeImage;
	int32_t		biXPelsPerMeter;
	int32_t		biYPelsPerMeter;
	uint32_t	biClrUsed;
	uint32_t	biClrImportant;
} BITMAPINFOHEADER;
#pragma pack(pop)
 
