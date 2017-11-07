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
#include <iostream> // for cerr and cout
#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>

#include "../../Spike-Masquelier-LIB/v0/SpikeEpspContainer.hpp"
#include "../../Spike-Masquelier-LIB/v0/SpikeOptionsMasq.hpp"
#include "../../Spike-Masquelier-LIB/v0/SpikeTools.hpp"

namespace spike
{
	namespace v0
	{
		class SpikeEpspContainer
		{
		public:

			~SpikeEpspContainer()
			{
				this->close();
			}

			SpikeEpspContainer() = delete;

			SpikeEpspContainer(const SpikeEpspContainer&) = delete;

			// copy assignment
			SpikeEpspContainer& operator=(const SpikeEpspContainer&) = delete;

			SpikeEpspContainer(
				const bool syncToFile,
				const SpikeOptionsMasq& options
			)
				: syncToFile_(syncToFile)
				, options_(options)
			{
			}

			void setOutputFilename(const std::string& outputFilename)
			{
				std::cout << "SpikeEpspContainer::setOutputFilename() ================================" << std::endl;
				std::cout << "SpikeEpspContainer::setOutputFilename() setting output file = " << outputFilename << std::endl;

				this->outputFile_ = std::ofstream(outputFilename, std::ofstream::out);
				if (!this->outputFile_.is_open())
				{
					std::cout << "Unable to open file = " << outputFilename << std::endl;
				}
				std::cout << "SpikeEpspContainer::setOutputFilename() ================================" << std::endl;
			}

			void storeSpikeEvent(const SpikeEvent spikeEvent)
			{
				//	std::cout << "SpikeEpspContainer::storeSpikeEvent(): storing event " << event2String(spikeEvent) << std::endl;
				this->outputEvents_.push_back(spikeEvent);

				if (this->syncToFile_)
				{
					this->writeToFile(spikeEvent);
				}
			}

			void writeToFile(const SpikeEvent spikeEvent)
			{
				//	std::cout << "SpikeEpspContainer::writeToFile(): writing event " << event2String(spikeEvent) << std::endl;
				const int time = static_cast<int>(getTimeFromSpikeEvent(spikeEvent));
				const double timeInSec = ((double)time / SpikeOptionsMasq::TIME_DENOMINATOR) / 1000;
				const int neuronId = static_cast<int>(getOriginatingNeuronIdFromSpikeEvent(spikeEvent));
				this->writeToFile(neuronId, timeInSec);
			}

			void writeToFile(const int neuronId, const double timeInSec)
			{
				this->outputFile_ << timeInSec << " " << neuronId << std::endl;
			}

			void close()
			{
				if (!this->syncToFile_)
				{
					for (size_t i = 0; i < this->outputEvents_.size(); ++i)
					{
						this->writeToFile(this->outputEvents_[i]);
					}
				}
				this->outputFile_.close();
			}

		private:

			const bool syncToFile_;
			const SpikeOptionsMasq options_;

			std::vector<SpikeEvent> outputEvents_;
			std::ofstream outputFile_;
		};
	}
}