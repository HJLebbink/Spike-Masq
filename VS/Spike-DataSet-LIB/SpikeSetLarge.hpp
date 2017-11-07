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
#include <set>
#include <map>
#include <cassert>

#include "../../Spike-Tools-LIB/SpikeTypes.hpp"
#include "../../Spike-Tools-LIB/SpikeRuntimeOptions.hpp"
#include "../../Spike-Tools-LIB/random.ipp"
#include "../../Spike-Tools-LIB/file.ipp"

#include "SpikeLine.hpp"

namespace spike
{
	namespace dataset
	{
		// A lite-weight set of spikes that can store many spikes, for faster set of spikes see SpikeCaseFast
		class SpikeSetLarge
		{
		public:

			using TimeType = TimeInMsI;

			// destructor
			~SpikeSetLarge()
			{
				//std::cout << "SpikeSet::dtor: dataSize=" <<this->data_.size()<< std::endl;
				for (SpikeLine * spikeLine : this->data_)
				{
					delete spikeLine;
				}
			}

			// constructor
			SpikeSetLarge()
			{
			}

			// constructor: neurons in neuronIds are placed in the order of occurance, no duplicated are allowed
			SpikeSetLarge(const std::vector<NeuronId> neuronIds, const TimeType durationInMs)
			{
				//std::cout << "SpikeSet::ctor: durationInMs=" << durationInMs << std::endl;
				this->init(neuronIds, durationInMs);
			}

			// copy constructor
			SpikeSetLarge(const SpikeSetLarge& other) // copy constructor
				: durationInMs_(other.durationInMs_)
				, neuronIds_(other.neuronIds_)
				, neuronIdPosition_(other.neuronIdPosition_)
				, data_(other.data_)
			{
				//std::cout << "SpikeSet::copy ctor" << std::endl;
			}

			// move constructor
			SpikeSetLarge(SpikeSetLarge&& other) // move constructor
				: durationInMs_(other.durationInMs_)
				, neuronIds_(std::move(other.neuronIds_))
				, neuronIdPosition_(other.neuronIdPosition_)
				, data_(std::move(other.data_))
			{
				//std::cout << "SpikeSet::move ctor" << std::endl;
			}

			// copy assignment
			SpikeSetLarge& operator=(const SpikeSetLarge& other) // copy assignment operator
			{
				std::cout << "SpikeSet::copy assignment" << std::endl;
				if (this != &other) // protect against invalid self-assignment
				{
					this->durationInMs_ = other.durationInMs_;
					this->neuronIdPosition_ = other.neuronIdPosition_;
					this->neuronIds_ = other.neuronIds_;
					this->data_ = other.data_;
				}
				else
				{
					std::cout << "Variable::operator=: prevented self-assignment" << std::endl;
				}
				return *this;	// by convention, always return *this
			}

			// move assignment
			SpikeSetLarge& operator=(SpikeSetLarge&& other) // move assignment operator
			{
				std::cout << "SpikeSet::move assignment" << std::endl;
				assert(this != &other);

				this->durationInMs_ = other.durationInMs_;
				this->neuronIdPosition_ = other.neuronIdPosition_;
				this->neuronIds_ = std::move(other.neuronIds_);
				this->data_ = std::move(other.data_);

				return *this;	// by convention, always return *this
			}

			TimeType getDurationInMs() const
			{
				return this->durationInMs_;
			}

			const std::set<NeuronId> getNeuronIds() const
			{
				return this->neuronIds_;
			}

			unsigned int getNumberOfSpikes() const
			{
				unsigned int count = 0;
				for (const SpikeLine * const spikeLine : this->data_)
				{
					count += spikeLine->getNumberOfSpikes();
				}
				return count;
			}

			void setSpike(const NeuronId neuronId, const TimeType timeInMs)
			{
				//1] update the spikeline 
				SpikeLine * const spikeLine = this->data_[timeInMs];
				spikeLine->setValue(this->getNeuronPosition(neuronId), true);
			}

			bool getSpike(const NeuronId neuronId, const TimeType timeInMs) const
			{
				const SpikeLine * const spikeLine = this->data_[timeInMs];
				return spikeLine->getValue(this->getNeuronPosition(neuronId));
			}

			std::vector<NeuronId> getSpikes(const TimeType time) const
			{
				const SpikeLine * const spikeLine = this->data_[time];
				const std::vector<NeuronId> spikePositions = spikeLine->getSpikePositions();
				std::vector<NeuronId> spikes;
				spikes.reserve(spikePositions.size());
				for (const NeuronId spikePosition : spikePositions)
				{
					const NeuronId neuronId = this->neuronIdPositionInv_.at(spikePosition);
					spikes.push_back(neuronId);
				}
				return spikes;
			}

			double getFrequencyInHz() const
			{
				const double f = 1000 * (((double)this->getNumberOfSpikes() / this->neuronIds_.size()) / this->durationInMs_);
				std::cout << "SpikeSet::getFrequencyInHz: nNeurons = " << this->neuronIds_.size() << "; durationInMs_=" << this->durationInMs_ << "; nSpikes = " << this->getNumberOfSpikes() << "; freq = " << f << std::endl;
				return f;
			}

			std::string toString() const
			{
				std::ostringstream oss;

				oss << "Frequency " << this->getFrequencyInHz() << " Hz" << std::endl;
				for (const NeuronId& neuronId : this->getNeuronIds())
				{
					oss << neuronId << " ";
				}
				oss << std::endl;
				for (const SpikeLine * const spikeLine : this->data_)
				{
					oss << spikeLine->toString() << std::endl;
				}
				return oss.str();
			}

			void clearAll()
			{
				this->clearData();
				this->neuronIds_.clear();
				this->neuronIdPosition_.clear();
				this->neuronIdPositionInv_.clear();
			}

			void clearData()
			{
				for (SpikeLine * const spikeLine : this->data_)
				{
					spikeLine->clear();
				}
				this->data_.clear();
			}

			void setRandomContent(const spike::tools::SpikeRuntimeOptions& spikeRuntimeOptions)
			{
				this->clearData();

				const Ms refractoryPeriodInMs = spikeRuntimeOptions.getRefractoryPeriodInMs();
				const double targetHz = spikeRuntimeOptions.getRandomSpikeHz();
				const unsigned int randThreshold = static_cast<unsigned int>(std::lround(1000.0 / targetHz));

				//std::cout << "SpikeSet::setRandomContent: randThreshold = " << randThreshold << std::endl;
				//__debugbreak();

				unsigned int spikeCount = 0;
				unsigned int noSpikeCount = 0;

				const std::set<NeuronId> neuronIds = this->getNeuronIds();

				for (const NeuronId& neuronId : neuronIds)
				{
					for (TimeType timeInMs = 0; timeInMs < this->durationInMs_; ++timeInMs)
					{
						const bool spikes = (::tools::random::rand_int32(0U, randThreshold) == 0);

						if (spikes)
						{
							spikeCount++;
							this->setSpike(neuronId, timeInMs);
							timeInMs += refractoryPeriodInMs;
						}
						else
						{
							noSpikeCount++;
						}
					}
				}

				const float observedHz = (static_cast<float>(spikeCount) / (noSpikeCount + spikeCount)) / 1000;
				std::cout << "SpikeSet::setRandomContent: targetHz = " << targetHz << "; observedHz = " << observedHz << std::endl;

				//for (int i = 0; i < (1000 / this->nNeurons); i++) {
				//	const NeuronId neuronId = static_cast<NeuronId>(getrandom(this->nNeurons));
				//	inputCurrent[neuronId] = EFFICACY;
				//}
			}

			void saveToFile(const std::string& filename) const
			{
				// mutex to protect file access
				//static std::mutex mutex;

				// lock mutex before accessing file
				//std::lock_guard<std::mutex> lock(mutex);

				// create the directory
				const std::string tree = ::tools::file::getDirectory(filename);
				if (!::tools::file::mkdirTree(tree))
				{
					std::cerr << "SpikeSetLarge::saveToFile(): Unable to create directory " << tree << std::endl;
					throw std::runtime_error("unable to create directory");
				}

				FILE * const fs = fopen(filename.c_str(), "w");
				if (fs == nullptr)
				{
					std::cerr << "SpikeSetLarge::saveToFile: Error: could not write to file " << filename << std::endl;
					return;
				}
				fprintf(fs, "#spike <second> <nCases> <nSpikes>\n");
				const unsigned int second = 0;
				const unsigned int nCases = 0;
				const unsigned int nSpikes = this->getNumberOfSpikes();
				fprintf(fs, "%u %u %u\n", second, nCases, nSpikes);

				fprintf(fs, "#caseOccurance <caseId> <startTimeInMs> <endTimeInMs>\n");

				fprintf(fs, "#spikeData <timeInMs> <neuronId> <firingReason>\n");
				for (TimeType timeInMs = 0; timeInMs < this->durationInMs_; ++timeInMs)
				{
					for (const NeuronId neuronId : this->getSpikes(timeInMs))
					{
						const FiringReason firingReason = FiringReason::FIRE_CLAMPED;
						fprintf(fs, "%u %u %i\n", timeInMs, neuronId, static_cast<int>(firingReason));
					}
				}
				fclose(fs);
			}

			void loadFromFile(const std::string& filename)
			{
				// mutex to protect file access
				//static std::mutex mutex;

				// lock mutex before accessing file
				//std::lock_guard<std::mutex> lock(mutex);

				std::ifstream inputFile(filename);

				if (!inputFile.is_open())
				{
					std::cerr << "SpikeSetLarge::loadFromFile(): Unable to open file " << filename << std::endl;
				}
				else
				{
					std::cout << "SpikeSetLarge::loadFromFile(): Opening file " << filename << std::endl;

					this->clearAll();
					std::string line;

					//1] load the first content line
					unsigned int nFiringsLocal = 0;
					unsigned int nCaseOccurances = 0;

					if (::tools::file::loadNextLine(inputFile, line))
					{
						const std::vector<std::string> content = ::tools::file::split(line, ' ');
						if (content.size() == 3)
						{
							//const int second = ::tools::file::string2int(content[0]);
							nCaseOccurances = static_cast<unsigned int>(::tools::file::string2int(content[1]));
							nFiringsLocal = static_cast<unsigned int>(::tools::file::string2int(content[2]));
						}
						else
						{
							std::cerr << "SpikeData1Sec::loadFromFile(): ERROR A. line " << line << " has incorrect content" << std::endl;
						}
					}
					else
					{
						std::cerr << "SpikeData1Sec::loadFromFile(): ERROR A. file has too little content" << std::endl;
					}

					//2] load the case occurances
					for (unsigned int i = 0; i < nCaseOccurances; ++i)
					{
						if (::tools::file::loadNextLine(inputFile, line))
						{
							const std::vector<std::string> content = ::tools::file::split(line, ' ');
							if (content.size() == 3)
							{
								//	this->addCaseOccurance(CaseOccurance(::tools::file::string2int(content[0]), (Ms)::tools::file::string2int(content[1]), (Ms)::tools::file::string2int(content[2])));
							}
							else
							{
								std::cerr << "SpikeData1Sec::loadFromFile(): ERROR B. line " << line << " has incorrect content" << std::endl;
							}
						}
						else
						{
							std::cerr << "SpikeData1Sec::loadFromFile(): ERROR B. file has too little content" << std::endl;
						}
					}

					std::vector<TimeType> timeInMsArray(nFiringsLocal);
					std::vector<NeuronId> neuronIdsArray(nFiringsLocal);
					//std::vector<FiringReason> firingReasonArray(nFiringsLocal);

					NeuronId highestNeuronId = 0;

					//3] load the spike data
					unsigned int counter = 0;
					for (unsigned int i = 0; i < nFiringsLocal; ++i)
					{
						if (::tools::file::loadNextLine(inputFile, line))
						{
							const std::vector<std::string> content = ::tools::file::split(line, ' ');
							if (content.size() == 3)
							{
								const TimeType timeInMs = static_cast<TimeType>(::tools::file::string2int(content[0]));
								const NeuronId neuronId = static_cast<NeuronId>(::tools::file::string2int(content[1]));
								//const FiringReason firingReason = static_cast<FiringReason>(::tools::file::string2int(content[2]));

								timeInMsArray[counter] = timeInMs;
								neuronIdsArray[counter] = neuronId;
								//firingReasonArray[counter] = firingReason;

								counter++;

								if (counter % 100000 == 0)
								{
									std::cout << "SpikeData1Sec::loadFromFile(): counter=" << counter << "/" << nFiringsLocal << std::endl;
								}

								if (neuronId > highestNeuronId)
								{
									highestNeuronId = neuronId;
								}
							}
							else
							{
								std::cerr << "SpikeData1Sec::loadFromFile(): ERROR C. line " << line << " has incorrect content" << std::endl;
							}
						}
						else
						{
							std::cerr << "SpikeData1Sec::loadFromFile(): ERROR C. file has too little content" << std::endl;
						}
					}

					//4] check whether all spikes are retrieved
					const unsigned int nRetrievedFirings = static_cast<unsigned int>(timeInMsArray.size());
					if (nRetrievedFirings != nFiringsLocal)
					{
						std::cerr << "SpikeData1Sec::loadFromFile(): ERROR D. nRetrievedFirings=" << nRetrievedFirings << " while nFiringsLocal=" << nFiringsLocal << std::endl;
					}

					//5] init this SpikeSetLarge
					const TimeType lastSpikeTime = timeInMsArray[nRetrievedFirings - 1];
					const TimeType durationInMs = lastSpikeTime + 1;
					std::vector<NeuronId> neuronIds(highestNeuronId);
					for (unsigned int i = 0; i <= highestNeuronId; ++i)
					{
						neuronIds[i] = static_cast<NeuronId>(i);
					}
					this->init(neuronIds, durationInMs);

					//6] 
					for (unsigned int i = 0; i < nRetrievedFirings; ++i)
					{
						this->setSpike(neuronIdsArray[i], timeInMsArray[i]);
					}
				}
			}

		private:

			// duration of this spike set
			TimeType durationInMs_;

			// set of neuronIds of this spike set
			std::set<NeuronId> neuronIds_;

			// map of neuronId to position of the neuron in the spikeLines 
			std::map<NeuronId, const unsigned int> neuronIdPosition_;

			// map of position in the spikeLines to the neuronId
			std::map<unsigned int, NeuronId> neuronIdPositionInv_;

			std::vector<SpikeLine*> data_;

			// return the position of the provided neuronId in the spikeLine
			unsigned int getNeuronPosition(const NeuronId neuronId) const
			{
				if (this->hasNeuronId(neuronId))
				{
					return this->neuronIdPosition_.at(neuronId);
				}
				std::cerr << "SpikeSet::getBitPosition(): provided neuronId " << neuronId << " is not present in this spikeSet" << std::endl;
				throw std::runtime_error("provided neuronId is not present in this spikeSet");
			}

			bool hasNeuronId(const NeuronId neuronId) const
			{
				return (this->neuronIds_.find(neuronId) != this->neuronIds_.end());
			}

			void init(const std::vector<NeuronId> neuronIds, const TimeType durationInMs)
			{
				this->durationInMs_ = durationInMs;

				unsigned int position = 0;
				for (const NeuronId& neuronId : neuronIds)
				{
					this->neuronIds_.insert(neuronId);
					this->neuronIdPosition_.insert(std::make_pair(neuronId, position));
					this->neuronIdPositionInv_.insert(std::make_pair(position, neuronId));
					position++;
				}

				const size_t nNeurons = this->neuronIds_.size();
				if (nNeurons != neuronIds.size())
				{
					std::cerr << "SpikeSet: provided neuronIds has duplicates." << std::endl;
					throw std::exception();
				}

				//std::cout << "SpikeSet::ctor" << std::endl;
				this->data_.reserve(durationInMs);
				for (TimeType time = 0; time < this->durationInMs_; ++time)
				{
					this->data_.push_back(new SpikeLine(static_cast<unsigned int>(nNeurons)));
				}
			}
		};
	}
}