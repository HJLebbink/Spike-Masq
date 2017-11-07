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
#include <sstream>	// for std::ostringstream
#include <vector>
#include <array>
#include <memory> // for make_unique, unique_ptr, shared_ptr

#include "../../Spike-Tools-LIB/SpikeRuntimeOptions.hpp"
#include "../../Spike-Tools-LIB/SpikeTypes.hpp"
#include "../../Spike-Tools-LIB/SpikeSet1Sec.hpp"

namespace spike
{
	namespace tools
	{
		template <typename Topology, typename Time>
		class SpikeNetworkPerformance
		{

		public:

			static const size_t nNeurons = Topology::nNeurons;
			static const CaseLabelType S = 10;

			// constructor
			SpikeNetworkPerformance()
			{
				this->clear();
			};

			// destructor
			~SpikeNetworkPerformance() = default;

			void addPerformanceMnist28x28(
				const std::vector<CaseOccurance<Time>>& caseOccurances,
				const SpikeSet1Sec<Time>& spikeData,
				const bool ignoreFirstSpike)
			{
				if (Topology::Nm != S)
				{
					//DEBUG_BREAK();
				}

				//output variable in the dataSet is varibleId 14x14 = 196
				if (Topology::Ns != (28 * 28))
				{
					//DEBUG_BREAK();
				}

				std::array<bool, S*S> tmpStore;

				std::array<NeuronId, S> outputNeuronIds;
				unsigned int k = 0;
				for (const NeuronId& neuronId : Topology::iterator_MotorNeurons())
				{
					outputNeuronIds[k] = neuronId;
					k++;
				}
				std::array<unsigned int, S> spikeCount;


				// the case occurances are stored in order of occurance time of the case

				for (const CaseOccurance<Time> caseOccurance : caseOccurances)
				{

					const Time startTime = caseOccurance.startTime_;
					const Time endTime = caseOccurance.endTime_;
					const CaseLabel correctLabel = CaseLabel(caseOccurance.caseLabel_);

					this->nTimesCasesIsPresented_[correctLabel.val]++;


					//std::cout << "analyzePerformanceMnist14x14: analyzing caseOccurrance: startTime=" << startTime << "; endTime=" << endTime << std::endl;

					//2] count the number of spikes of ouputNeuronId within the time range of caseOccurance.
					spikeCount.fill(0);
					tmpStore.fill(false);

					for (unsigned int i = ((ignoreFirstSpike) ? 1 : 0); i < spikeData.nFirings_; ++i)
					{
						const Time firingTime = spikeData.firingTime_[i];

						//std::cout << "analyzePerformanceMnist14x14: found a spike; neuron " << neuronId << std::endl;
						if (firingTime > endTime)
						{
							break; // break out of this loop and go to the next caseOccurrance
						}
						else if (firingTime >= startTime)
						{

							const FiringReason firingReason = spikeData.firingReason_[i];

							if ((firingReason == FiringReason::FIRE_PROPAGATED_CORRECT) || (firingReason == FiringReason::FIRE_PROPAGATED_INCORRECT))
							{
								const NeuronId neuronId = spikeData.firingNeuronId_[i];

								if ((neuronId >= outputNeuronIds[0]) && (neuronId <= outputNeuronIds[S - 1]))
								{
									//std::cout << "analyzePerformanceMnist14x14: found a propaged ouput neuron spike; neuron " << neuronId << std::endl;

									for (unsigned int j = 0; j < S; ++j)
									{
										if (neuronId == outputNeuronIds[j])
										{
											const CaseLabel observedLabel = CaseLabel(static_cast<CaseLabelType>(j));

											//std::cout << "analyzePerformanceMnist14x14: analyzing caseOccurrance: observedLabel=" << observedLabel << "; correctLabel=" << correctLabel << std::endl;

											tmpStore[index(observedLabel.val, correctLabel.val)] = true;
											break;
										}
									}
								}
							}
						}
					}

					for (unsigned int j = 0; j < tmpStore.size(); ++j)
					{
						if (tmpStore[j])
						{
							this->confusionMatrix_[j]++;
						}
					}
				}
				//	//DEBUG_BREAK();
			}

			std::string toStringConfusionMatrix() const
			{
				std::ostringstream oss;

				oss << "total:\t\t";
				for (CaseLabelType label = 0; label < S; ++label)
				{
					oss << this->nTimesCasesIsPresented_[label] << "\t";
				}
				oss << std::endl;
				for (CaseLabelType observedLabel = 0; observedLabel < S; ++observedLabel)
				{
					oss << "observed " << observedLabel << ":\t";
					for (CaseLabelType correctLabel = 0; correctLabel < S; ++correctLabel)
					{
						const unsigned int value = this->confusionMatrix_[this->index(correctLabel, observedLabel)];
						const unsigned int valueRandom = this->confusionMatrixRandom_[this->index(correctLabel, observedLabel)];

						if (correctLabel == observedLabel)
						{
							oss << "[" << value << "]";
						}
						else
						{
							if (value > 0)
							{
								oss << value;
							}
						}
						if (valueRandom > 0) oss << " R" << valueRandom;
						oss << "\t";
					}
					oss << std::endl;
				}
				return oss.str();
			}

			std::string toStringPerformance() const
			{
				std::ostringstream oss;

				for (CaseLabelType observedLabel = 0; observedLabel < S; ++observedLabel)
				{
					const float nCorrect = static_cast<float>(this->confusionMatrix_[index(observedLabel, observedLabel)]);

					const float recall = (nCorrect == 0.0) ? 0 : (nCorrect / this->nTimesCasesIsPresented_[observedLabel]);

					unsigned int sum = 0;
					for (unsigned int l2 = 0; l2 < S; ++l2)
					{
						sum += this->confusionMatrix_[index(l2, observedLabel)];
					}
					const float precision = (nCorrect == 0.0) ? 0 : (nCorrect / sum);
					const float minPrecision = (nCorrect == 0.0) ? 0 : (nCorrect / (S * nCorrect));

					oss << "label " << observedLabel << ": recall " << recall << "; precision " << precision << "; minPrecision " << minPrecision << std::endl;
				}

				return oss.str();
			}

			double getAveragePrecision() const
			{
				double precisionSum = 0;
				for (CaseLabelType observedLabel = 0; observedLabel < S; ++observedLabel)
				{
					const double nCorrect = static_cast<double>(this->confusionMatrix_[this->index(observedLabel, observedLabel)]);
					unsigned int sum = 0;
					for (unsigned int l2 = 0; l2 < S; ++l2)
					{
						sum += this->confusionMatrix_[this->index(l2, observedLabel)];
					}
					if (nCorrect > 0)
					{
						const double precision = nCorrect / sum;
						precisionSum += precision;
					}
				}
				return precisionSum / S;
			}

			void clear()
			{
				this->confusionMatrix_.fill(0);
				this->confusionMatrixRandom_.fill(0);
				this->nTimesCasesIsPresented_.fill(0);
			}

			void addEvent2(const CaseLabel observedLabel, const CaseLabel correctLabel, const FiringReason firingReason)
			{
				if (firingReason == FiringReason::FIRE_RANDOM)
				{
					this->confusionMatrixRandom_[index(observedLabel.val, correctLabel.val)]++;
				}
				else
				{
					this->confusionMatrix_[index(observedLabel.val, correctLabel.val)]++;
				}
			}

		private:

			unsigned int index(const unsigned int correctLabel, const unsigned int observedLabel) const
			{
				return (S * correctLabel) + observedLabel;
			}

			std::array<unsigned int, S> nTimesCasesIsPresented_;
			std::array<unsigned int, S * S> confusionMatrix_;
			std::array<unsigned int, S * S> confusionMatrixRandom_;
		};
	}
}