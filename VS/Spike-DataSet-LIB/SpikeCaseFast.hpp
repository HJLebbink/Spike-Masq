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
#include <bitset>

#include "../Spike-Tools-LIB/SpikeTypes.hpp"
#include "../Spike-Tools-LIB/assert.ipp"

#include "SpikeSetLarge.hpp"

namespace spike
{
	namespace dataset
	{
		//SpikeCase is an enhanced SpikeSet especially made for fast loading of input currents.
		class SpikeCaseFast
		{
		public:

			// destructor
			~SpikeCaseFast() = default;

			// default constructor
			SpikeCaseFast() = delete;

			// constructor
			SpikeCaseFast(const CaseId caseId, const CaseLabel caseLabel, const std::vector<NeuronId>& neuronIds, const Ms caseLength, const Ms tailSilence)
				: caseId_(caseId)
				, caseLabel_(caseLabel)
				, neuronIds_(neuronIds)
				, nNeurons_(static_cast<unsigned int>(neuronIds.size()))
				, caseLength_(caseLength)
				, tailSilence_(tailSilence)
				, caseData_(std::vector<bool>(this->nNeurons_ * caseLength))
				, areAllNeuronsRandom_(true)
				, randomFireHz_(std::vector<float>(this->nNeurons_))
			{
				//std::cout << "SpikeCase::ctor: caseId=" << caseId << "; nNeurons=" << this->_nNeurons << "; nMs=" << this->_nMs << std::endl;
				this->clear();
			}

			// copy constructor
			SpikeCaseFast(const SpikeCaseFast& other) = delete;

			// move constructor
			SpikeCaseFast(SpikeCaseFast&& other)
				: caseId_(other.caseId_)
				, caseLabel_(other.caseLabel_)
				, neuronIds_(other.neuronIds_)
				, nNeurons_(other.nNeurons_)
				, caseLength_(other.caseLength_)
				, tailSilence_(other.tailSilence_)
				, caseData_(other.caseData_)
				, areAllNeuronsRandom_(other.areAllNeuronsRandom_)
				, randomFireHz_(other.randomFireHz_)
			{
				//std::cout << "Variable::move ctor" << std::endl;
			}

			// copy assignment
			SpikeCaseFast& operator=(const SpikeCaseFast& other) = delete;

			// move assignment
			SpikeCaseFast& operator=(SpikeCaseFast&& other) // move assignment operator
			{
				//std::cout << "Variable::move assignment operator" << std::endl;
				assert(this != &other);

				this->caseId_ = other.caseId_;
				this->caseLabel_ = other.caseLabel_;
				this->caseLength_ = other.caseLength_;
				this->tailSilence_ = other.tailSilence_;
				this->nNeurons_ = other.nNeurons_;
				this->caseData_ = other.caseData_;
				this->areAllNeuronsRandom_ = other.areAllNeuronsRandom_;
				this->randomFireHz_ = std::move(other.randomFireHz_);

				return *this;	// by convention, always return *this
			}

			void setData(const NeuronId neuronId, const Ms time, const bool spike)
			{
				::tools::assert::assert_msg(neuronId < this->nNeurons_, "SpikeCaseFast::load() ERROR neuronId ", neuronId, " is too big: nNeurons=", this->nNeurons_);
				::tools::assert::assert_msg(time < this->caseLength_, "SpikeCaseFast::load() ERROR time is too big");

				//std::cout << "SpikeCase::setData() neuronId=" << neuronId << "; ms=" << time << "; spike=" << spike << std::endl;

				// set the data to the array
				this->caseData_[this->caseDataIndex(time, neuronId)] = spike;
			}

			void setRandomSpikeHz(const NeuronId neuronId, const float spikeHz)
			{
				//std::cout << "SpikeCaseFast::setRandomSpikeHz: neuronId=" << neuronId << "; spikeHz=" << spikeHz << std::endl;
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
						std::cout << "SpikeCaseFast::setRandomSpikeHz: neuronId=" << neuronId << "; spikeHz=" << spikeHz << std::endl;
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
					std::cout << "SpikeCaseFast::setRandomSpikeHz: neuronId=" << neuronId << "; spikeHz=" << spikeHz << ": provided neuronId is too large." << std::endl;
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

			Ms getDurationInMs() const
			{
				return this->caseLength_;
			}

			Ms getDurationPlusSilenceInMs() const
			{
				return this->caseLength_ + this->tailSilence_;
			}

			unsigned int getNumberOfNeurons() const
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
				std::fill(this->caseData_.begin(), this->caseData_.end(), false);
			}

			std::string toString() const
			{
				std::ostringstream oss;

				if (this->areAllNeuronsRandom())
				{
					oss << "fully random case data" << std::endl;
				}
				else
				{
					oss << "spikeCase " << this->caseId_ << "; nNeurons=" << this->nNeurons_ << "; caseLength=" << this->caseLength_ << "; tailSilence=" << this->tailSilence_ << std::endl;
					for (Ms ms = 0; ms < (this->caseLength_ + this->tailSilence_); ++ms)
					{
						for (NeuronId neuronId = 0; neuronId < this->nNeurons_; neuronId++)
						{

							// plot a space
							if ((neuronId % 8 == 0) && (neuronId > 0)) oss << " ";

							// plot 
							oss << (this->isRandom(neuronId) ? "r" : (this->getClampedValue(ms, neuronId) ? "o" : "-"));
						}
						oss << std::endl;
					}
				}
				return oss.str();
			}

			bool getClampedValue(const Ms time, const NeuronId neuronId) const
			{
				::tools::assert::assert_msg(time <= (this->caseLength_ + this->tailSilence_), "time ", time, " is too large");
				if (time < this->caseLength_)
				{
					return this->caseData_[this->caseDataIndex(time, neuronId)];
				}
				else
				{
					return false;
				}
			}

			const std::vector<NeuronId> getNeuronIds() const
			{
				return this->neuronIds_;
			}

		private:

			CaseId caseId_;
			CaseLabel caseLabel_;

			std::vector<NeuronId> neuronIds_;
			unsigned int nNeurons_;
			Ms caseLength_;	// length of the case spikes in ms
			Ms tailSilence_; // silence after the case spikes in ms

			std::vector<bool> caseData_;

			// whether this case is fully random; if true than _caseDataPresent is undefined
			bool areAllNeuronsRandom_;

			// store which neuronIds contain random content; if this field is set to zero this indicates that the neuron is not random but with case data
			std::vector<float> randomFireHz_;

			__forceinline unsigned int caseDataIndex(const Ms time, const NeuronId neuronId) const
			{
				return static_cast<unsigned int>((neuronId * this->caseLength_) + time);
			}
		};
	}
}
