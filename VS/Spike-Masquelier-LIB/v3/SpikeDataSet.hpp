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

#include "../../Spike-Tools-LIB/file.ipp"
#include "../../Spike-DataSet-Lib/DataSetState.hpp"
#include "../../Spike-DataSet-LIB/Translations.hpp"

#include "Types.hpp"
#include "SpikeOptionsStatic.hpp"

namespace spike
{
	namespace v3
	{
		template <typename Options_i, typename D = int>
		class SpikeDataSet
		{
		public:

			using Options = Options_i;

			// destructor
			virtual ~SpikeDataSet() = default;

			// default constructor
			SpikeDataSet(): isInitialized_(false)
			{
			}

			void init(
				const size_t nNeurons,
				const size_t nCases)
			{
				//1] clear old content
				this->clear();
				//std::cout << "SpikeDataSetState::init: going to add " << neuronIds.size() << " neurons." <<std::endl;

				this->neuronIds_.resize(nNeurons);
				this->caseDuration_.resize(nCases);
				this->data_.resize(nCases);

				for (CaseIdType caseId = 0; caseId < nCases; ++caseId)
				{
					this->caseIds_.push_back(CaseId(caseId));
				}
				for (NeuronId neuronId = 0; neuronId < nNeurons; ++neuronId)
				{
					this->neuronIds_.push_back(neuronId);
				}

				//2] set bit positions
				unsigned int bitPosition = 0;
				for (NeuronId neuronId = 0; neuronId < nNeurons; ++neuronId)
				{
					//std::cout << "SpikeDataSetState::init: adding neuron " << neuronId << std::endl;
					this->neuronIdPosition_.insert(std::make_pair(neuronId, bitPosition));
					bitPosition++;
				}
				this->isInitialized_ = true;
			}

			void clear()
			{
				this->caseIds_.clear();
				this->neuronIds_.clear();
				this->neuronIdPosition_.clear();
				this->caseDuration_.clear();
				this->data_.clear();
			}

			size_t getNumberOfCases() const
			{
				return this->caseIds_.size();
			}

			const std::vector<CaseId>& getCaseIds() const
			{
				return this->caseIds_;
			}

			size_t getNumberOfNeurons() const
			{
				return this->neuronIds_.size();
			}

			const std::vector<NeuronId>& getNeuronIds() const
			{
				return this->neuronIds_;
			}

			TimeInMs getCaseDuration(const CaseId caseId) const
			{
				return this->caseDuration_[caseId.val];
			}

			void setCaseDuration(const CaseId caseId, const TimeInMs caseDuration)
			{
				this->caseDuration_[caseId.val] = caseDuration;
			}

			void addSpikeTime(
				const CaseId caseId,
				const NeuronId neuronId,
				const TimeInMs spikeTime)
			{
				//std::cout << "spike::v3::SpikeDataSet::addSpikeTime: caseId=" << caseId << "; neuronId=" << neuronId << std::endl;

				//::tools::assert::assert_msg(caseId < this->getCaseDurationInMs(), "spike::v3::SpikeDataSet::addSpikesTime: caseId " << caseId " is too large");
				::tools::assert::assert_msg(neuronId < Options::nNeurons, "spike::v3::SpikeDataSet:addSpikeTime: neuronId ", neuronId, " is too large");


				const size_t nSpikes = this->data_[caseId.val][neuronId].size();
				if (nSpikes > 0)
				{
					if (this->data_[caseId.val][neuronId][nSpikes - 1] >= this->data_[caseId.val][neuronId][nSpikes])
					{
						std::cout << "spike::v3::addSpikesTime:addSpikeTime: provided spikeTime " << spikeTime << " is not in temporal order" << std::endl;
						__debugbreak();
					}
				}
				this->data_[caseId.val][neuronId].push_back(spikeTime);
			}

			void setSpikeTimes(
				const CaseId caseId,
				const NeuronId neuronId,
				const std::vector<TimeInMs>& spikeTimes)
			{
				//std::cout << "spike::v3::SpikeDataSet:setSpikeTimes: caseId=" << caseId << "; neuronId=" << neuronId << std::endl;
				BOOST_ASSERT_MSG_HJ(this->isInitialized_, "spike::v3::SpikeDataSet:setSpikeTimes: not initialized");

				//BOOST_ASSERT_MSG_HJ(caseId < this->getCaseDurationInMs(), "spike::v3::SpikeDataSet::setSpikesTimes: caseId " << caseId " is too large");
				//BOOST_ASSERT_MSG_HJ(neuronId < Options::nNeurons, "spike::v3::SpikeDataSet:setSpikeTimes: neuronId " << neuronId " is too large");
				this->data_[caseId.val][neuronId] = spikeTimes;
			}

			const std::vector<TimeInMs>& getSpikeTimes(
				const CaseId caseId,
				const NeuronId neuronId) const
			{
				//std::cout << "spike::v3::SpikeDataSet::getSpikeTimes: caseId=" << caseId << "; neuronId=" << neuronId << std::endl;
				//BOOST_ASSERT_MSG_HJ(caseId < this->getCaseDurationInMs(), "spike::v3::SpikeDataSet::getSpikeTimes: caseId " << caseId " is too large");
				//BOOST_ASSERT_MSG_HJ(neuronId < Options::nNeurons, "spike::v3::SpikeDataSet:getSpikeTimes: neuronId " << neuronId << " is too large");
				return this->data_[caseId.val][neuronId];
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

					for (TimeInMs timeInMs = 0; timeInMs < this->getCaseDurationInMs(); ++timeInMs)
					{
						oss << this->getSpikeLine(caseId, timeInMs)->toString() << std::endl;
					}
				}
				return oss.str();
			}

			size_t getNumberOfSpikes() const
			{
				size_t count = 0;
				for (const CaseId& caseId : this->getCaseIds())
				{
					for (const NeuronId& neuronId : this->getNeuronIds())
					{
						count += this->data_[caseId][neuronId].size();
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
			/*
			void load(
				const std::shared_ptr<const DataSetState<D>>& dataSetState,
				const std::shared_ptr<const Translations<D>>& translations)
			{
				const D missingValue = dataSetState->getOptions().getMissingValue();

				//1] initialize this spikeDataSetState
				const std::set<NeuronId> neuronIds = translations->getNeuronIds();
				const TimeInMs duration = translations->getDurationInMs();
				this->init(neuronIds, duration);

				//2] load the content into this spikeDataSetState
				const std::vector<CaseId> caseIds = dataSetState->getCaseIds();
				for (const VariableId& inputVariableIds : dataSetState->getVariableIds()) {
					//std::cout << "SpikeDataSetState::load(): dataLine size=" << dataLine.size() << std::endl;

					for (const CaseId& caseId : caseIds) {
						if ((caseId.val % 100) == 0) std::cout << "SpikeDataSetState::load: inputVariableIds " << inputVariableIds << "; caseId " << caseId << std::endl;

						const D value = dataSetState->getData(caseId, inputVariableIds);
						if (value != missingValue) {
							if (translations->hasTranslation(inputVariableIds, value)) {
								this->loadOneVariable(caseId, translations->getTranslation(inputVariableIds, value));
							} else {
								std::cout << "SpikeDataSetState::load: unable to translate inputVariableIds=" << inputVariableIds << ", value=" << value << std::endl;
								throw std::runtime_error("unable to translate data");
							}
						}
					}
				}
			}
			*/
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

				std::ofstream outputFile = std::ofstream(filename);
				if (!outputFile.is_open())
				{
					std::cerr << "SpikeDataSetBackendTxt::saveToFileBackend(): Unable to open file " << filename << std::endl;
				}
				else
				{
					std::cout << "SpikeDataSetBackendTxt::saveToFileBackend(): Opening file " << filename << std::endl;

					const std::set<CaseId> caseIds = this->getCaseIds();
					const size_t nCases = caseIds.size();
					const size_t nNeurons = this->getNumberOfNeurons();
					const size_t nSpikes = this->getNumberOfSpikes();

					//1] print the number of cases and the number of variables
					outputFile << "#SpikeDataSet <nCases> <nNeurons> <nSpikes>" << std::endl;
					outputFile << nCases << " " << nNeurons << " " << nSpikes << std::endl;

					//2] 
					outputFile << "#CaseData <caseId> <caseDurationInMs> <classificationLabel>" << std::endl;
					const TimeInMs caseDurationInMs = this->getCaseDurationInMs();
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
					outputFile << "#SpikeData <caseId> <neuronId> <timeInMs>" << std::endl;
					for (const CaseId& caseId : caseIds)
					{
						for (const NeuronId& neuronId : neuronIds)
						{
							for (const TimeInMs timeInMs : this->data_[caseId][neuronId])
							{
								outputFile << caseId << " " << neuronId << " " << timeInMs << std::endl;
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
					std::cerr << "spike::v3::SpikeDataSet:loadFromFile: Unable to open file " << filename << std::endl;
				}
				else
				{
					std::cout << "spike::v3::SpikeDataSet:loadFromFile: Opening file " << filename << std::endl;
					this->clear();

					//1] load the number of cases in this file
					if (!::tools::file::loadNextLine(inputFileStream, line))
					{
						std::cerr << "spike::v3::SpikeDataSet:loadFromFile: first line " << line << " has incorrect content" << std::endl;
						throw std::exception();
					}
					//std::cout << "spike::v3::SpikeDataSet::loadFromFile: first line = " << line << std::endl;
					const std::vector<std::string> content1 = ::tools::file::split(line, ' ');
					if (content1.size() < 3)
					{
						std::cerr << "spike::v3::SpikeDataSet:loadFromFile: got less than 3 items at line " << line << std::endl;
						throw std::exception();
					}
					const size_t nCases = static_cast<size_t>(::tools::file::string2int(content1[0]));
					const size_t nNeurons = static_cast<size_t>(::tools::file::string2int(content1[1]));
					const size_t nSpikes = static_cast<size_t>(::tools::file::string2int(content1[2]));

					this->init(nCases, nNeurons);

					//3] load the case data
					for (size_t i = 0; i < nCases; ++i)
					{
						::tools::file::loadNextLine(inputFileStream, line);
						const std::vector<std::string> content2 = ::tools::file::split(line, ' ');
						if (content2.size() < 3)
						{
							std::cerr << "spike::v3::SpikeDataSet:loadFromFile: got less than 3 items at line " << line << std::endl;
							throw std::exception();
						}
						const CaseId caseId = CaseId(static_cast<CaseIdType>(::tools::file::string2int(content2[0])));
						const TimeInMs caseDurationInMs = static_cast<TimeInMs>(::tools::file::string2float(content2[1]));
						const CaseLabel caseLabel = CaseLabel(static_cast<CaseLabelType>(::tools::file::string2int(content2[2])));
						//std::cout << "spike::v3::SpikeDataSet:loadFromFile: caseId=" << caseId << "; caseDurationInMs=" << caseDurationInMs << "; caseLabel=" << caseLabel << std::endl;

						this->setClassificationLabel(caseId, caseLabel);
						this->setCaseDuration(caseId, caseDurationInMs);
					}

					this->init(nNeurons, nCases);

					//4] handle neuron data
					for (size_t i = 0; i < (nNeurons * nCases); ++i)
					{
						::tools::file::loadNextLine(inputFileStream, line);
						const std::vector<std::string> content3 = ::tools::file::split(line, ' ');
						if (content3.size() < 3)
						{
							std::cerr << "spike::v3::SpikeDataSet:loadFromFile: got less than 3 items at line " << line << std::endl;
							throw std::exception();
						}
						const CaseId caseId = CaseId(static_cast<CaseIdType>(::tools::file::string2int(content3[0])));
						const NeuronId neuronId = static_cast<NeuronId>(::tools::file::string2int(content3[1]));
						const float randomHz = ::tools::file::string2float(content3[2]);
						this->setRandomHz(caseId, neuronId, randomHz);
					}

					//5] handle the spike data
					for (size_t i = 0; i < nSpikes; ++i)
					{
						::tools::file::loadNextLine(inputFileStream, line);
						const std::vector<std::string> content4 = ::tools::file::split(line, ' ');
						if (content4.size() < 3)
						{
							std::cerr << "spike::v3::SpikeDataSet:loadFromFile: got less than 3 items at line " << line << std::endl;
							throw std::exception();
						}
						const CaseId caseId = static_cast<CaseId>(static_cast<CaseIdType>(::tools::file::string2int(content4[0])));
						const NeuronId neuronId = static_cast<NeuronId>(::tools::file::string2int(content4[1]));
						const TimeInMs timeInMs = static_cast<TimeInMs>(::tools::file::string2float(content4[2]));
						this->addSpikeTime(caseId, neuronId, timeInMs);
					}
				}
			}

		private:

			bool isInitialized_;
			std::vector<TimeInMs> caseDuration_;
			std::vector<NeuronId> neuronIds_;
			std::vector<CaseId> caseIds_;

			std::map<NeuronId, unsigned int> neuronIdPosition_;
			std::map<CaseId, std::map<NeuronId, double>> randomSpikeHz_;
			std::map<CaseId, CaseLabelType> classification_;

			std::vector<std::array<std::vector<TimeInMs>, Options::nNeurons>> data_;

		};
	}
}