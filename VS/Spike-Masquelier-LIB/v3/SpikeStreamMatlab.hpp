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

#include <memory>	// for std::shared_ptr
#include <utility>	// for make_pair
#include <string>
#include <vector>
#include <array>
#include <map>
#include <bitset>

#include "../../Spike-Tools-LIB/SpikeRuntimeOptions.hpp"

#include "../../Spike-DataSet-LIB/SpikeSetLarge.hpp"
#include "../../Spike-DataSet-LIB/Translations.hpp"

#include "SpikeCase.hpp"
#include "SpikeDataSet.hpp"
#include "Types.hpp"

namespace spike
{
	namespace v3
	{
		using namespace spike::dataset;

		template <typename Topology_i>
		class SpikeStreamMatlab
		{
		public:

			using Topology = Topology_i;
			using Options = typename Topology_i::Options;

			static const size_t nNeurons = Options::nNeurons;

			// destructor
			~SpikeStreamMatlab() = default;

			// constructor
			SpikeStreamMatlab()
			{
			};

			// copy constructor
		//	SpikeStreamMatlab(const SpikeStreamMatlab& other) = default;
//			{};


			// constructor
			SpikeStreamMatlab(const SpikeRuntimeOptions& spikeRuntimeOptions)
				: spikeRuntimeOptions_(spikeRuntimeOptions)
				, useRandomCase_(false)
				, currentTime_(0)
				, currentTimeInCase_(0)
				, currentCaseCounter_(0)
				, currentCaseLabel_(NO_CASE_LABEL)
				, currentCaseStartTime_(0)
			{
				this->randInt_ = static_cast<unsigned int>(rand());
				this->randomSpikeHz_.fill(-1);
				this->randomSpikeHzInteger_.fill(0);
				this->nextRandomSpikeTime_.fill(std::numeric_limits<int>::max());
				this->nextSpikeTime_.fill(LAST_KERNEL_TIME);
				this->nextSpikeIndex_.fill(0);
			}

			// copy assignment
			SpikeStreamMatlab<Topology>& operator= (const SpikeStreamMatlab<Topology>& rhs)
			{
				this->spikeRuntimeOptions_ = rhs.spikeRuntimeOptions_;
				this->useRandomCase_ = rhs.useRandomCase_;
				this->currentTime_ = rhs.currentTime_;
				this->currentTimeInCase_ = rhs.currentTimeInCase_;
				this->currentCaseCounter_ = rhs.currentCaseCounter_;
				this->currentCaseLabel_ = rhs.currentCaseLabel_;
				this->currentCaseStartTime_ = rhs.currentCaseStartTime_;
			};

			void add(const std::shared_ptr<const SpikeCase<Options>>& spikeCase)
			{
				//std::cerr << "SpikeStreamMatlab::add(): entering; nCases = " << this->getNumberOfCases() << std::endl;

				if (spikeCase)
				{
					if (spikeCase->areAllNeuronsRandom())
					{
						//std::cerr << "SpikeStreamMatlab::add_private(): all Neurons are random." << std::endl;

						this->useRandomCase_ = (spikeCase->getDuration() > 0);
						this->currentCase_ = spikeCase;
						this->randomCaseData_ = spikeCase;
					}
					else
					{
						const CaseId caseId = spikeCase->getCaseId();
						//std::cerr << "spike::v3::SpikeStreamMatlab:add: adding case with Id " << caseId << std::endl;
						if (this->hasCaseId(caseId))
						{
							std::cerr << "spike::v3::SpikeStreamMatlab:add: WARNING: provided caseId " << caseId << " is already present." << std::endl;
						}
						else
						{
							//std::cout << "SpikeStreamMatlab::add_private(): adding with regular data caseId " << caseId << std::endl;
							this->currentCase_ = spikeCase;
							this->caseIdsVector_.push_back(caseId);
							this->data_.insert(std::make_pair(caseId, spikeCase));
						}
					}
				}
				else
				{
					std::cerr << "spike::v3::SpikeStreamMatlab:add: WARNING: provided spikeCase has no managed object." << std::endl;
				}
			}

			bool hasCaseId(const CaseId caseId) const
			{
				return std::find(this->caseIdsVector_.cbegin(), this->caseIdsVector_.cend(), caseId) != this->caseIdsVector_.cend();
			}

			const std::shared_ptr<const SpikeCase<Options>> getSpikeCase(const CaseId caseId) const
			{
				if (this->hasCaseId(caseId))
				{
					auto spikeCase = this->data_.at(caseId);
					#ifdef _DEBUG
					if (spikeCase->getCaseId() != caseId)
					{
						std::cerr << "SpikeCases::getSpikeCase(): ERROR: caseId does not equal the returned caseId." << std::endl;
						throw std::runtime_error("caseId does not equal the returned caseId");
					}
					#endif
					return spikeCase;
				}
				else
				{
					std::cerr << "SpikeStreamMatlab::getSpikeCase(): ERROR: invalid caseId " << caseId << std::endl;
					throw std::runtime_error("caseId does not exist");
				}
			}

			const std::shared_ptr<const SpikeCase<Options>> getRandomSpikeCase() const
			{
				return this->randomCaseData_;
			}

			const std::vector<CaseId> getCaseIds() const
			{
				return this->caseIdsVector_;
			}

			size_t getNumberOfCases() const
			{
				return this->caseIdsVector_.size();
			}

			void start()
			{
				this->startNewCase();
			}

			void substractTime(const KernelTime time)
			{
				std::cout << "spike::v3::SpikeStreamMatlab:substractTime: time=" << time << std::endl;
				//TODO
				//DEBUG_BREAK();
			}

			// return true if a next spike exists before future time, and advance the time to future time
			std::tuple<bool, KernelTime> getNextSpikeTimeAndAdvance(const NeuronId neuronId, const KernelTime futureTime)
			{
				const KernelTime spikeTime = this->nextSpikeTime_[neuronId];
				const KernelTime randomSpikeTime = this->nextRandomSpikeTime_[neuronId];

				if (spikeTime < randomSpikeTime)
				{
					const bool exists = (spikeTime < futureTime);// && (spikeTime >= this->currentTime_);
					if (exists)
					{
						this->updateNextSpikeTime(neuronId);
					}
					return std::make_tuple(exists, spikeTime);
				}
				else
				{
					const bool exists = (randomSpikeTime < futureTime);// && (spikeTime >= this->currentTime_);
					if (exists)
					{
						this->updateNextRandomSpikeTime(neuronId);
					}
					return std::make_tuple(exists, randomSpikeTime);
				}
			}

			void advanceCurrentTime(const KernelTime timeDelta)
			{
				this->currentTime_ += timeDelta;
				this->currentTimeInCase_ += timeDelta;
				//std::cout << "spike::v3::SpikeStreamMatlab::advanceCurrentTime: timeDelta " << timeDelta << "; new currentTime=" << this->currentTime_ << "; currentTimeInCase=" << this->currentTimeInCase_ << std::endl;
				if (this->currentTimeInCase_ > this->durationCurrentCase_)
				{
					this->startNewCase();
				}
			}

			template <typename D>
			void addSpikeDataSet(
				const SpikeDataSet<Options, D>& spikeDataSet)
			{
				// non-speed critical code

				//1] reset current state;
				this->clearCaseUsage();
				this->caseIdsVector_.clear();
				this->data_.clear();

				//2]
				const std::vector<NeuronId> neuronsInCases = spikeDataSet.getNeuronIds();
				const size_t nNeuronsInCases = neuronsInCases.size();
				const size_t nSensorNeurons = Topology::Ns;

				const std::vector<CaseId> caseIds = spikeDataSet.getCaseIds();


				std::vector<NeuronId> allNeuronIds;
				for (const NeuronId& neuronId : Topology::iterator_AllNeurons())
				{
					allNeuronIds.push_back(neuronId);
				}

				const TimeInMs caseTailSilenceInMs = static_cast<TimeInMs>(this->spikeRuntimeOptions_.getCaseTailSilenceInMs());
				std::cout << "spike::v3::SpikeStreamMatlab:addSpikeDataSet: loading " << caseIds.size() << " cases with " << nNeuronsInCases << " neurons (with " << nSensorNeurons << " nSensorNeurons; tailSilence " << caseTailSilenceInMs << " ms)" << std::endl;

				for (const CaseId& caseId : caseIds)
				{

					const TimeInMs caseDurationInMs = spikeDataSet.getCaseDuration(caseId);
					if (caseDurationInMs > 1000)
					{
						std::cerr << "spike::v3::SpikeStreamMatlab:addSpikeDataSet: case duration of " << caseDurationInMs << " is too long for SpikeStreamMatlab, maximal length is 1000 ms." << std::endl;
						//DEBUG_BREAK();
					}

					// get the case label of the to be created new spike case
					const CaseLabel caseLabel = spikeDataSet.getClassificationLabel(caseId);

					// create the spike case
					const auto spikeCase = std::make_shared<SpikeCase<Options>>(SpikeCase<Options>(caseId, caseLabel, allNeuronIds, caseDurationInMs, caseTailSilenceInMs));

					// set the randomHz of all the neurons equal to the random spike Hz
					spikeCase->setAllNeuronsRandomSpikeHz(this->spikeRuntimeOptions_.getRandomSpikeHz());
					for (const NeuronId& sensorNeuronId : Topology::iterator_SensorNeurons())
					{
						spikeCase->setRandomSpikeHz(sensorNeuronId, 0);
					}

					// set the randomHz of the motor neuron
					for (const NeuronId& motorNeuronId : Topology::iterator_MotorNeurons())
					{
						spikeCase->setRandomSpikeHz(motorNeuronId, 0);
					}
					const NeuronId correctMotorNeuronId = Topology::translateToMotorNeuronId(caseLabel);
					spikeCase->setRandomSpikeHz(correctMotorNeuronId, static_cast<float>(this->spikeRuntimeOptions_.getCorrectNeuronSpikeHz()));

					// set the spikes of the caseData
					for (const NeuronId& neuronId : spikeDataSet.getNeuronIds())
					{
						const NeuronId sensorNeuronId = Topology::translateToSensorNeuronId(neuronId);
						spikeCase->setSpikeTimes(sensorNeuronId, spikeDataSet.getSpikeTimes(caseId, neuronId));
					}

					// add (move) the newly created case to this spike stream
					this->add(std::move(spikeCase));
				}
			}

			const std::vector<CaseOccurance<TimeInMs>>& getCaseUsage() const
			{
				return this->caseOccurances_;
			}

			void clearCaseUsage()
			{
				this->caseOccurances_.clear();
			}

			CaseLabel getCurrentLabel() const
			{
				return this->currentCaseLabel_;
			}

			std::string toString() const
			{
				std::ostringstream oss;

				oss << "spike::v3::SpikeStreamMatlab: currentTimeInCase=" << this->currentTimeInCase_ << "ms; nCases=" << this->getNumberOfCases() << "; current CaseId=" << this->currentCase_->getCaseId() << "; current label=" << this->getCurrentLabel() << std::endl;

				if (this->useRandomCase_)
				{
					oss << this->randomCaseData_->toString() << std::endl;
				}
				else
				{
					oss << "no random case;" << std::endl;
				}

				for (const CaseId& caseId : this->caseIdsVector_)
				{
					oss << "case Id " << caseId.val << ":" << std::endl;
					oss << this->data_.at(caseId)->toString() << std::endl;
				}
				return oss.str();
			}

		private:

			SpikeRuntimeOptions spikeRuntimeOptions_;
			size_t currentCaseCounter_; // the total number of cases that have occured in this stream
			KernelTime currentTime_;

			CaseLabel currentCaseLabel_;
			KernelTime currentCaseStartTime_;
			KernelTime currentTimeInCase_;
			KernelTime durationCurrentCase_;

			bool useRandomCase_;
			std::shared_ptr<const SpikeCase<Options>> randomCaseData_;
			std::shared_ptr<const SpikeCase<Options>> currentCase_;

			std::vector<CaseOccurance<TimeInMs>> caseOccurances_;

			// next spike time is the current case's next spike time
			std::array<KernelTime, nNeurons> nextSpikeTime_;

			// next index of the next spike in the spikeDataSet
			std::array<unsigned int, nNeurons> nextSpikeIndex_;


			std::vector<CaseId> caseIdsVector_;
			std::map<CaseId, std::shared_ptr<const SpikeCase<Options>>> data_;

			unsigned int randInt_;
			// randomSpikeHz is a cache of currentCase_->randomSpikeHz
			std::array<float, nNeurons> randomSpikeHz_;
			std::array<unsigned int, nNeurons> randomSpikeHzInteger_;
			std::array<KernelTime, nNeurons> nextRandomSpikeTime_;


			void startNewCase()
			{
				//std::cout << "spike::v3::SpikeStreamMatlab::startNewCase: starting a new case" << std::endl;

				if (!this->currentCase_)
				{
					std::cout << "spike::v3::SpikeStreamMatlab::startNewCase: trying to start a new case, but there is no current case" << std::endl;
					////DEBUG_BREAK();
				}

				this->currentTimeInCase_ = 0; // reset the current position in the current case
				this->currentCaseStartTime_ = this->currentTime_;

				if (this->currentCase_->areAllNeuronsRandom())
				{  // the current (previous) case was complete random
//std::cout << "spike::v3::SpikeStreamMatlab::startNewCase(): A: current case is random" << std::endl;
					if (this->caseIdsVector_.size() > 0)
					{
						this->startNewRegularCase();
					}
					else
					{
						//std::cout << "spike::v3::SpikeCases::startNewCase(): A: next cases is random case" << std::endl;
						this->currentCase_ = this->randomCaseData_;
						this->currentCaseLabel_ = NO_CASE_LABEL;
					}
				}
				else
				{
					//std::cout << "spike::v3::SpikeStreamMatlab::startNewCase(): B: current case is normal (non-random)" << std::endl;
					if (this->useRandomCase_)
					{
						//std::cout << "spike::v3::SpikeStreamMatlab::startNewCase(): B: next cases is random case" << std::endl;
						this->currentCase_ = this->randomCaseData_;
						this->currentCaseLabel_ = NO_CASE_LABEL;
					}
					else
					{
						this->startNewRegularCase();
					}
				}
				this->updateAllNextRandomSpikes();
			}

			void startNewRegularCase()
			{
				const size_t nCases = this->getNumberOfCases();
				::tools::assert::assert_msg(nCases > 0, "number of cases is too small");

				size_t randomIndex;
				if (false)
				{
					randomIndex = (nCases <= 1) ? 0 : static_cast<size_t>(::tools::random::rand_int32(0U, static_cast<unsigned int>(nCases)));
				}
				else
				{
					this->currentCaseCounter_++;
					if (this->currentCaseCounter_ >= nCases)
					{
						this->currentCaseCounter_ = 0;
					}
					randomIndex = this->currentCaseCounter_;
				}
				//std::cout << "spike::v3::SpikeStreamMatlab:startNewRegularCase:: randomIndex=" << randomIndex << std::endl;

				const CaseId randomCaseId = this->caseIdsVector_[randomIndex];
				this->currentCase_ = this->data_[randomCaseId];

				this->nextSpikeIndex_.fill(0);
				for (const NeuronId& neuronId : Topology::iterator_SensorNeurons())
				{
					this->updateNextSpikeTime(neuronId);
				}
				for (const NeuronId& neuronId : Topology::iterator_MotorNeurons())
				{
					this->updateNextRandomSpikeTime(neuronId);
				}

				this->currentCaseLabel_ = this->currentCase_->getCaseLabel();
				this->durationCurrentCase_ = Options::toKernelTime(this->currentCase_->getDurationPlusSilence());

				const TimeInMs startTime = Options::toTimeInMs(this->currentCaseStartTime_);
				const TimeInMs endTime = startTime + this->currentCase_->getDurationPlusSilence();
				//std::cout << "spike::v3::SpikeStreamMatlab:startNewRegularCase:: caseId=" << randomCaseId << "; caseLabel=" << this->currentCaseLabel_ << "; startTime=" << startTime << "; endTime=" << endTime << std::endl;
				this->caseOccurances_.push_back(CaseOccurance<TimeInMs>(randomCaseId, startTime, endTime, this->currentCaseLabel_));
			}

			void updateAllNextRandomSpikes()
			{
				//std::cout << "spike::v3::SpikeStreamMatlab:updateAllNextRandomSpikes: entering; currentCase=" << this->currentCase_->toString() << std::endl;

				for (const NeuronId& neuronId : this->currentCase_->getNeuronIds())
				{
					const float randHz = this->currentCase_->getRandomFireHz(neuronId);
					//std::cout << "spike::v3::SpikeStreamMatlab:updateAllNextRandomSpikes: neuronId=" << neuronId << " randHz=" << randHz << std::endl;

					if (this->randomSpikeHz_[neuronId] != randHz)
					{
						//std::cout << "spike::v3::SpikeStreamMatlab:updateAllNextRandomSpikes: neuronId=" << neuronId << " randHz=" << randHz << std::endl;

						this->randomSpikeHz_[neuronId] = randHz;
						this->randomSpikeHzInteger_[neuronId] = (randHz > 0.0) ? static_cast<unsigned int>(std::lroundf(2000.0f / randHz)) : 0;
						this->updateNextRandomSpikeTime(neuronId);
					}
				}
			}

			void updateNextRandomSpikeTime(const NeuronId neuronId)
			{
				//std::cout << "spike::v3::SpikeStreamMatlab::updateNextRandomSpikeTime(): neuronId=" << neuronId << std::endl;

				const unsigned int m = this->randomSpikeHzInteger_[neuronId];
				if (m > 0)
				{
					const TimeInMs refractoryPeriodInMs = this->spikeRuntimeOptions_.getRefractoryPeriodInMs();

					this->randInt_ = ::tools::random::next_rand(this->randInt_);
					const KernelTime timeDelta = ::tools::random::priv::rescale_incl<0>(this->randInt_, m);
					this->nextRandomSpikeTime_[neuronId] = this->currentTime_ + Options::toKernelTime(timeDelta + refractoryPeriodInMs);
					//std::cout << "spike::v3::SpikeStreamMatlab::updateNextRandomSpikeTime(): neuronId=" << neuronId << "; m=" << m << "; timeDelta=" << timeDelta << "; nextRandomSpikeTime=" << this->nextRandomSpikeTime_[neuronId] << std::endl;
				}
				else
				{
					this->nextRandomSpikeTime_[neuronId] = LAST_KERNEL_TIME;
				}
			}

			void updateNextSpikeTime(const NeuronId neuronId)
			{
				const std::vector<TimeInMs> spikeTimes = this->currentCase_->getSpikeTimes(neuronId);

				unsigned int index = this->nextSpikeIndex_[neuronId];
				if (index < spikeTimes.size())
				{
					this->nextSpikeTime_[neuronId] = this->currentTime_ + Options::toKernelTime(spikeTimes[index]);
					index++;
					this->nextSpikeIndex_[neuronId] = index;
					//std::cout << "spike::v3::SpikeStreamMatlab::updateNextSpikeTime: neuron " << neuronId << ": setting next spike time to " << this->nextSpikeTime_[neuronId] << std::endl;
				}
				else
				{
					this->nextSpikeTime_[neuronId] = LAST_KERNEL_TIME;
				}
				//std::cout << "spike::v3::SpikeStreamMatlab::updateNextSpikeTime: neuron " << neuronId << ": setting next spike time to " << this->nextSpikeTime_[neuronId] << std::endl;
			}
		};
	}
}