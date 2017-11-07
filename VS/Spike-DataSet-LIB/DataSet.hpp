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
#include <iostream> // for cerr and cout

#include "DataSetState.hpp"

namespace spike
{
	namespace dataset
	{
		template <class B, class D>
		class DataSet
			: public DataSetState<D>
			, private B
		{
		public:
			virtual ~DataSet() = default;
			DataSet() = default;

			DataSet(const DataSet<B, D>& other) = default;
			//		DataSet(DataSet<B, D>&& other) = default;		// move constructor
			DataSet& operator=(const DataSet& other) = default;
			//		DataSet<B, D>& operator=(DataSet<B, D>&& other) = default;		// move assignment operator 


			void saveToFile(const std::string& filename) const
			{
				this->saveToFileBackend(filename, *this);
			}
			void loadFromFile(const std::string& filename)
			{
				this->loadFromFileBackend(filename, *this);
			}
		};
	}
}