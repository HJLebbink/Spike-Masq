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
#include <map>
#include <memory>	// for std::shared_ptr

#include "Translations.hpp"
#include "SpikeLine.hpp"
#include "DataSetState.hpp"

#include "../../Spike-Tools-LIB/SpikeTypes.hpp"

namespace spike
{
	namespace dataset
	{
		template <class D = int>
		class SpikeDataSet
		{
		public:

			using TimeType = TimeInMsI;

			// destructor
			virtual ~SpikeDataSet() = default;

			// default constructor
			SpikeDataSet(): isInitialized_(false)
			{
			}

			void init(
				const std::set<NeuronId>& neuronIds,
				const TimeType durationInMs)
			{
				//1] clear old content
				this->clear();
				//std::cout << "SpikeDataSetState::init: going to add " << neuronIds.size() << " neurons." <<std::endl;

				this->durationInMs_ = durationInMs;

				//2] set bit positions
				unsigned int bitPosition = 0;
				for (const NeuronId& neuronId : neuronIds)
				{
					//std::cout << "SpikeDataSetState::init: adding neuron " << neuronId << std::endl;
					this->neuronIdPosition_.insert(std::make_pair(neuronId, bitPosition));
					bitPosition++;
				}
				this->isInitialized_ = true;
			}

			void clear()
			{
				this->neuronIdPosition_.clear();
				this->data_.clear();
			}

			TimeType getCaseDurationInMs() const
			{
				return this->durationInMs_;
			}

			size_t getNumberOfCases() const
			{
				return this->data_.size();
			}

			const std::set<CaseId> getCaseIds() const
			{
				std::set<CaseId> s;
				for (CaseIdType i = 0; i < this->getNumberOfCases(); ++i)
				{
					s.insert(CaseId(i));
				}
				return s;
			}

			size_t getNumberOfNeurons() const
			{
				return this->neuronIdPosition_.size();
			}

			const std::set<NeuronId> getNeuronIds() const
			{
				std::set<NeuronId> s;
				for (std::map<NeuronId, unsigned int>::const_iterator it = this->neuronIdPosition_.cbegin(); it != this->neuronIdPosition_.cend(); ++it)
				{
					s.insert(it->first);
				}
				return s;
			}

			void setData(
				const CaseId caseId,
				const NeuronId neuronId,
				const bool value,
				const TimeType timeInMs)
			{
				assert(timeInMs < this->getCaseDurationInMs());
				//std::cout << "SpikeDataSetState::setData: caseId=" << caseId << "; neuronId=" << neuronId << std::endl;

				if (!this->isInitialized_)
				{
					std::cout << "SpikeDataSetState::setData: not initialized" << std::endl;
					throw std::runtime_error("not initialized");
				}
				//	std::cout << "BitDataSet::setData(): entering" << std::endl;
				//std::cout << "DataSet::setData() caseId="<<caseId <<"; propertyId="<<propertyId <<"; value="<<value <<std::endl; 

				//1] allocate space to store the provided value; initialize on false
				while (this->data_.size() <= caseId.val)
				{
					this->addEmptyCase();
				}
				//2] save the provided data

				const unsigned int pos = this->neuronIdPosition_.at(neuronId);
				std::shared_ptr<SpikeLine> spikeLine = this->data_[caseId.val][timeInMs];
				spikeLine->setValue(pos, value);
			}

			const std::shared_ptr<const SpikeLine> getSpikeLine(
				const CaseId caseId,
				const TimeType timeInMs) const
			{
				return this->data_[caseId.val][timeInMs];
			}

			double getRandomHz(
				const CaseId caseId,
				const NeuronId neuronId) const
			{
				//__debugbreak();
				if (neuronId == NO_NEURON_ID)
				{
					std::cout << "SpikeDataSet::getRandomHz: case " << caseId << "; neuronId NO_NEURON_ID does not have an random spike Hz" << std::endl;
					return 0.0;
				}
				else
				{
					const auto it1 = this->randomSpikeHz_.find(caseId);
					if (it1 != this->randomSpikeHz_.end())
					{
						const auto map1 = it1->second;
						const auto it2 = map1.find(neuronId);
						if (it2 != map1.end())
						{
							const double hz = it2->second;
							//std::cout << "SpikeDataSet::getRandomHz: neuronId " << neuronId << "; caseId " << caseId << "; hz=" << hz << std::endl;
							return hz;
						}
					}
					//std::cout << "SpikeDataSet::getRandomHz: case " << caseId << "; neuronId " << neuronId << " does not have an random spike Hz" << std::endl;
					return 0.0;
				}
			}

			void setRandomHz(
				const CaseId caseId,
				const NeuronId neuronId,
				const double hz)
			{
				const auto it1 = this->randomSpikeHz_.find(caseId);
				if (it1 == this->randomSpikeHz_.end())
				{
					this->randomSpikeHz_[caseId] = std::map<NeuronId, double>();
				}
				this->randomSpikeHz_.at(caseId)[neuronId] = hz;
			}

			std::string toString() const
			{
				std::ostringstream oss;

				for (const CaseId& caseId : this->getCaseIds())
				{
					oss << "--------------------------------------" << std::endl;
					oss << "caseId " << caseId << std::endl;

					for (NeuronId neuronId : this->getNeuronIds())
					{
						oss << this->getRandomHz(caseId, neuronId) << " ";
					}
					oss << std::endl;

					for (TimeType timeInMs = 0; timeInMs < this->getCaseDurationInMs(); ++timeInMs)
					{
						oss << this->getSpikeLine(caseId, timeInMs)->toString() << std::endl;
					}
				}
				return oss.str();
			}

			unsigned int getNumberOfSpikes() const
			{
				unsigned int count = 0;
				for (const CaseId& caseId : this->getCaseIds())
				{
					for (TimeType timeInMs = 0; timeInMs < this->durationInMs_; ++timeInMs)
					{
						count += this->getSpikeLine(caseId, timeInMs)->getNumberOfSpikes();
					}
				}
				return count;
			}

			CaseLabel getClassificationLabel(const CaseId caseId) const
			{
				const std::map<CaseId, CaseLabelType>::const_iterator it = this->classification_.find(caseId);
				if (it != this->classification_.cend())
				{
					return CaseLabel(it->second);
				}
				std::cout << "SpikeDataSet::getClassificationLabel: could not find classification for caseId " << caseId << std::endl;
				return NO_CASE_LABEL;
			}

			void setClassificationLabel(
				const CaseId caseId,
				const CaseLabel caseLabel)
			{
				this->classification_[caseId] = caseLabel.val;
			}

			void load(
				const std::shared_ptr<const DataSetState<D>>& dataSetState,
				const std::shared_ptr<const Translations<D>>& translations)
			{
				const D missingValue = dataSetState->getOptions().getMissingValue();

				//1] initialize this spikeDataSetState
				const std::set<NeuronId> neuronIds = translations->getNeuronIds();
				const TimeType duration = translations->getDurationInMs();
				this->init(neuronIds, duration);

				//2] load the content into this spikeDataSetState 
				const std::vector<CaseId> caseIds = dataSetState->getCaseIds();
				for (const VariableId& inputVariableIds : dataSetState->getVariableIds())
				{
					//std::cout << "SpikeDataSetState::load(): dataLine size=" << dataLine.size() << std::endl;

					for (const CaseId& caseId : caseIds)
					{
						if ((caseId.val % 100) == 0) std::cout << "SpikeDataSetState::load: inputVariableIds " << inputVariableIds << "; caseId " << caseId << std::endl;

						const D value = dataSetState->getData(caseId, inputVariableIds);
						if (value != missingValue)
						{
							if (translations->hasTranslation(inputVariableIds, value))
							{
								this->loadOneVariable(caseId, translations->getTranslation(inputVariableIds, value));
							}
							else
							{
								std::cout << "SpikeDataSetState::load: unable to translate inputVariableIds=" << inputVariableIds << ", value=" << value << std::endl;
								throw std::runtime_error("unable to translate data");
							}
						}
					}
				}
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
					std::cerr << "SpikeDataSetBackendTxt::saveToFileBackend(): Unable to create directory " << tree << std::endl;
					throw std::runtime_error("unable to create directory");
				}

				std::ofstream outputFile(filename);
				if (!outputFile.is_open())
				{
					std::cerr << "SpikeDataSetBackendTxt::saveToFileBackend(): Unable to open file " << filename << std::endl;
				}
				else
				{
					std::cout << "SpikeDataSetBackendTxt::saveToFileBackend(): Opening file " << filename << std::endl;

					const std::set<CaseId> caseIds = this->getCaseIds();
					const unsigned int nCases = static_cast<unsigned int>(caseIds.size());
					const unsigned int nNeurons = static_cast<unsigned int>(this->getNumberOfNeurons());
					const unsigned int nSpikes = this->getNumberOfSpikes();

					//1] print the number of cases and the number of variables
					outputFile << "#SpikeDataSet <nCases> <nNeurons> <nSpikes>" << std::endl;
					outputFile << nCases << " " << nNeurons << " " << nSpikes << std::endl;

					//2] 
					outputFile << "#CaseData <caseId> <caseDurationInMs> <classificationLabel>" << std::endl;
					const TimeType caseDurationInMs = this->getCaseDurationInMs();
					for (const CaseId& caseId : caseIds)
					{
						const CaseLabel caseLabel = this->getClassificationLabel(caseId);
						outputFile << caseId << " " << caseDurationInMs << " " << caseLabel << std::endl;
					}

					//3] 
					const std::set<NeuronId> neuronIds = this->getNeuronIds();
					outputFile << "#NeuronData <caseId> <neuronId> <randomSpikeHz>" << std::endl;
					for (const CaseId& caseId : caseIds)
					{
						for (const NeuronId& neuronId : neuronIds)
						{
							outputFile << caseId << " " << neuronId << " " << this->getRandomHz(caseId, neuronId) << std::endl;
						}
					}

					//4] 
					outputFile << "#SpikeData <caseId> <neuronId> <time>" << std::endl;

					for (const CaseId& caseId : caseIds)
					{
						for (TimeType timeInMs = 0; timeInMs < caseDurationInMs; ++timeInMs)
						{
							const std::shared_ptr<const SpikeLine> spikeLine = this->getSpikeLine(caseId, timeInMs);
							//std::cout << "BitDataSet::saveToFile(): spikeLine=" << spikeLine.toString() << std::endl;

							const std::vector<NeuronId> positions = spikeLine->getSpikePositions();
							//std::cout << "BitDataSet::saveToFile(): bitPositions.size()=" << bitPositions.size() << std::endl;

							for (const NeuronId pos : positions)
							{
								//	std::cout << "BitDataSet::saveToFile(): pos=" << pos << std::endl;
								outputFile << caseId << " " << pos << " " << timeInMs << std::endl;
							}
						}
					}
				}
			}

			void loadFromFile(const std::string& filename)
			{
				// mutex to protect file access
				//static std::mutex mutex;

				// lock mutex before accessing file
				//std::lock_guard<std::mutex> lock(mutex);

				std::string line;
				std::ifstream inputFileStream(filename);

				if (!inputFileStream.is_open())
				{
					std::cerr << "SpikeDataSet::loadFromFile: Unable to open file " << filename << std::endl;
				}
				else
				{
					std::cout << "SpikeDataSet::loadFromFile: Opening file " << filename << std::endl;
					this->clear();

					//1] load the number of cases in this file
					if (!::tools::file::loadNextLine(inputFileStream, line))
					{
						std::cerr << "SpikeDataSet::loadFromFile: first line " << line << " has incorrect content" << std::endl;
						throw std::exception();
					}
					//std::cout << "loadFromFile() first line = " << line << std::endl;
					const std::vector<std::string> content1 = ::tools::file::split(line, ' ');
					if (content1.size() < 3)
					{
						std::cerr << "SpikeDataSet::loadFromFile: got less than 3 items at line " << line << std::endl;
						throw std::exception();
					}
					const unsigned int nCases = static_cast<unsigned int>(::tools::file::string2int(content1[0]));
					const unsigned int nNeurons = static_cast<unsigned int>(::tools::file::string2int(content1[1]));
					const unsigned int nSpikes = static_cast<unsigned int>(::tools::file::string2int(content1[2]));

					//3] load the case data
					std::set<CaseId> caseIds;
					TimeType durationInMs = 0;

					for (unsigned int i = 0; i < nCases; ++i)
					{
						::tools::file::loadNextLine(inputFileStream, line);
						const std::vector<std::string> content2 = ::tools::file::split(line, ' ');
						if (content2.size() < 3)
						{
							std::cerr << "SpikeDataSet::loadFromFile: got less than 3 items at line " << line << std::endl;
							throw std::exception();
						}
						const CaseId caseId = CaseId(static_cast<CaseIdType>(::tools::file::string2int(content2[0])));
						const TimeType durationInMsTmp = static_cast<TimeType>(::tools::file::string2int(content2[1]));
						const CaseLabel caseLabel = CaseLabel(static_cast<CaseLabelType>(::tools::file::string2int(content2[2])));
						//std::cout << "SpikeDataSet::loadFromFile: caseId=" << caseId << "; durationInMs=" << durationInMsTmp << "; caseLabel=" << caseLabel << std::endl;

						caseIds.insert(caseId);
						this->setClassificationLabel(caseId, caseLabel);

						if (durationInMs == 0)
						{
							durationInMs = durationInMsTmp;
						}
						else if (durationInMs != durationInMsTmp)
						{
							std::cerr << "SpikeDataSet::loadFromFile: duration is not equal" << std::endl;
							throw std::exception();
						}
					}

					std::set<NeuronId> neuronIds;
					for (NeuronId neuronId = 0; neuronId < nNeurons; ++neuronId)
					{
						neuronIds.insert(neuronId);
					}
					this->init(neuronIds, durationInMs);

					//4] handle 
					for (unsigned int i = 0; i < (nNeurons * nCases); ++i)
					{
						::tools::file::loadNextLine(inputFileStream, line);
						const std::vector<std::string> content3 = ::tools::file::split(line, ' ');
						if (content3.size() < 3)
						{
							std::cerr << "SpikeDataSet::loadFromFile: got less than 3 items at line " << line << std::endl;
							throw std::exception();
						}
						const CaseId caseId = CaseId(static_cast<CaseIdType>(::tools::file::string2int(content3[0])));
						const NeuronId neuronId = static_cast<NeuronId>(::tools::file::string2int(content3[1]));
						const float randomHz = ::tools::file::string2float(content3[2]);
						this->setRandomHz(caseId, neuronId, randomHz);
					}

					//5] handle the spikes
					for (unsigned int i = 0; i < nSpikes; ++i)
					{
						::tools::file::loadNextLine(inputFileStream, line);
						const std::vector<std::string> content4 = ::tools::file::split(line, ' ');
						if (content4.size() < 3)
						{
							std::cerr << "SpikeDataSet::loadFromFile: got less than 3 items at line " << line << std::endl;
							throw std::exception();
						}
						const CaseId caseId = CaseId(static_cast<CaseIdType>(::tools::file::string2int(content4[0])));
						const NeuronId neuronId = static_cast<NeuronId>(::tools::file::string2int(content4[1]));
						const TimeType timeInMs = static_cast<TimeType>(::tools::file::string2int(content4[2]));
						this->setData(caseId, neuronId, true, timeInMs);
					}
				}
			}

		private:

			bool isInitialized_;
			TimeType durationInMs_;

			std::map<NeuronId, unsigned int> neuronIdPosition_;
			std::map<CaseId, std::map<NeuronId, double>> randomSpikeHz_;
			std::map<CaseId, CaseLabelType> classification_;

			// first vector: cases; second vector: time in ms; third vector: bit of the variable at the given ms
			std::vector<std::vector<std::shared_ptr<SpikeLine>>> data_;

			void addEmptyCase()
			{
				const unsigned int nNeurons = static_cast<unsigned int>(this->neuronIdPosition_.size());
				std::vector<std::shared_ptr<SpikeLine>> emptyCase;
				for (TimeType time = 0; time < this->getCaseDurationInMs(); ++time)
				{
					emptyCase.push_back(std::make_shared<SpikeLine>(nNeurons));
				}
				this->data_.push_back(std::move(emptyCase));
			}

			void loadOneVariable(
				const CaseId caseId,
				const std::shared_ptr<const SpikeSetLarge>& spikeSet)
			{
				const std::set<NeuronId> neuronIds = spikeSet->getNeuronIds();
				for (TimeType timeInMs = 0; timeInMs < spikeSet->getDurationInMs(); ++timeInMs)
				{
					for (const NeuronId& neuronId : neuronIds)
					{
						const bool value = spikeSet->getSpike(neuronId, timeInMs);
						if (value)
						{
							this->setData(caseId, neuronId, value, timeInMs);
						}
					}
				}
			}
		};
	}
}
