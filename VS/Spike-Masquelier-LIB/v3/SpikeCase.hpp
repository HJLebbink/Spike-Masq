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

#include <vector>
#include <array>
#include <bitset>

#include "../../Spike-Tools-LIB/SpikeTypes.hpp"

#include "Types.hpp"
#include "SpikeOptionsStatic.hpp"


namespace spike
{
	namespace v3
	{
		//SpikeCase is an enhanced SpikeSet especially made for fast loading of input currents.
		template <typename Options_i>
		class SpikeCase
		{
		public:

			using Options = Options_i;

			// destructor
			~SpikeCase() = default;

			// default constructor
			SpikeCase() = default;

			// constructor
			SpikeCase(const CaseId caseId, const CaseLabel caseLabel, const std::vector<NeuronId>& neuronIds, const TimeInMs caseLength, const TimeInMs tailSilence)
				: caseId_(caseId)
				, caseLabel_(caseLabel)
				, caseLength_(caseLength)
				, tailSilence_(tailSilence)
				, neuronIds_(neuronIds)
				, nNeurons_(neuronIds.size())
				, areAllNeuronsRandom_(true)
				, randomFireHz_(std::vector<float>(this->nNeurons_))
			{
				if (this->nNeurons_ == 0)
				{
					__debugbreak();
				}
				this->clear();
			}

			// copy constructor
			SpikeCase(const SpikeCase&) = delete;

			// move constructor
			SpikeCase(SpikeCase&& other)
				: caseId_(other.caseId_)
				, caseLabel_(other.caseLabel_)
				, caseLength_(other.caseLength_)
				, tailSilence_(other.tailSilence_)
				, neuronIds_(other.neuronIds_)
				, nNeurons_(other.nNeurons_)
				, data_(other.data_)
				, areAllNeuronsRandom_(other.areAllNeuronsRandom_)
				, randomFireHz_(other.randomFireHz_)
			{
				//std::cout << "Variable::move ctor" << std::endl;
			}

			// copy assignment
			SpikeCase& operator=(const SpikeCase&) = delete;

			// move assignment
			SpikeCase& operator=(SpikeCase&& other) // move assignment operator
			{
				//std::cout << "Variable::move assignment operator" << std::endl;
				BOOST_ASSERT_MSG_HJ(this != &other, "");

				this->caseId_ = other.caseId_;
				this->caseLabel_ = other.caseLabel_;
				this->caseLength_ = other.caseLength_;
				this->tailSilence_ = other.tailSilence_;
				this->nNeurons_ = other.nNeurons_;
				this->data_ = other.data_;
				this->areAllNeuronsRandom_ = other.areAllNeuronsRandom_;
				this->randomFireHz_ = std::move(other.randomFireHz_);

				return *this;	// by convention, always return *this
			}

			void setSpikeTimes(const NeuronId neuronId, const std::vector<TimeInMs>& spikeTimes)
			{
				this->data_[neuronId] = spikeTimes;
			}

			void setRandomSpikeHz(const NeuronId neuronId, const float spikeHz)
			{
				//std::cout << "spike::v3::SpikeCase:setRandomSpikeHz: neuronId=" << neuronId << "; spikeHz=" << spikeHz << std::endl;
				if (neuronId < this->randomFireHz_.size())
				{
					if (spikeHz == 0.0f)
					{
						if (this->randomFireHz_[neuronId] != 0.0f)
						{
							// randomFireHz_ has changed
							this->randomFireHz_[neuronId] = 0.0f;
							this->areAllNeuronsRandom_ = false;
						}
					}
					else
					{
						std::cout << "spike::v3::SpikeCase:setRandomSpikeHz: neuronId=" << neuronId << "; spikeHz=" << spikeHz << std::endl;
						if (this->randomFireHz_[neuronId] != spikeHz)
						{
							// randomFireHz_ has changed
							this->randomFireHz_[neuronId] = spikeHz;

							this->areAllNeuronsRandom_ = true;
							for (NeuronId neuronId2 = 0; neuronId2 < this->nNeurons_; neuronId2++)
							{
								if (this->randomFireHz_[neuronId2] == 0.0f)
								{
									this->areAllNeuronsRandom_ = false;
									return;
								}
							}
						}
					}
				}
				else
				{
					std::cout << "spike::v3::SpikeCase:setRandomSpikeHz: neuronId=" << neuronId << "; spikeHz=" << spikeHz << ": provided neuronId is too large." << std::endl;
				}
			}

			void setAllNeuronsRandomSpikeHz(const double spikeHz)
			{
				this->areAllNeuronsRandom_ = (spikeHz != 0.0);
				std::fill(this->randomFireHz_.begin(), this->randomFireHz_.end(), static_cast<float>(spikeHz));
			}

			bool areAllNeuronsRandom() const
			{
				return this->areAllNeuronsRandom_;
			}

			bool isRandom(const NeuronId neuronId) const
			{
				return this->randomFireHz_[neuronId] != 0.0f;
			}

			float getRandomFireHz(const NeuronId neuronId) const
			{
				return this->randomFireHz_[neuronId];
			}

			TimeInMs getDuration() const
			{
				return this->caseLength_;
			}

			TimeInMs getDurationPlusSilence() const
			{
				return this->caseLength_ + this->tailSilence_;
			}

			size_t getNumberOfNeurons() const
			{
				return this->nNeurons_;
			}

			CaseId getCaseId() const
			{
				return this->caseId_;
			}

			CaseLabel getCaseLabel() const
			{
				return this->caseLabel_;
			}

			void clear()
			{
				this->setAllNeuronsRandomSpikeHz(0.0f);
				for (size_t i = 0; i < Options::nNeurons; ++i)
				{
					this->data_[i].clear();
				}
			}

			std::string toString() const
			{
				std::ostringstream oss;

				if (this->areAllNeuronsRandom())
				{
					oss << "fully random case data";
				}
				else
				{
					oss << "spikeCase " << this->caseId_ << "; nNeurons=" << this->nNeurons_ << "; caseLength=" << this->caseLength_ << "; tailSilence=" << this->tailSilence_;
					//TODO
				}
				return oss.str();
			}

			// get the sorted vector of spike times for the provided neuronId
			const std::vector<TimeInMs>& getSpikeTimes(const NeuronId neuronId) const
			{
				return this->data_[neuronId];
			}

			const std::vector<NeuronId>& getNeuronIds() const
			{
				return this->neuronIds_;
			}

		private:

			CaseId caseId_;
			CaseLabel caseLabel_;

			std::vector<NeuronId> neuronIds_;
			size_t nNeurons_;
			TimeInMs caseLength_;	// length of the case spikes in kernelTime
			TimeInMs tailSilence_; // silence after the case spikes in kernelTime

			std::array<std::vector<TimeInMs>, Options::nNeurons> data_;

			// whether this case is fully random; if true than _caseDataPresent is undefined
			bool areAllNeuronsRandom_;

			// store which neuronIds contain random content; if this field is set to zero this indicates that the neuron is not random but with case data
			std::vector<float> randomFireHz_;

		};
	}
}