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
#include <array>
#include <unordered_map>

#include "SpikeRuntimeOptions.hpp"

namespace spike
{
	namespace tools
	{
		template <typename Time, typename Voltage, typename Options_i>
		class DumperState
		{
		public:

			using Options = Options_i;

			DumperState() = default;
			DumperState(const DumperState&) = default;
			~DumperState() = default;

			DumperState(const SpikeRuntimeOptions& spikeRuntimeOptions)
				: spikeRuntimeOptions_(spikeRuntimeOptions)
			{
			}

			DumperState& operator=(const DumperState &d) = default;

			void clear()
			{
				for (NeuronId neuronId = 0; neuronId < Options::nNeurons; ++neuronId)
				{
					this->dataTime_[neuronId].clear();
					this->dataVoltage_[neuronId].clear();
					this->dataThreshold_[neuronId].clear();
				}
			}

			bool dumpTest(const unsigned int sec) const
			{
				return (this->spikeRuntimeOptions_.isDumpToFileOn_State() && ((sec % this->spikeRuntimeOptions_.getDumpIntervalInSec_State()) == 0));
			}

			void store(
				const NeuronId neuronId,
				const Time time,
				const Voltage voltage,
				const Voltage threshold)
			{
				this->dataTime_[neuronId].push_back(time);
				this->dataVoltage_[neuronId].push_back(voltage);
				this->dataThreshold_[neuronId].push_back(threshold);
			}

			void dump(
				const unsigned int sec,
				const std::string& nameSuffix)
			{
				this->dumpToFile_printf(sec, nameSuffix);
			}

		private:

			SpikeRuntimeOptions spikeRuntimeOptions_;


			std::array<std::vector<Time>, Options::nNeurons> dataTime_;
			std::array<std::vector<Voltage>, Options::nNeurons> dataVoltage_;
			std::array<std::vector<Voltage>, Options::nNeurons> dataThreshold_;

			void dumpToFile_printf(
				const unsigned int sec,
				const std::string& nameSuffix)
			{
				// create the filename
				std::stringstream filenameStream;
				if (nameSuffix.empty() || nameSuffix.length() == 0)
				{
					filenameStream << this->spikeRuntimeOptions_.getFilenamePath_State() << "/" << this->spikeRuntimeOptions_.getFilenamePrefix_State() << "." << sec << ".txt";
				}
				else
				{
					filenameStream << this->spikeRuntimeOptions_.getFilenamePath_State() << "/" << this->spikeRuntimeOptions_.getFilenamePrefix_State() << "." << nameSuffix << "." << sec << ".txt";
				}
				const std::string filename = filenameStream.str();

				// create the directory
				const std::string tree = ::tools::file::getDirectory(filename);
				if (!::tools::file::mkdirTree(tree))
				{
					std::cerr << "spike::tools::DumperState::dumpToFile_printf(): Unable to create directory " << tree << std::endl;
					throw std::runtime_error("unable to create directory");
				}

				FILE * const fs = fopen(filename.c_str(), "w");
				if (fs == nullptr)
				{
					std::cerr << "spike::tools::DumperState::dumpToFile_printf: Error: could not write to file " << filename << std::endl;
					return;
				}
				fprintf(fs, "#state <second> <nNeurons>\n");
				fprintf(fs, "%d %zd\n", sec, Options::nNeurons);

				fprintf(fs, "#stateData <neuronId> <ms> <v> <threshold> <0>\n");
				for (NeuronId neuronId = 0; neuronId < Options::nNeurons; ++neuronId)
				{
					for (size_t i = 0; i < this->dataTime_[neuronId].size(); ++i)
					{
						if (std::is_integral<Time>::value)
						{
							fprintf(fs, "%u %f %f %f %d\n", neuronId, this->dataTime_[neuronId][i], this->dataVoltage_[neuronId][i], this->dataThreshold_[neuronId][i], 0);
						}
						else if (std::is_floating_point<Time>::value)
						{
							const Voltage v = this->dataVoltage_[neuronId][i];
							if (std::isnan(v))
							{
								fprintf(fs, "%d %f NaN %f %d\n", neuronId, this->dataTime_[neuronId][i], this->dataThreshold_[neuronId][i], 0);
							}
							else
							{
								fprintf(fs, "%d %f %f %f %d\n", neuronId, this->dataTime_[neuronId][i], v, this->dataThreshold_[neuronId][i], 0);
							}
						}
						else
						{
							std::cerr << "spike::tools::DumperState::loadFromFile(): unsupported Time type" << std::endl;
							__debugbreak();
						}
					}
				}

				this->clear();
				fclose(fs);
			}
		};
	}
}