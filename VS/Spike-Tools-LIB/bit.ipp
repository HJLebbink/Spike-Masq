// The MIT License (MIT)
//
// Copyright (c) 2017 Henk-Jan Lebbink
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <string>
#include <vector>
#include <array>
#include <bitset>

#include <emmintrin.h>
#include <intrin.h>


#include "assert.ipp"

namespace tools
{
	namespace bit
	{
		// forward declaration to such that they can be used in local namespace
		inline bool getBit(const void * const d, const size_t pos);
		inline void setBit(void * const d, const size_t pos);
		inline void clearBit(void * const d, const size_t pos);

		namespace local
		{
			inline void swapBit_method1(void * const d, const size_t pos1, const size_t pos2)
			{
				const bool b1 = ::tools::bit::getBit(d, pos1);
				const bool b2 = ::tools::bit::getBit(d, pos2);
				if (b1 != b2)
				{
					if (b1)
					{
						tools::bit::setBit(d, pos2);
						tools::bit::clearBit(d, pos1);
					}
					else
					{
						tools::bit::clearBit(d, pos2);
						tools::bit::setBit(d, pos1);
					}
				}
			}

			inline void swapBit_method2(unsigned int * const ptr, const size_t pos1, const size_t pos2)
			{
				#ifdef _MSC_VER
				long * const dataPtr = reinterpret_cast<long *>(ptr);

				const long pos1x = static_cast<long>(pos1 & 0x1F);
				const long pos2x = static_cast<long>(pos2 & 0x1F);

				const unsigned char b1 = _bittestandreset(&dataPtr[pos1 >> 5], pos1x);
				const unsigned char b2 = (b1 > 0) ? _bittestandset(&dataPtr[pos2 >> 5], pos2x) : _bittestandreset(&dataPtr[pos2 >> 5], pos2x);
				if (b2 > 0) _bittestandset(&dataPtr[pos1 >> 5], pos1x);
				#else 
				printf("tools::perm::swapBit_method2 : not implemented yet\n");
				//TODO
				//DEBUG_BREAK();
				#endif
			}

			inline void swapBit_method3(unsigned int * const d, const size_t pos1, const size_t pos2)
			{
				//bitswap_gas(d, pos1, pos2);
			}
		}

		/******************************************
		* bitwise and
		*******************************************/
		inline unsigned long long bitwise_and(unsigned long long a, unsigned long long b)
		{
			return a & b;
		}
		inline __m128i bitwise_and(__m128i a, __m128i b)
		{
			return _mm_and_si128(a, b);
		}

		/******************************************
		* bitwise or
		*******************************************/
		inline unsigned long long bitwise_or(unsigned long long a, unsigned long long b)
		{
			return a | b;
		}
		inline __m128i bitwise_or(__m128i a, __m128i b)
		{
			return _mm_or_si128(a, b);
		}

		/******************************************
		* static optional negation
		*******************************************/
		template <bool optional>
		inline static unsigned long long optional_neg(const unsigned long long d)
		{
			return (optional) ? ~d : d;
		}
		template <bool optional>
		inline static __m128i optional_neg(__m128i d)
		{
			static const __m128i mask = _mm_cmpeq_epi32(_mm_setzero_si128(), _mm_setzero_si128()); // compare something with itself will result in 0xFFFFFFFF
			return (optional) ? _mm_andnot_si128(d, mask) : d;
		}


		/******************************************
		 * get Bits
		 *******************************************/
		constexpr inline bool getBitNeg_constexpr(const unsigned long long i, const size_t pos)
		{
			//return !((i & (1ull << pos)) != 0);
			return !((i >> pos) & 1);
		}


		inline bool getBit(const unsigned char i, const size_t pos)
		{
			tools::assert::assert_msg(pos < 8, "pos ", pos, " is too large");
			return (i & (1 << pos)) != 0;
		}
		inline bool getBit(const unsigned short i, const size_t pos)
		{
			tools::assert::assert_msg(pos < 16, "pos ", pos, " is too large");
			return (i & (1 << pos)) != 0;
		}
		inline bool getBit(const unsigned int i, const size_t pos)
		{
			tools::assert::assert_msg(pos < 32, "pos ", pos, " is too large");
			return (i & (1u << pos)) != 0;
		}
		inline bool getBit(const unsigned long long i, const size_t pos)
		{
			tools::assert::assert_msg(pos < 64, "pos ", pos, " is too large");
			return (i & (1ull << pos)) != 0;
			//return _bittest64(&i, pos) != 0;
		}
		inline bool getBit(const long long i, const size_t pos)
		{
			tools::assert::assert_msg(pos < 64, "pos ", pos, " is too large");
			return (i & (1ull << pos)) != 0;
			//return _bittest64(&i, pos) != 0;
		}

		inline bool getBit(const void * const d, const size_t pos)
		{
			return ::tools::bit::getBit(reinterpret_cast<const unsigned char * const>(d)[pos >> 3], pos & 0x7);
		}
		/*
		inline bool getBit(const unsigned int * const d, const size_t pos)
		{
			return ::tools::bit::getBit(d[pos >> 5], pos & 0x1F);
		}
		inline bool getBit(const unsigned long long * const d, const size_t pos)
		{
			return ::tools::bit::getBit(d[pos >> 6], pos & 0x3F);
		}

		*/
		inline bool getBit(const std::vector<__m128i>& i, const size_t pos)
		{
			::tools::assert::assert_msg(pos < (128 * i.size()), "pos ", pos, " is too large");
			const unsigned int * const ptr = reinterpret_cast<const unsigned int * const>(i.data());
			return ::tools::bit::getBit(ptr, pos);
		}
		template <size_t S>
		inline bool getBit(const std::array<__m128i, S>& i, const size_t pos)
		{
			::tools::assert::assert_msg(pos < (128 * i.size()), "pos ", pos, " is too large");
			const unsigned int * const ptr = reinterpret_cast<const unsigned int * const>(i.data());
			return ::tools::bit::getBit(ptr, pos);
		}
		inline bool getBit(const __m128i i, const size_t pos)
		{
			::tools::assert::assert_msg(pos < 128, "pos ", pos, " is too large");
			std::array<__m128i, 1> a = { {i} };
			return ::tools::bit::getBit(a, pos);
		}


		/******************************************
		* set Bits
		*******************************************/
		inline void setBit(unsigned char& i, const size_t pos)
		{
			::tools::assert::assert_msg(pos < 8, "pos ", pos, " is too large");
			i |= (1 << pos);
		}
		inline void setBit(unsigned short& i, const size_t pos)
		{
			::tools::assert::assert_msg(pos < 16, "pos ", pos, " is too large");
			i |= (1 << pos);
		}
		inline void setBit(unsigned int& i, const size_t pos)
		{
			::tools::assert::assert_msg(pos < 32, "pos ", pos, " is too large");
			i |= (1ul << pos);
		}
		inline void setBit(unsigned long long& i, const size_t pos)
		{
			::tools::assert::assert_msg(pos < 64, "pos ", pos, " is too large");
			i |= (1ull << pos);
		}
		inline void setBit(long long& i, const size_t pos)
		{
			::tools::assert::assert_msg(pos < 64, "pos ", pos, " is too large");
			i |= (1ull << pos);
			//_bittestandset64(&i, pos);
		}
		inline void setBit(void * const d, const size_t pos)
		{
			::tools::bit::setBit(reinterpret_cast<unsigned char * const>(d)[pos >> 3], pos & 0x7);
		}
		/*
		inline void setBit(unsigned int * const d, const size_t pos)
		{
			tools::bit::setBit(d[pos >> 5], pos & 0x1F);
		}
		inline void setBit(unsigned long long * const d, const size_t pos)
		{
			tools::bit::setBit(d[pos >> 6], pos & 0x3F);
		}
		*/
		inline void setBit(std::vector<__m128i>& i, const size_t pos)
		{
			::tools::assert::assert_msg(pos < (128 * i.size()), "pos ", pos, " is too large");
			unsigned long long * const ptr = reinterpret_cast<unsigned long long * const>(i.data());
			::tools::bit::setBit(ptr, pos);
		}
		template <size_t S>
		inline void setBit(std::array<__m128i, S>& i, const size_t pos)
		{
			::tools::assert::assert_msg(pos < (128 * i.size()), "pos ", pos, " is too large");
			unsigned long long * const ptr = reinterpret_cast<unsigned long long * const>(i.data());
			::tools::bit::setBit(ptr, pos);
		}
		inline void setBit(__m128i& i, const size_t pos)
		{
			::tools::assert::assert_msg(pos < 128, "pos ", pos, " is too large");
			unsigned long long * const ptr = reinterpret_cast<unsigned long long * const>(&i);
			::tools::bit::setBit(ptr, pos);
		}


		/******************************************
		* clear Bits
		*******************************************/
		inline void clearBit(unsigned char& i, const size_t pos)
		{
			::tools::assert::assert_msg(pos < 8, "pos ", pos, " is too large");
			i &= ~(1 << pos);
		}

		inline void clearBit(unsigned short& i, const size_t pos)
		{
			::tools::assert::assert_msg(pos < 16, "pos ", pos, " is too large");
			i &= ~(1 << pos);
		}
		inline void clearBit(unsigned int& i, const size_t pos)
		{
			::tools::assert::assert_msg(pos < 32, "pos ", pos , " is too large");
			i &= ~(1u << pos);
		}
		inline void clearBit(unsigned long long& i, const size_t pos)
		{
			::tools::assert::assert_msg(pos < 64, "pos ", pos, " is too large");
			i &= ~(1ull << pos);
			//_bittestandreset64(&i, pos);
		}
		inline void clearBit(long long& i, const size_t pos)
		{
			::tools::assert::assert_msg(pos < 64, "pos ", pos, " is too large");
			i &= ~(1ull << pos);
			//_bittestandreset64(&i, pos);
		}
		inline void clearBit(void * const d, const size_t pos)
		{
			::tools::bit::clearBit(reinterpret_cast<unsigned char * const>(d)[pos >> 3], pos & 0x7);
		}
		/*
		inline void clearBit(unsigned int * const d, const size_t pos)
		{
			tools::bit::clearBit(d[pos >> 5], pos & 0x1F);
		}
		inline void clearBit(unsigned long long * const d, const size_t pos)
		{
			tools::bit::clearBit(d[pos >> 6], pos & 0x3F);
		}
		*/
		inline void clearBit(std::vector<__m128i>& i, const size_t pos)
		{
			::tools::assert::assert_msg(pos < (128 * i.size()), "pos ", pos, " is too large");
			unsigned long long * const ptr = reinterpret_cast<unsigned long long * const>(i.data());
			::tools::bit::clearBit(ptr, pos);
		}
		template <size_t S>
		inline void clearBit(std::array<__m128i, S>& i, const size_t pos)
		{
			::tools::assert::assert_msg(pos < (128 * i.size()), "pos ", pos, " is too large");
			unsigned long long * const ptr = reinterpret_cast<unsigned long long * const>(i.data());
			::tools::bit::clearBit(ptr, pos);
		}
		inline void clearBit(__m128i& i, const size_t pos)
		{
			::tools::assert::assert_msg(pos < 128, "pos ", pos, " is too large");
			unsigned long long * const ptr = reinterpret_cast<unsigned long long * const>(&i);
			::tools::bit::clearBit(ptr, pos);
		}


		/******************************************
		* get and clear bit
		*******************************************/
		inline bool getAndClearBit(long long& i, const size_t pos)
		{
			::tools::assert::assert_msg(pos < 64, "pos ", pos, " is too large");
			#ifdef _MSC_VER
			//const bool b = (i & (1ull << pos)) != 0;
			//i &= ~(1ull << pos);
			//return b;
			return (_bittestandreset64(&i, static_cast<__int64>(pos)) != 0);
			#else
			//	__asm__("int3");
			//	__asm__("bts %1,%0" : "+m" (i) : "r" (pos));
			const long long mask = 1ull << pos;
			const bool returnValue = (i & mask) != 0;
			i &= ~mask;
			return returnValue;
			#endif
		}
		inline bool getAndClearBit(long long * i, const size_t pos)
		{
			::tools::assert::assert_msg(pos < 64, "pos ", pos, " is too large");
			#ifdef _MSC_VER
			return (_bittestandreset64(i, static_cast<__int64>(pos)) != 0);
			#else
			//DEBUG_BREAK();
			return false;
			//const long long mask = 1ull << pos;
			//const bool returnValue = (i & mask) != 0;
			//i &= ~mask;
			//return returnValue;
			#endif
		}

		inline bool getAndClearBit(unsigned long long& d2, const size_t pos)
		{
			long long& d3 = reinterpret_cast<long long&>(d2);
			return ::tools::bit::getAndClearBit(d3, pos);
		}
		inline bool getAndClearBit(unsigned long long * const d, const size_t pos)
		{
			return ::tools::bit::getAndClearBit(d[pos >> 6], pos & 0x3F);
		}


		/******************************************
		* get and set bit
		*******************************************/
		inline bool getAndSetBit(long long& i, const size_t pos)
		{
			::tools::assert::assert_msg(pos < 64, "pos ", pos, " is too large");
			#ifdef _MSC_VER
			return (_bittestandset64(&i, static_cast<__int64>(pos)) != 0);
			#else
			const long long mask = 1ull << pos;
			const bool returnValue = (i & mask) != 0;
			i |= mask;
			return returnValue;
			#endif
		}
		inline bool getAndSetBit(unsigned long long& d2, const size_t pos)
		{
			long long& d3 = reinterpret_cast<long long&>(d2);
			return ::tools::bit::getAndSetBit(d3, pos);
		}
		inline bool getAndSetBit(unsigned long long * const d, const size_t pos)
		{
			return ::tools::bit::getAndSetBit(d[pos >> 6], pos & 0x3F);
		}


		/******************************************
		* swap bits
		*******************************************/
		inline void swapBit(void * const d, const size_t nBytes, const size_t pos1, const size_t pos2)
		{
			::tools::assert::assert_msg(pos1 < (nBytes * 8), "pos1 ", pos1, " is too large");
			::tools::assert::assert_msg(pos2 < (nBytes * 8), "pos2 ", pos2, " is too large");

			#if _DEBUG
			char * ptr2 = static_cast<char *>(d);
			std::vector<char> copy1(ptr2, ptr2 + nBytes);
			tools::bit::local::swapBit_method1(copy1.data(), pos1, pos2);
			#endif
			::tools::bit::local::swapBit_method1(d, pos1, pos2);
			//::tools::bit::local::swapBit_method2(d, pos1, pos2);
			//::tools::bit::local::swapBit_method3(d, pos1, pos2);

			#if _DEBUG
			for (size_t i = 0; i < nBytes; ++i)
			{
				if (copy1[i] != ptr2[i])
				{
					std::cout << "tools::bit::swapBit: pos1=" << pos1 << "; pos2=" << pos2 << "; copy[" << i << "] = " << copy1[i] << "; d[" << i << "] = " << static_cast<int>(ptr2[i]) << std::endl;
					//DEBUG_BREAK();
				}
			}
			#endif
		}

		inline void swapBit(std::vector<__m128i>& d, const size_t pos1, const size_t pos2)
		{
			const size_t nBytes = d.size() * sizeof(__m128i);
			::tools::bit::swapBit(d.data(), nBytes, pos1, pos2);
		}
		template <size_t S>
		inline void swapBit(std::array<__m128i, S>& d, const size_t pos1, const size_t pos2)
		{
			const size_t nBytes = S * sizeof(__m128i);
			::tools::bit::swapBit(d.data(), nBytes, pos1, pos2);
		}

		inline void get_lexi_next_bit_perm(const unsigned int v, unsigned int& w)
		{
			// http://graphics.stanford.edu/~seander/bithacks.html#NextBitPermutation
			// Suppose we have a pattern of N bits set to 1 in an integer and we want the next permutation of N 1 bits 
			// in a lexicographical sense. For example, if N is 3 and the bit pattern is 00010011, the next patterns 
			// would be 00010101, 00010110, 00011001,00011010, 00011100, 00100011, and so forth. The following is a fast 
			// way to compute the next permutation.

			//unsigned int v; // current permutation of bits 
			//unsigned int w; // next permutation of bits
			unsigned long index;
			const unsigned int t = v | (v - 1); // t gets v's least significant 0 bits set to 1
										  // Next set to 1 the most significant bit to change, 
										  // set to 0 the least significant ones, and add the necessary 1 bits.
			_BitScanForward(&index, v);
			#			pragma warning( push )
			#			pragma warning( disable : 4146)
			w = (t + 1) | (((~t & -~t) - 1) >> (index + 1));
			#			pragma warning( pop ) 
		}

		inline void test_get_lexi_next_bit_perm()
		{
			unsigned int v = 7;
			unsigned int w;
			for (int i = 0; i < 1000; i++)
			{
				get_lexi_next_bit_perm(v, w);
				std::bitset<32> x(w);
				std::cout << i << ":" << x.to_string() << std::endl;
				v = w;
			}
		}
	}
}