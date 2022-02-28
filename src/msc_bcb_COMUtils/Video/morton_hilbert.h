#pragma once

struct Morton_Hilbert2D_base_t{
	typedef unsigned int uint;	
	inline static	uint MortonToHilbert( const uint morton, const uint bits )
	{
		uint hilbert = 0;
		uint remap = 0xb4;
		uint block = ( bits << 1 );
		while( block )
		{
			block -= 2;
			uint mcode = ( ( morton >> block ) & 3 );
			uint hcode = ( ( remap >> ( mcode << 1 ) ) & 3 );
			remap ^= ( 0x82000028 >> ( hcode << 3 ) );
			hilbert = ( ( hilbert << 2 ) + hcode );
		}
		return( hilbert );
	}
	inline static	uint HilbertToMorton( const uint hilbert, const uint bits )
	{
		uint morton = 0;
		uint remap = 0xb4;
		uint block = ( bits << 1 );
		while( block )
		{
			block -= 2;
			uint hcode = ( ( hilbert >> block ) & 3 );
			uint mcode = ( ( remap >> ( hcode << 1 ) ) & 3 );
			remap ^= ( 0x330000cc >> ( hcode << 3 ) );
			morton = ( ( morton << 2 ) + mcode );
		}
		return( morton );
	}

};

template <int Bits=5>
struct Morton_Hilbert2D_t:Morton_Hilbert2D_base_t{

};


template <>
struct Morton_Hilbert2D_t<5>:Morton_Hilbert2D_base_t{

	static uint encode( uint index1, uint index2 )
	{ // pack 2 5-bit indices into a 10-bit Morton code
		index1 &= 0x0000001f;
		index2 &= 0x0000001f;
		index1 *= 0x01041041;
		index2 *= 0x01041041;
		index1 &= 0x10204081;
		index2 &= 0x10204081;
		index1 *= 0x00108421;
		index2 *= 0x00108421;
		index1 &= 0x15500000;
		index2 &= 0x15500000;
		return( ( index1 >> 20 ) | ( index2 >> 19 ) );
	}
	static void decode( const uint morton, uint& index1, uint& index2 )
	{ // unpack 2 5-bit indices from a 10-bit Morton code
		uint value1 = morton;
		uint value2 = ( value1 >> 1 );
		value1 &= 0x00000155;
		value2 &= 0x00000155;
		value1 |= ( value1 >> 1 );
		value2 |= ( value2 >> 1 );
		value1 &= 0x00000133;
		value2 &= 0x00000133;
		value1 |= ( value1 >> 2 );
		value2 |= ( value2 >> 2 );
		value1 &= 0x0000010f;
		value2 &= 0x0000010f;
		value1 |= ( value1 >> 4 );
		value2 |= ( value2 >> 4 );
		value1 &= 0x0000001f;
		value2 &= 0x0000001f;
		index1 = value1;
		index2 = value2;
	}
};


