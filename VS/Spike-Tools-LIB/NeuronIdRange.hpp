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

#include <type_traits> // for std::is_integral

#include "../../Spike-Tools-LIB/SpikeTypes.hpp"

namespace spike
{
	namespace tools
	{
		template <typename T>
		class LoopRangeIterator
		{
		public:
			LoopRangeIterator(T value)
				: value_(value)
			{
				static_assert(std::is_integral<T>::value, "LoopRangeIterator: range only accepts integral values");
			}

			bool operator!=(const LoopRangeIterator& other) const
			{
				return this->value_ != other.value_;
			}

			const T& operator*() const
			{
				return this->value_;
			}

			void operator++()
			{
				++this->value_;
			}

		private:
			T value_;
		};

		template<typename T, T FROM, T TO>
		class NeuronIdRange
		{
		public:

			LoopRangeIterator<T> begin() const
			{
				return LoopRangeIterator<T>(FROM);
			}

			const LoopRangeIterator<T> end() const
			{
				return LoopRangeIterator<T>(TO);
			}
		};
	}
}