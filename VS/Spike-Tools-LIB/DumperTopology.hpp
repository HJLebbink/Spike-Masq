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
#include <memory> // for make_unique, unique_ptr

#include "SpikeRuntimeOptions.hpp"

namespace spike {
	namespace tools {

		template <typename Topology_i>
		class DumperTopology
		{
		public:

			using Topology = Topology_i;

			DumperTopology() = default;
			~DumperTopology() = default;

			DumperTopology(const SpikeRuntimeOptions& options)
				: options_(options)
			{}

			DumperTopology& operator=(const DumperTopology &d) = delete;

			bool dumpTest(const unsigned int sec) const
			{
				return (this->options_.isDumpToFileOn_Topology() && ((sec % this->options_.getDumpIntervalInSec_Topology()) == 0));
			}

			void dump(
				const unsigned int sec,
				const std::string& nameSuffix,
				const std::shared_ptr<const Topology>& topology) const
			{
				// create the filename
				std::stringstream filenameStream;
				if (nameSuffix.empty() || nameSuffix.length() == 0) {
					filenameStream << this->options_.getFilenamePath_Topology() << "/" << this->options_.getFilenamePrefix_Topology() << "." << sec << ".txt";
				} else {
					filenameStream << this->options_.getFilenamePath_Topology() << "/" << this->options_.getFilenamePrefix_Topology() << "." << nameSuffix << "." << sec << ".txt";
				}
				const std::string filename = filenameStream.str();
				topology->saveToFile(filename);
			}

		private:

			const SpikeRuntimeOptions options_;

		};
	}
}