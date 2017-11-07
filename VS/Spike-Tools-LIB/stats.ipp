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

#include <vector>
#include <iterator>		// std::begin
#include <algorithm>	// std::for_each
#include <numeric>      // std::accumulate
#include <cmath> 		// std::sqrt
#include <iostream>     // std::cout

namespace tools
{
	namespace stats
	{
		inline double binomial(const size_t n, const size_t k)
		{
			if (k > n)
			{
				return 0;
			}
			if (n == k)
			{
				return 1;
			}
			else
			{
				double result = 1;
				for (size_t i = 1; i <= k; ++i)
				{
					const double d = static_cast<double>(n + 1 - i) / i;
					result *= d;
					//				if (result == std::numeric_limits<double>::infinity()) {
					//					return result;
					//				}
					//std::cout << "i=" << i << "; (n + 1 - i)=" << (n + 1 - i) << "; ((n + 1 - i) / i)=" << d << "; result = " << result << std::endl;
				}
				return result;
			}
		}

		inline double multinomial(const size_t n0, const size_t n1, const size_t n2)
		{
			const double l1 = binomial(n0 + n1, n1);
			//if (l1 == std::numeric_limits<double>::infinity()) {
			//	return l1;
			//}
			const double l2 = binomial(n0 + n1 + n2, n2);
			//if (l2 == std::numeric_limits<double>::infinity()) {
			//	return l2;
			//}
			return l1 * l2;
		}

		template <typename T>
		inline T mean(
			const T * const v,
			const size_t length)
		{
			T sum = 0.0;
			for (size_t i = 0; i < length; ++i)
			{
				sum += v[i];
			}
			return static_cast<T>(sum / length);
		}

		template <typename T>
		inline T mean(const std::vector<T>& v)
		{
			return mean(v.data(), v.size());
			//const T sum = std::accumulate(std::begin(v), std::end(v), 0.0);
			//return sum / v.size();
		}

		template <typename T>
		inline T variance(
			const T * const v,
			const size_t length)
		{
			if (length < 2)
			{
				return 0;
			}

			const T m = mean(v, length);

			T accum = 0.0;
			for (size_t i = 0; i < length; ++i)
			{
				accum += (v[i] - m) * (v[i] - m);
			}

			return accum / (length - 1);
		}

		template <typename T>
		inline T variance(
			const T * const v,
			const size_t length,
			const T mean)
		{
			if (length < 2)
			{
				return 0;
			}

			T accum = 0.0;
			for (size_t i = 0; i < length; ++i)
			{
				accum += (v[i] - mean) * (v[i] - mean);
			}

			return accum / (length - 1);
		}


		template <typename T>
		inline T stdev(
			const T * const v,
			const size_t length)
		{
			return std::sqrt(variance(v, length));
		}

		template <typename T>
		inline T stdev(
			const std::vector<T>& v)
		{
			return stdev(v.data(), v.size());
		}

		namespace test
		{
			void testMultinomial()
			{
				printf("Running testMultinomial code\n");
				for (unsigned int n = 150; n < 180; ++n)
				{
					for (unsigned int k = 150; k <= n; ++k)
					{
						std::cout << "(" << n << ", " << k << ")! = " << ::tools::stats::binomial(n, k) << std::endl;
					}
				}
				std::cout << "(" << 2 << ", " << 2 << ", " << 2 << ")! = " << ::tools::stats::multinomial(40, 40, 40) << std::endl;
				std::cout << "(" << 2 << ", " << 2 << ", " << 0 << ")! = " << ::tools::stats::multinomial(2, 2, 0) << std::endl;
			}
		}
	}
}