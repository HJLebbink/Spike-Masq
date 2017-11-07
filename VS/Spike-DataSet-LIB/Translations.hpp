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
#include <string>
#include <map>
#include <set>

#include "SpikeSetLarge.hpp"
#include "DataSetState.hpp"

namespace spike
{
	namespace dataset
	{
		template <class D>
		class Translations
		{
		public:

			using TimeType = TimeInMsI;

			Translations() = default;
			~Translations() = default;

			void setTranslation(
				const VariableId variableId,
				const D value,
				const std::shared_ptr<SpikeSetLarge>& spikeSet)
			{
				if (this->data_.empty())
				{
					this->durationInMs_ = spikeSet->getDurationInMs();
				}
				else if (spikeSet->getDurationInMs() != this->durationInMs_)
				{
					std::cerr << "Translation::setTranslation: unequal durations" << std::endl;
					throw new std::exception();
				}
				if (!this->hasVariableId(variableId))
				{
					std::map<D, std::shared_ptr<SpikeSetLarge>> emptyMap;
					this->data_.insert(std::make_pair(variableId, std::move(emptyMap)));
				}
				this->data_.at(variableId).insert(std::make_pair(value, spikeSet));
			}

			void clear()
			{
				this->data_.clear();
			}

			const std::shared_ptr<SpikeSetLarge>getTranslation(
				const VariableId variableId,
				const D value) const
			{
				if (this->hasVariableId(variableId))
				{
					return this->data_.at(variableId).at(value);
				}
				throw std::runtime_error("variableId not present");
			}

			bool hasTranslation(
				const VariableId variableId,
				const D value) const
			{
				if (this->hasVariableId(variableId))
				{
					const auto variableData = this->data_.at(variableId);
					if (variableData.find(value) != variableData.end())
					{
						return true;
					}
				}
				return false;
			}

			const std::set<VariableId> getVariableIds() const
			{
				std::set<VariableId> s;
				for (auto it = this->data_.cbegin(); it != this->data_.cend(); ++it)
				{
					s.insert(it->first);
				}
				return s;
			}

			std::set<NeuronId> getNeuronIds() const
			{
				std::set<NeuronId> s;
				for (auto it1 = this->data_.cbegin(); it1 != this->data_.cend(); ++it1)
				{
					const auto map = it1->second;

					for (auto it2 = map.cbegin(); it2 != map.cend(); ++it2)
					{
						const auto spikeSet = it2->second;
						for (const NeuronId& neuronId : spikeSet->getNeuronIds())
						{
							s.insert(neuronId);
						}
					}
				}
				//std::cout << "Translations<D>::getAllPositions: returning " << s.size() << " positions" << std::endl;
				return s;
			}

			TimeType getDurationInMs() const
			{
				return this->durationInMs_;
			}

			std::string toString() const
			{
				std::ostringstream oss;

				for (const VariableId& variableId : this->getVariableIds())
				{
					for (const D& value : this->getUniqueValues(variableId))
					{
						const auto spikeSet = this->getTranslation(variableId, value);

						oss << "variableId=" << variableId << "; value=" << value << "; maxTimeInMs=" << spikeSet->getMaxTimeInMs() << "; nSpikes=" << spikeSet->getNumberOfSpikes() << std::endl;
						oss << spikeSet->toString() << std::endl;
					}
				}
				return oss.str();
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
					std::cerr << "Translations::saveToFile(): Unable to create directory " << tree << std::endl;
					throw std::runtime_error("unable to create directory");
				}

				std::ofstream outputFile(filename);
				if (!outputFile.is_open())
				{
					std::cerr << "Translations::saveToFile(): Unable to open file " << filename << std::endl;
				}
				else
				{
					std::cout << "Translations::saveToFile(): Opening file " << filename << std::endl;

					const std::set<VariableId> variableIds = this->getVariableIds();

					unsigned int nTranslationPairs = 0;
					for (const VariableId& variableId : variableIds)
					{
						nTranslationPairs += static_cast<unsigned int>(this->getUniqueValues(variableId).size());
					}

					outputFile << "#translation <nTranslationPairs>" << std::endl;
					outputFile << nTranslationPairs << std::endl;


					//2] print the number of cases and the number of variables
					for (const VariableId& variableId : variableIds)
					{

						for (const D& value : this->getUniqueValues(variableId))
						{
							const auto spikeSet = this->getTranslation(variableId, value);
							const TimeType durationInMs = spikeSet->getDurationInMs();
							const unsigned int nSpikes = spikeSet->getNumberOfSpikes();
							const std::set<NeuronId> neuronIds = spikeSet->getNeuronIds();

							//1] output the variable data
							outputFile << "#translationPair <variableId> <value> <nNeurons> <nSpikes> <durationInMs>" << std::endl;
							outputFile << variableId << " " << value << " " << neuronIds.size() << " " << nSpikes << " " << durationInMs << std::endl;

							//2] output the neurons used in this variable-value pair
							outputFile << "#neurons <neuronId>" << std::endl;
							for (const NeuronId& neuronId : neuronIds)
							{
								outputFile << neuronId << std::endl;
							}

							//3] output the spike times
							outputFile << "#spikeData <neuronId> <time>" << std::endl;
							for (TimeType time = 0; time < durationInMs; time++)
							{
								for (const NeuronId& neuronId : neuronIds)
								{
									if (spikeSet->getSpike(neuronId, time))
									{
										outputFile << neuronId << " " << time << std::endl;
									}
								}
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
					std::cerr << "Translations::loadFromFile(): Unable to open file " << filename << std::endl;
					throw std::exception();
				}
				this->clear();

				std::cout << "Translations::loadFromFile(): Opening file " << filename << std::endl;

				//1] load the number of translation pairs in this file
				if (!::tools::file::loadNextLine(inputFileStream, line))
				{
					std::cerr << "Translations::loadFromFile(): first line " << line << " has incorrect content" << std::endl;
					throw std::exception();
				}
				//std::cout << "loadFromFile() first line = " << line << std::endl;
				const std::vector<std::string> content = ::tools::file::split(line, ' ');
				if (content.size() < 1)
				{
					std::cerr << "SpikeDataSet::loadFromFile: got less than 1 item at line " << line << std::endl;
					throw std::exception();
				}

				const int nTranslationPairs = ::tools::file::string2int(content[0]);
				for (int translationPair = 0; translationPair < nTranslationPairs; ++translationPair)
				{

					//2] load the translationPair data 
					if (!::tools::file::loadNextLine(inputFileStream, line))
					{
						std::cerr << "Translations::loadFromFile(): line " << line << " has incorrect content" << std::endl;
						throw std::exception();
					}
					//std::cout << "loadFromFile() first line = " << line << std::endl;
					const std::vector<std::string> content2 = ::tools::file::split(line, ' ');
					if (content2.size() < 5)
					{
						std::cerr << "SpikeDataSet::loadFromFile: got less than 5 items at line " << line << std::endl;
						throw std::exception();
					}
					const VariableId variableId = static_cast<VariableId>(::tools::file::string2int(content2[0]));
					const D value = ::tools::file::string2int(content2[1]);
					const unsigned int nNeurons = static_cast<unsigned int>(::tools::file::string2int(content2[2]));
					const unsigned int nSpikes = static_cast<unsigned int>(::tools::file::string2int(content2[3]));
					const TimeType durationInMs = static_cast<TimeType>(::tools::file::string2int(content2[4]));

					//3] load the neuronIds
					std::vector<NeuronId> neuronIds;
					for (unsigned int i = 0; i < nNeurons; ++i)
					{

						if (!::tools::file::loadNextLine(inputFileStream, line))
						{
							std::cerr << "TransDataSet::loadFromFile(): line " << line << " has incorrect content" << std::endl;
							throw std::exception();
						}
						const std::vector<std::string> content3 = ::tools::file::split(line, ' ');
						if (content3.size() < 1)
						{
							std::cerr << "SpikeDataSet::loadFromFile: got less than 1 item at line " << line << std::endl;
							throw std::exception();
						}
						const NeuronId neuronId = static_cast<NeuronId>(::tools::file::string2int(content3[0]));
						neuronIds.push_back(neuronId);
					}

					//4] create new spikeset
					auto spikeSet = std::make_shared<SpikeSetLarge>(neuronIds, durationInMs);

					//4] fill the spikeset
					for (unsigned int i = 0; i < nSpikes; ++i)
					{

						if (!::tools::file::loadNextLine(inputFileStream, line))
						{
							std::cerr << "TransDataSet::loadFromFile(): line " << line << " has incorrect content" << std::endl;
							throw std::exception();
						}
						const std::vector<std::string> content4 = ::tools::file::split(line, ' ');
						if (content4.size() < 2)
						{
							std::cerr << "SpikeDataSet::loadFromFile: got less than 2 item at line " << line << std::endl;
							throw std::exception();
						}
						const NeuronId neuronId = static_cast<NeuronId>(::tools::file::string2int(content4[0]));
						const TimeType timeInMs = static_cast<TimeType>(::tools::file::string2int(content4[1]));
						spikeSet->setSpike(neuronId, timeInMs);
					}
					this->setTranslation(variableId, value, spikeSet);
				}
			}

		private:

			TimeType durationInMs_;
			std::map<VariableId, std::map<D, std::shared_ptr<SpikeSetLarge>>> data_;

			bool hasVariableId(const VariableId variableId) const
			{
				return (this->data_.find(variableId) != this->data_.cend());
			}

			const std::set<D> getUniqueValues(const VariableId variableId) const
			{
				if (!this->hasVariableId(variableId))
				{
					std::cerr << "Translations<D>::getUniqueValues: variableId " << variableId << " not present" << std::endl;
					throw std::runtime_error("variableId not present");
				}
				std::set<D> s;
				const auto variableData = this->data_.at(variableId);
				//	for (std::map<D, std::shared_ptr<SpikeSet<D>>>::const_iterator it = variableData.cbegin(); it != variableData.cend(); ++it) {
				for (auto it = variableData.cbegin(); it != variableData.cend(); ++it)
				{
					s.insert(it->first);
				}
				return s;
			}
		};
	}
}