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
#include <unordered_map>
#include <fstream>
#include <iostream> // for cerr and cout
#include <memory>

#include "../../Spike-Tools-LIB/DumperSpikes.hpp"
#include "../../Spike-Tools-LIB/SpikeRuntimeOptions.hpp"

#include "../../Spike-Masquelier-LIB/v0/SpikeOptionsMasq.hpp"
#include "../../Spike-Masquelier-LIB/v0/SpikeTools.hpp"

namespace spike
{
	namespace v0
	{
		class DumperSpikesMasquelier
		{
		public:

			using Time = float;
			//using Time = ::spike::Ms;

			static const unsigned int maxNumberOfSpikes = 200000;

			DumperSpikesMasquelier() = delete;

			DumperSpikesMasquelier(const DumperSpikesMasquelier& other) = delete;

			DumperSpikesMasquelier(
				const SpikeOptionsMasq& options,
				const SpikeRuntimeOptions& spikeRuntimeOptions)
				: options_(options)
				, spikeRuntimeOptions_(spikeRuntimeOptions)
				, dumperSpikes_(DumperSpikes<Time>(spikeRuntimeOptions))
				, sec_(0)
			{
				this->startDumpFile(this->sec_);
			}

			~DumperSpikesMasquelier() = default;

			// copy assignment
			DumperSpikesMasquelier& operator= (const DumperSpikesMasquelier& rhs) = delete;


			void setSpikesOutputFilename(const std::string& spikesOutputFilenamePrefix)
			{
				this->spikeOutputFilenamePrefix_ = spikesOutputFilenamePrefix;
			}

			void addNextSpike(const SpikeNeuronId neuronId, const SpikeTime spikeTime, const FiringReason firingReason)
			{
				//std::cout << "DumperSpikesMasquelier::addSpike: adding spike for neuronId " << neuronId << "; spikeTime=" << spikeTime << std::endl;

				const float spikeTimeInMsFloat = static_cast<float>(spikeTime) / SpikeOptionsMasq::TIME_DENOMINATOR;
				const unsigned int sec = static_cast<unsigned int>(floor(spikeTimeInMsFloat / 1000));
				const float spikeTimeInMsFloat2 = spikeTimeInMsFloat - (sec * 1000);

				if (sec > this->sec_)
				{
					this->closeDumpFile();
					this->startDumpFile(sec);
				}
				else
				{
					const NeuronId neuronId2 = static_cast<NeuronId>(neuronId);
					this->spikeData_.addFiring(static_cast<Time>(spikeTimeInMsFloat2), neuronId2, firingReason);
				}
			}

			void addCaseOccurance(const CaseOccurance<Time>& caseOccurance)
			{
				this->caseOccurances_.push_back(caseOccurance);
			}

		private:

			SpikeOptionsMasq options_;
			SpikeRuntimeOptions spikeRuntimeOptions_;

			DumperSpikes<Time> dumperSpikes_;
			SpikeSet1Sec<Time> spikeData_;
			std::vector<CaseOccurance<Time>> caseOccurances_;

			std::string spikeOutputFilenamePrefix_;
			unsigned int sec_;

			void closeDumpFile()
			{
				this->dumperSpikes_.dump(this->sec_, "", this->spikeData_, this->caseOccurances_);
			}

			void startDumpFile(const unsigned int sec)
			{
				this->sec_ = sec;
				this->spikeData_.clear();
				this->caseOccurances_.clear();
			}
		};
	}
}