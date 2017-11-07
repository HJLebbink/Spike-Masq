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

#include "../../Spike-Tools-LIB/SpikeSet1Sec.hpp"
#include "../../Spike-Tools-LIB/SpikeRuntimeOptions.hpp"

namespace spike {
	namespace tools {

		template <typename Time = Ms>
		class DumperSpikes
		{
		public:

			//DumperSpikes() = default;
			//DumperSpikes(const DumperSpikes&) = default;
			~DumperSpikes() = default;

			DumperSpikes(const SpikeRuntimeOptions& spikeRuntimeOptions)
				: spikeRuntimeOptions_(spikeRuntimeOptions)
				, spikeData_(SpikeSet1Sec<Time>())
			{}

			bool dumpTest(const unsigned int sec) const
			{
				return (this->spikeRuntimeOptions_.isDumpToFileOn_Spikes() && ((sec % this->spikeRuntimeOptions_.getDumpIntervalInSec_Spikes()) == 0));
			}

			void dump(
				const unsigned int sec,
				const std::string& nameSuffix,
				const SpikeSet1Sec<Time>& spikeData)
			{
				this->dump(sec, nameSuffix, spikeData, std::vector<CaseOccurance<Time>>());
			}

			void dump(
				const unsigned int sec,
				const std::string& nameSuffix,
				const SpikeSet1Sec<Time>& spikeData,
				const std::vector<CaseOccurance<Time>>& caseOccurances)
			{
				// create the filename
				std::stringstream filenameStream;
				if (nameSuffix.empty() || nameSuffix.length() == 0) {
					filenameStream << this->spikeRuntimeOptions_.getFilenamePath_Spikes() << "/" << this->spikeRuntimeOptions_.getFilenamePrefix_Spikes() << "." << sec << ".txt";
				} else {
					filenameStream << this->spikeRuntimeOptions_.getFilenamePath_Spikes() << "/" << this->spikeRuntimeOptions_.getFilenamePrefix_Spikes() << "." << nameSuffix << "." << sec << ".txt";
				}
				const std::string filename = filenameStream.str();

				//std::cout << "Spike_Network_Dump::dumpSpikeData1SecToFile(): filename=" << filename << std::endl;
				this->spikeData_.clear();
				this->spikeData_.addCaseOccurances(caseOccurances);
				this->spikeData_.setTimeSecond(sec);
				for (unsigned int i = 1; i < spikeData.nFirings_; ++i) { // first element in firings is a dummy element
					if ((spikeData.firingTime_[i] < 1000) && (spikeData.firingTime_[i] >= 0)) {
						this->spikeData_.addFiring(spikeData.firingTime_[i], spikeData.firingNeuronId_[i], spikeData.firingReason_[i]);
					}
				}
				this->spikeData_.freeze();
				this->spikeData_.saveToFile(filename);
			}

		private:

			SpikeRuntimeOptions spikeRuntimeOptions_;
			SpikeSet1Sec<Time> spikeData_;

		};
	}
}