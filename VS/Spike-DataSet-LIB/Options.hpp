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

#include <algorithm>	// std::min
#include <limits>		// std::numeric_limits<>::max()
#include <iostream>		// std::cerr

namespace spike
{
	namespace dataset
	{
		template <class D>
		class Options
		{
		public:
			virtual ~Options() = default;

			Options()
				: missingValuePresent_(false)
			{
			}

			void setMissingValue(const D value)
			{
				this->missingValue_ = value;
				this->missingValuePresent_ = true;
			}

			D getMissingValue() const
			{
				if (this->missingValuePresent_)
				{
					return this->missingValue_;
				}
				//	throw std::runtime_error("Options::getMissingValue: missing values is not set");
				const long long newMissingValue = std::min(static_cast<long long>(std::numeric_limits<D>::max()), static_cast<long long>(99999));
				std::cerr << "Options::getMissingValue: missing values is not set, assuming " << newMissingValue << std::endl;
				return static_cast<D>(newMissingValue);
			}

		private:

			bool missingValuePresent_;
			D missingValue_;

		};
	}
}