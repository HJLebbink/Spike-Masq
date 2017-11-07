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

#include "../../Spike-Tools-LIB/SpikeTypes.hpp"
#include "../../Spike-Tools-LIB/SpikeRuntimeOptions.hpp"

#include "SpikeDataSet.hpp"
#include "SpikeCaseFast.hpp"
#include "SpikeSetLarge.hpp"

namespace spike
{
	namespace dataset
	{
		using namespace spike::tools;

		using TimeType = TimeInMsI;

		template <typename Topology>
		class SpikeStream
		{
		public:

			static const size_t nNeurons = Topology::nNeurons;

			// destructor
			~SpikeStream() = default;

			// constructor
			SpikeStream(const SpikeRuntimeOptions& options)
				: options_(options)
				, currentCaseCounter_(0)
				, currentCaseLabel_(NO_CASE_LABEL)
				, useRandomCase_(false)
				, currentPosInCase_(0)
			{
				this->resetTime();
				this->randInt_ = rand();
				this->randomSpikeHz_.fill(-1);
				this->randomSpikeHzInteger_.fill(0);
				this->neuronDynamicType_.fill(NeuronDynamicType::RANDOM);
				this->nextRandomSpikeTime_.fill(std::numeric_limits<int>::max());
			}

			// copy assignment
			SpikeStream& operator= (const SpikeStream& rhs) = delete;

			void add(const std::shared_ptr<const SpikeCaseFast>& spikeCase)
			{
				//std::cerr << "SpikeStream::add(): entering; nCases = " << this->getNumberOfCases() << std::endl;

				if (spikeCase)
				{
					if (spikeCase->areAllNeuronsRandom())
					{
						//std::cerr << "SpikeStream::add_private(): all Neurons are random." << std::endl;

						this->useRandomCase_ = (spikeCase->getDurationInMs() > 0);
						this->currentCase_ = spikeCase;
						this->randomCaseData_ = spikeCase;
					}
					else
					{
						const CaseId caseId = spikeCase->getCaseId();
						std::cerr << "SpikeStream::add(): adding case with Id " << caseId << std::endl;
						if (this->hasCaseId(caseId))
						{
							std::cerr << "SpikeStream::add(): WARNING: provided caseId " << caseId << " is already present." << std::endl;
						}
						else
						{
							//std::cout << "SpikeStream::add_private(): adding with regular data caseId " << caseId << std::endl;
							this->currentCase_ = spikeCase;
							this->caseIdsVector_.push_back(caseId);
							this->data_.insert(std::make_pair(caseId, spikeCase));
						}
					}
				}
				else
				{
					std::cerr << "SpikeStream::add(): WARNING: provided spikeCase has no managed object." << std::endl;
				}
			}

			bool hasCaseId(const CaseId caseId) const
			{
				return std::find(this->caseIdsVector_.cbegin(), this->caseIdsVector_.cend(), caseId) != this->caseIdsVector_.cend();
			}

			const std::shared_ptr<const SpikeCaseFast> getSpikeCase(const CaseId caseId) const
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
					std::cerr << "SpikeStream::getSpikeCase(): ERROR: invalid caseId " << caseId << std::endl;
					throw std::runtime_error("caseId does not exist");
				}
			}

			const std::shared_ptr<const SpikeCaseFast> getRandomSpikeCase() const
			{
				return this->randomCaseData_;
			}

			const std::vector<CaseId> getCaseIds() const
			{
				return this->caseIdsVector_;
			}

			void resetTime()
			{
				this->timeInMs_ = 0;
				this->ms_ = 0;
			}

			TimeType getTimeInMs() const
			{
				return this->timeInMs_;
			}

			unsigned int getNumberOfCases() const
			{
				return static_cast<unsigned int>(this->caseIdsVector_.size());
			}

			void start()
			{
				this->resetTime();
				this->startNewCase();
			}

			void incTime()
			{
				//std::cout << "SpikeCases::incTime()" << std::endl;

				//1] increment the absolute time since the last time reset.
				if (this->ms_ >= 999)
				{
					//std::cout << "SpikeCases::incTime(): round wrap time" << std::endl;
					this->ms_ = 0; // round wrap time
				}
				else
				{
					this->ms_++;
				}

				//2] start a new case if the current case is finished
				this->currentPosInCase_++;

				//3] increment the absolute time;
				this->timeInMs_++;
			}

			void startNewCaseIfNeeded()
			{
				if (this->currentPosInCase_ >= static_cast<unsigned int>(this->currentCase_->getDurationPlusSilenceInMs()))
				{
					this->startNewCase();
				}
			}

			NeuronDynamicType getNeuronDynamicType(const NeuronId neuronId) const
			{
				return this->neuronDynamicType_[neuronId];
			}

			bool getClampedValue(const NeuronId neuronId) const
			{
				//	std::cout << "SpikeCases::getClampedValue(): neuronId" << neuronId << "; currentPosInCase=" << this->currentPosInCase<< std::endl;
				const Ms pos = static_cast<Ms>(this->currentPosInCase_);
				return this->currentCase_->getClampedValue(pos, neuronId);
			}

			template <typename D>
			void addSpikeDataSet(
				const std::shared_ptr<SpikeDataSet<D>>& spikeDataSet)
			{
				// non-speed critical code

				//1] reset current state;
				this->resetTime();
				this->clearCaseUsage();
				this->caseIdsVector_.clear();
				this->data_.clear();

				//2]
				const std::set<NeuronId> neuronsInCases = spikeDataSet->getNeuronIds();
				const size_t nNeuronsInCases = neuronsInCases.size();
				const size_t nSensorNeurons = Topology::Ns;

				if (nNeuronsInCases > nSensorNeurons)
				{
					std::cerr << "SpikeStream::addSpikeDataSet(): too many neurons in cases " << nNeuronsInCases << " while only " << nSensorNeurons << " sensor neurons exist" << std::endl;
					//				throw std::runtime_error("too many neurons in cases");
					//DEBUG_BREAK();
				}
				const std::set<CaseId> caseIds = spikeDataSet->getCaseIds();

				const TimeType caseDuration = spikeDataSet->getCaseDurationInMs();
				if (caseDuration > 1000)
				{
					std::cerr << "SpikeStream:addSpikeDataSet:: case duration of " << caseDuration << " is too long for SpikeStream, maximal length is 1000ms." << std::endl;
					//DEBUG_BREAK();
				}


				const Ms caseTailSilenceInMs = this->options_.getCaseTailSilenceInMs();
				const Ms caseDurationInMs = static_cast<Ms>(caseDuration);
				std::cout << "SpikeStream:addSpikeDataSet(): loading " << caseIds.size() << " cases with " << nNeuronsInCases << " neurons (with " << nSensorNeurons << " nSensorNeurons; caseDuration " << caseDurationInMs << " ms; tailSilence " << caseTailSilenceInMs << " ms)" << std::endl;

				std::vector<NeuronId> allNeuronIds;
				for (const NeuronId& neuronId : Topology::iterator_AllNeurons())
				{
					allNeuronIds.push_back(neuronId);
				}

				for (const CaseId& caseId : caseIds)
				{
					const CaseLabel caseLabel = spikeDataSet->getClassificationLabel(caseId);
					const auto spikeCase = std::make_shared<SpikeCaseFast>(caseId, caseLabel, allNeuronIds, caseDurationInMs, caseTailSilenceInMs);

					//1.a] set the randomHz of all the neurons equal to the random spike Hz
					spikeCase->setAllNeuronsRandomSpikeHz(this->options_.getRandomSpikeHz());
					/*
					1.b] set the randomHz of the neurons in the spike case as provided by the spikeDataSet
					for (const NeuronId& neuronIdInCase : neuronsInCases) {
					const NeuronId sensorNeuronId = Topology::translateToSensorNeuronId(neuronIdInCase);
					spikeCase->setRandomSpikeHz(sensorNeuronId, spikeDataSet->getRandomHz(caseId, neuronIdInCase));
					}
					*/

					for (const NeuronId& sensorNeuronId : Topology::iterator_SensorNeurons())
					{
						spikeCase->setRandomSpikeHz(sensorNeuronId, 0);
					}

					//1.c] set the randomHz of the motor neuron
					for (const NeuronId& motorNeuronId : Topology::iterator_MotorNeurons())
					{
						spikeCase->setRandomSpikeHz(motorNeuronId, 0);
					}
					const NeuronId correctMotorNeuronId = Topology::translateToMotorNeuronId(caseLabel);
					spikeCase->setRandomSpikeHz(correctMotorNeuronId, static_cast<float>(this->options_.getCorrectNeuronSpikeHz()));


					//2] set the spikes of the caseData
					for (Ms timeInMs = 0; timeInMs < caseDurationInMs; ++timeInMs)
					{
						const auto spikeLine = spikeDataSet->getSpikeLine(caseId, static_cast<TimeType>(timeInMs));
						const auto spikePositions = spikeLine->getSpikePositions();

						//std::cout << "SpikeStream:addSpikeDataSet() caseId " << caseId << "; time " << timeInMs << "; spikeLine contains " << spikePositions.size() << " neurons" << std::endl;

						for (const NeuronId& neuronId : spikePositions)
						{
							const NeuronId sensorNeuronId = Topology::translateToSensorNeuronId(neuronId);
							spikeCase->setData(sensorNeuronId, timeInMs, true);
						}
					}
					this->add(std::move(spikeCase));
				}
			}

			const std::vector<CaseOccurance<Ms>> getCaseUsage() const
			{
				return this->caseOccurances_;
			}

			void clearCaseUsage()
			{
				this->caseOccurances_.clear();
			}

			bool hasRandomSpike(const NeuronId neuronId)
			{
				//std::cout << "SpikeStream::hasRandomSpike: neuronId=" << neuronId << "; currentPosInCase=" << this->currentPosInCase_ << "; nextRandomSpikeTime=" << this->nextRandomSpikeTime_[neuronId] << std::endl;
				if (static_cast<int>(this->currentPosInCase_) >= this->nextRandomSpikeTime_[neuronId])
				{
					this->updateRandomSpikeTime(neuronId);
					//std::cout << "SpikeStream::hasRandomSpike: neuronId=" << neuronId << "; currentPosInCase=" << this->currentPosInCase_ << "; nextRandomSpikeTime=" << this->nextRandomSpikeTime_[neuronId] << std::endl;
					//std::cout << "SpikeStream::hasRandomSpike: is about to return true" << std::endl;
					return true;
				}
				else
				{
					return false;
				}
			}

			CaseLabel getCurrentLabel() const
			{
				return this->currentCaseLabel_;
			}

			// hasSpike is only used by Masquelier code
			FiringReason hasSpike(const NeuronId neuronId)
			{
				const NeuronDynamicType neuronDynamicType = this->getNeuronDynamicType(neuronId);
				//std::cout << "SpikeStream::hasSpike: neuronId = " << neuronId << "; neuronDynamicType = " << static_cast<unsigned int>(neuronDynamicType) << " (random = 0, clamped = 1, forced = 2)" << std::endl;

				switch (neuronDynamicType)
				{

					case NeuronDynamicType::RANDOM:
					{
						if (this->hasRandomSpike(neuronId))
						{
							//std::cout << "SpikeStream::hasSpike: neuronId = " << neuronId << " is about the FIRE randomly" << std::endl;
							return FiringReason::FIRE_RANDOM;
						}
						else
						{
							return FiringReason::NO_FIRE;
						}
						break;
					}

					case NeuronDynamicType::CLAMPED:
					{
						const bool neuronClampedValue = this->getClampedValue(neuronId);

						if (neuronClampedValue)
						{
							return FiringReason::FIRE_CLAMPED;
						}
						else
						{
							return FiringReason::NO_FIRE;
						}
						break;
					}

					default:
					{
						//					__assume(0);
						// This tells the optimizer that the default cannot be reached. As so, it does not have to generate
						// the extra code to check that 'p' has a value not represented by a case arm. This makes the switch 
						// run faster.

						std::cerr << "hasSpike: neuronDynamicType = " << static_cast<unsigned int>(neuronDynamicType) << std::endl;
					}
				}
				//			__assume(0);
				std::cerr << "hasSpike: ERROR;" << std::endl;
				return FiringReason::NO_FIRE;
			}

			std::string toString() const
			{
				std::ostringstream oss;

				oss << "SpikeStream Info: timeInMs=" << this->timeInMs_ << "; ms=" << this->ms_ << "; nCases=" << this->caseIdsVector_.size() << "; current CaseId=" << this->currentCase_->getCaseId() << "; current position=" << this->currentPosInCase_ << std::endl;

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

			const SpikeRuntimeOptions options_;
			unsigned int randInt_;
			unsigned int currentCaseCounter_; // the total number of cases that have occured in this stream
			CaseLabel currentCaseLabel_;

			TimeType timeInMs_;
			Ms ms_;

			std::vector<CaseId> caseIdsVector_;
			std::map<CaseId, std::shared_ptr<const SpikeCaseFast>> data_;

			// randomSpikeHz is a cache of currentCase_->randomSpikeHz
			std::array<float, nNeurons> randomSpikeHz_;
			std::array<unsigned int, nNeurons> randomSpikeHzInteger_;
			std::array<int, nNeurons> nextRandomSpikeTime_;
			std::array<NeuronDynamicType, nNeurons> neuronDynamicType_;

			bool useRandomCase_;
			std::shared_ptr<const SpikeCaseFast> randomCaseData_;

			unsigned int currentPosInCase_;
			std::shared_ptr<const SpikeCaseFast> currentCase_;

			std::vector<CaseOccurance<Ms>> caseOccurances_;

			void startNewCase()
			{
				if (!this->currentCase_)
				{
					std::cout << "startNewCase: trying to start a new case, but there is no current case" << std::endl;
				}

				for (NeuronId i = 0; i < nNeurons; ++i)
				{
					this->nextRandomSpikeTime_[i] -= this->currentCase_->getDurationPlusSilenceInMs();
				}

				//std::cout << "startNewCase: starting a new case" << std::endl;
				this->currentPosInCase_ = 0; // reset the current position in the current case
				if (this->currentCase_->areAllNeuronsRandom())
				{  // the current (previous) case was complete random
//std::cout << "SpikeStream::startNewCase(): A: current case is random" << std::endl;
					if (this->caseIdsVector_.size() > 0)
					{
						this->startNewRegularCase();
					}
					else
					{
						//std::cout << "SpikeCases::startNewCase(): A: next cases is random case" << std::endl;
						this->currentCase_ = this->randomCaseData_;
						this->currentCaseLabel_ = NO_CASE_LABEL;
					}
				}
				else
				{
					//std::cout << "SpikeStream::startNewCase(): B: current case is normal (non-random)" << std::endl;
					if (this->useRandomCase_)
					{
						//std::cout << "SpikeStream::startNewCase(): B: next cases is random case" << std::endl;
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

			void updateAllNextRandomSpikes()
			{
				for (const NeuronId& neuronId : this->currentCase_->getNeuronIds())
				{
					const float randHz = this->currentCase_->getRandomFireHz(neuronId);
					if (this->randomSpikeHz_[neuronId] != randHz)
					{
						//std::cout << "updateAllNextRandomSpikes: neuronId=" << neuronId << " randHz=" << randHz << std::endl;

						this->randomSpikeHz_[neuronId] = randHz;
						this->randomSpikeHzInteger_[neuronId] = (randHz > 0.0) ? static_cast<unsigned int>(std::lroundf(2000.0f / randHz)) : 0;
						this->updateRandomSpikeTime(neuronId);
					}
					this->neuronDynamicType_[neuronId] = (this->currentCase_->isRandom(neuronId)) ? NeuronDynamicType::RANDOM : NeuronDynamicType::CLAMPED;
				}
			}

			void updateRandomSpikeTime(const NeuronId neuronId)
			{
				//std::cout << "updateRandomSpikeTime: neuronId=" << neuronId << std::endl;

				const unsigned int m = this->randomSpikeHzInteger_[neuronId];
				if (m > 0)
				{
					const Ms refractoryPeriodInMs = this->options_.getRefractoryPeriodInMs();

					this->randInt_ = ::tools::random::next_rand(this->randInt_);
					const unsigned int timeDelta = ::tools::random::priv::rescale_incl<0>(this->randInt_, m);
					this->nextRandomSpikeTime_[neuronId] = this->currentPosInCase_ + timeDelta + refractoryPeriodInMs;
					//std::cout << "SpikeStream::updateRandomSpikeTime(): neuronId=" << neuronId << "; m=" << m << "; timeDelta=" << timeDelta << "; nextRandomSpikeTime=" << this->nextRandomSpikeTime_[neuronId] << std::endl;
				}
				else
				{
					this->nextRandomSpikeTime_[neuronId] = std::numeric_limits<int>::max();
				}
			}

			void startNewRegularCase()
			{
				const unsigned int nCases = this->getNumberOfCases();

				#ifdef _DEBUG
				if (nCases == 0)
				{
					std::cerr << "SpikeStream::startNewRegularCase(): number of cases is too small " << nCases << std::endl;
					throw std::runtime_error("number of cases is too small");
				}
				#endif

				unsigned int randomIndex;
				if (false)
				{
					randomIndex = (nCases <= 1) ? 0 : ::tools::random::rand_int32(nCases);
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
				//std::cout << "SpikeStream::startNewRegularCase:: randomIndex=" << randomIndex << std::endl;

				const CaseId randomCaseId = this->caseIdsVector_[randomIndex];
				this->currentCase_ = this->data_[randomCaseId];

				const Ms startTime = this->ms_;
				const Ms endTime = this->ms_ + this->currentCase_->getDurationPlusSilenceInMs();
				this->currentCaseLabel_ = this->currentCase_->getCaseLabel();
				//std::cout << "SpikeStream::startNewRegularCase:: startTime=" << startTime << "; endTime=" << endTime << std::endl;
				this->caseOccurances_.push_back(CaseOccurance<Ms>(randomCaseId, startTime, endTime, this->currentCaseLabel_));
			}
		};
	}
}