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
#include <deque>
#include <memory>
#include <algorithm> // for std::min
#include <iostream> // for cerr and cout

#include "../Spike-Tools-LIB/SpikeRuntimeOptions.hpp"

#include "../Spike-DataSet-LIB/SpikeStream.hpp"
#include "../Spike-DataSet-LIB/SpikeDataSet.hpp"

#include "../Spike-Masquelier-LIB/v0/SpikeOptionsMasq.hpp"
#include "../Spike-Masquelier-LIB/v0/SpikeTools.hpp"

namespace spike
{
	namespace v0
	{
		using namespace ::spike::dataset;

		template <typename Topology>
		class SpikeInputContainerDynamic
		{
		public:

			using TimeType = float;

			static const size_t nNeurons = Topology::nNeurons;


			~SpikeInputContainerDynamic() = default;

			SpikeInputContainerDynamic() = delete;

			SpikeInputContainerDynamic(
				const std::shared_ptr<const SpikeOptionsMasq>& options,
				const std::shared_ptr<const SpikeRuntimeOptions>& spikeOptions
			)
				: options_(options)
				, spikeOptions_(spikeOptions)
				, isEmpty_(false)
				, lastNeuronThatFired_(static_cast<NeuronId>(-1))
				, spikeStream_(SpikeStream<nNeurons>(spikeOptions))
			{
			}

			void setInputFilename(const std::string& inputFilename)
			{
				this->inputFilename_ = inputFilename;
			}

			void setClassificationFilename(const std::string& filename)
			{
			}

			void load(const TimeType timeInMs)
			{
				this->loadInputData(timeInMs);
			}

			float latency(const float spikeTimeInMsFloat, const CaseId caseId) const
			{
				//std::cout << "SpikeInputContainerDynamic: latency: spikeTimeInMs=" << spikeTimeInMs << "; caseId=" << caseId.val << std::endl;
				const std::vector<CaseOccurance> caseOccurances = this->spikeStream_.getCaseUsage();
				if (caseOccurances.empty())
				{
					//std::cout << "SpikeInputContainerDynamic: latency: no cases have occurred yet."<< std::endl;
					return 0.0f;
				}
				else
				{
					const CaseOccurance lastCaseOccurance = caseOccurances.back();
					//std::cout << "SpikeInputContainerDynamic: latency: lastCaseOccurance=" << lastCaseOccurance.toString() << std::endl;

					if (lastCaseOccurance.caseId == caseId.val)
					{
						const TimeType endTime = lastCaseOccurance.endTime;
						const unsigned int sec = floor(spikeTimeInMsFloat / 1000);
						const TimeType spikeTimeInMs = spikeTimeInMsFloat - (sec * 1000);

						//std::cout << "SpikeInputContainerDynamic: latency; endTime = " << endTime << "; spikeTimeInMsFloat=" << spikeTimeInMsFloat << "; sec=" << sec << "; spikeTimeInMs=" << spikeTimeInMs << std::endl;
						const TimeType startTime = lastCaseOccurance.startTime;

						if ((spikeTimeInMs > startTime) && (spikeTimeInMs <= endTime))
						{

							const float lat = spikeTimeInMs - startTime;
							//std::cout << "SpikeInputContainerDynamic: latency; caseOccurances = " << lastCaseOccurance.toString() << "; spikeTimeInMs=" << spikeTimeInMs << "; startTime case=" << startTime << "; latency = " << lat << std::endl;

							if (lat > 1000)
							{
								std::cout << "SpikeInputContainerDynamic: latency; caseOccurances = " << lastCaseOccurance.toString() << "; spikeTimeInMs=" << spikeTimeInMs << "; startTime case=" << startTime << "; latency = " << lat << std::endl;
								throw 1;
							}

							return lat;
						}
						else
						{
							return 0.0f;
						}
					}
					else
					{
						//std::cout << "SpikeInputContainerDynamic: latency: lastCaseOccurance=" << lastCaseOccurance.toString() << "; case did not match with provided caseId=" << caseId << std::endl;
						return 0.0f;
					}
				}
			}

			SpikeEvent removeNextEvent()
			{
				const SpikeEvent nextEvent = this->nextSpikeEvent_;
				this->updateNextEvent();
				return nextEvent;
			}

			SpikeEvent getNextEvent() const
			{
				return this->nextSpikeEvent_;
			}

			bool isEmpty()
			{
				return this->isEmpty_;
			}

			float getMaxTimeInMs() const
			{
				return this->maxTimeInMs_;
			}

			std::string toString() const
			{
				std::ostringstream oss;
				oss << "inputFilename_=" << this->inputFilename_ << std::endl;
				oss << "maxTimeInMs_=" << this->maxTimeInMs_ << std::endl;
				oss << "spikeStream_=" << this->spikeStream_-> << std::endl;
				return oss.str();
			}

			const std::vector<CaseId> getCaseIds() const
			{
				return this->spikeStream_.getCaseIds();
			}


		private:

			const std::shared_ptr<const SpikeOptionsMasq> options_;
			const std::shared_ptr<const SpikeRuntimeOptions> spikeOptions_;
			SpikeStream<Topology> spikeStream_;

			TimeType maxTimeInMs_;

			SpikeEvent nextSpikeEvent_;
			std::string inputFilename_;
			bool isEmpty_;

			SpikeNeuronId lastNeuronThatFired_;

			void updateNextEvent()
			{
				SpikeNeuronId neuronId = this->lastNeuronThatFired_ + 1;
				//std::cout << "SpikeInputContainerDynamic::updateNextEvent: initial neuronId=" << neuronId << std::endl;
				while (true)
				{
					while (neuronId < nNeurons)
					{
						if (this->spikeStream_.hasSpike(neuronId) == FiringReason::NO_FIRE)
						{
							neuronId++;
							//std::cout << "SpikeInputContainerDynamic::updateNextEvent: next neuronId=" << neuronId << std::endl;
						}
						else
						{
							const TimeType timeInMs = this->spikeStream_.getTimeInMs();
							const SpikeTime spikeTime = std::lroundf(timeInMs * TIME_DENOMINATOR);
							this->lastNeuronThatFired_ = neuronId;
							this->nextSpikeEvent_ = makeSpikeEvent(spikeTime, neuronId);
							//std::cout << "SpikeInputContainerDynamic::updateNextEvent: neuronId " << neuronId << " has fired at " << timeInMs << " ms" << std::endl;
							return;
						}
					}
					neuronId = 0;
					this->spikeStream_.incTime();
					this->spikeStream_.startNewCaseIfNeeded();
					//std::cout << "SpikeInputContainerDynamic::updateNextEvent: incremented the time to " << this->spikeStream_->getTimeInMs() << " ms" << std::endl;

					if (static_cast<float>(this->spikeStream_.getTimeInMs()) >= this->maxTimeInMs_)
					{
						std::cout << "SpikeInputContainerDynamic::updateNextEvent: end of the stream has been reached at " << this->maxTimeInMs_ << " ms" << std::endl;
						this->isEmpty_ = true;
						return;
					}
				}
			}

			void loadInputData(const TimeType timeInMs)
			{
				//1] check if the length of this dynamic input container fits
				this->maxTimeInMs_ = timeInMs;

				std::vector<NeuronId> neuronIds(nNeurons);
				for (NeuronId neuronId = 0; neuronId < nNeurons; ++neuronId)
				{
					neuronIds[neuronId] = neuronId;
				}

				//1] create one random case
				const CaseId nextCaseId = CaseId(0);
				const CaseLabel caseLabel = NO_CASE_LABEL;
				const auto spikeCase = std::make_shared<SpikeCaseFast>(nextCaseId, caseLabel, neuronIds, this->spikeOptions_->getRandomCaseDurationInMs());
				spikeCase->setAllNeuronsRandomSpikeHz(this->spikeOptions_->getRandomSpikeHz());
				this->spikeStream_.add(std::move(spikeCase));

				//2.2 load the spike data
				const auto spikeDataSet = std::make_shared<SpikeDataSet<MyDataType>>();
				spikeDataSet->loadFromFile(this->inputFilename_);

				//2.3 add the spike data to the stream
				this->spikeStream_.addSpikeDataSet(spikeDataSet, neuronIds);

				this->spikeStream_.start();
				this->updateNextEvent(); // such that the a call to getNextEvent yields a correct event
			}
		};
	}
}