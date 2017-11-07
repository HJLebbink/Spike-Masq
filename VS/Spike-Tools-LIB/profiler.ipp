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

#include <type_traits>
#include <iostream>		// for cerr and cout
#include <array>

//#include "src/GccTools.hpp"

namespace tools
{
	namespace profiler
	{
		namespace priv
		{
			inline std::string elapsed_cycles_str(const unsigned long long cycles)
			{
				std::ostringstream os;
				os << cycles << " cycles = "
					<< cycles / 1000 << " kcycles = "
					<< cycles / 1000000 << " mcycles";
				return os.str();
			}
		}

		//WARNING: is NOT thread safe
		template <int SIZE_IN, bool ON_IN>
		struct Profiler
		{
			static const int SIZE = SIZE_IN;
			static const bool ON = ON_IN;

			static std::array<unsigned long long, ON_IN> startTime_;
			static std::array<unsigned long long, ON_IN> totalTime_;

			static inline void reset()
			{}

			template <int I>
			static inline void start()
			{
				static_assert(I < SIZE, "provided I is invalid");
			}

			// stop stopwatch for profiler identified with parameter i, and add the time since starting it to the total time.
			template <int I>
			static inline void stop()
			{
				static_assert(I < SIZE, "provided I is invalid");
			}

			static inline void print_elapsed_time()
			{};
		};

		template <int SIZE_In>
		struct Profiler <SIZE_In, true>
		{
			static const int SIZE = SIZE_In;
			static const bool ON = true;

			using ThisClass = Profiler <SIZE, true>;

			static std::array<unsigned long long, SIZE_In> startTime_;
			static std::array<unsigned long long, SIZE_In> totalTime_;

			// constructor
			Profiler()
			{
				ThisClass::totalTime_.fill(0);
			}

			static void reset()
			{
				ThisClass::totalTime_.fill(0);
			}

			// start stopwatch for profiler identified with parameter i
			template <int I>
			static void start()
			{
				static_assert(I < SIZE, "provided I is invalid");
				//::tools::assert::static_assert_msg(I < SIZE, "provided i is invalid");
				ThisClass::startTime_[I] = rdtsc();
			}

			// stop stopwatch for profiler identified with parameter i, and add the time since starting it to the total time.
			template <int I>
			static void stop()
			{
				static_assert(I < SIZE, "provided I is invalid");
				ThisClass::totalTime_[I] += (rdtsc() - ThisClass::startTime_[I]);
			}
			
			static void print_elapsed_cycles()
			{
				unsigned long long sum = 0;
				for (int i = 0; i < SIZE; ++i)
				{
					sum += ThisClass::totalTime_[i];
				}

				for (int i = 0; i < SIZE; ++i)
				{
					const double timeInMs = static_cast<double>(ThisClass::totalTime_[i]) / (1024. * 1024.);
					const double percent = 100 * (static_cast<double>(ThisClass::totalTime_[i]) / sum);
					printf("\tprofiler::elapsed cycles: i=%2zu: percent %5.2f; %s\n", i, percent, priv::elapsed_cycles_str(ThisClass::totalTime_[i]).c_str());
				}
			}
		};

		template <int SIZE>
		std::array<unsigned long long, SIZE> Profiler <SIZE, true>::startTime_;

		template <int SIZE>
		std::array<unsigned long long, SIZE> Profiler <SIZE, true>::totalTime_;
	}
}