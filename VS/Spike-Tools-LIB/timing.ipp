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

#ifdef _MSC_VER		// compiler: Microsoft Visual Studio
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <x86intrin.h>
#endif
#define rdtsc __rdtsc

#include <string>
#include <chrono>
#include <sstream>


namespace tools
{
	namespace timing
	{
		namespace priv
		{
			unsigned long long timing_start;
			unsigned long long timing_end;
		}

		inline void reset_and_start_timer() {
			priv::timing_start = rdtsc();
		}

		// Returns the number of millions of elapsed processor cycles since the last reset_and_start_timer() call.
		inline double get_elapsed_mcycles() {
			priv::timing_end = rdtsc();
			return static_cast<double>(priv::timing_end - priv::timing_start) / (1024. * 1024.);
		}

		// Returns the number of thousands of elapsed processor cycles since the last reset_and_start_timer() call.
		inline double get_elapsed_kcycles()
		{
			priv::timing_end = rdtsc();
			return static_cast<double>(priv::timing_end - priv::timing_start) / (1024.);
		}

		// Returns the number of elapsed processor cycles since the last reset_and_start_timer() call.
		inline unsigned long long get_elapsed_cycles()
		{
			priv::timing_end = rdtsc();
			return (priv::timing_end - priv::timing_start);
		}

		inline std::string elapsed_cycles_str(const unsigned long long start, const unsigned long long end)
		{
			std::ostringstream os;
			const auto diff = end - start;
			os << diff << " cycles = "
				<< diff / 1000 << " kcycles = "
				<< diff / 1000000 << " mcycles";
			return os.str();
		}

		inline std::string elapsed_time_str(const std::chrono::system_clock::time_point start, const std::chrono::system_clock::time_point end)
		{
			std::ostringstream os;
			const auto diff = end - start;
			os << std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() << " ms = "
				<< std::chrono::duration_cast<std::chrono::seconds>(diff).count() << " sec = "
				<< std::chrono::duration_cast<std::chrono::minutes>(diff).count() << " min = "
				<< std::chrono::duration_cast<std::chrono::hours>(diff).count() << " hours" << std::endl;
			return os.str();
		}
	}
}