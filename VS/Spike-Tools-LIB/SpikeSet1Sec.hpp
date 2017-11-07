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
#include <array>
#include <valarray>

#include "SpikeTypes.hpp"

#include "../Spike-Tools-LIB/file.ipp"

namespace spike
{
	namespace tools
	{
		// SpikeData1Sec is a container for 1 second of spikes
		template <typename Time = Ms>
		class SpikeSet1Sec
		{
		public:

			//std::atomic<unsigned int> nFirings_; // the number of fired neurons 
			unsigned int nFirings_;

			// the following three fields are public such that Spike_Network_State can reused these fields
			std::vector<NeuronId> firingNeuronId_;
			std::vector<Time> firingTime_;
			std::vector<FiringReason> firingReason_;

			// destructor
			~SpikeSet1Sec() = default;

			// constructor
			SpikeSet1Sec()
				: nFirings_(0)
				, firingNeuronId_(std::vector<NeuronId>(10000))
				, firingTime_(std::vector<Time>(10000))
				, firingReason_(std::vector<FiringReason>(10000))
				, second_(0)
				, frozen_(false)
			{
			}

			SpikeSet1Sec& operator=(const SpikeSet1Sec&) = default;
			SpikeSet1Sec(const SpikeSet1Sec&) = default;

			void clear()
			{
				this->frozen_ = false;
				this->nFirings_ = 0;
				this->second_ = 0;
				this->caseOccurances_.clear();
				this->timePosBegin_.clear();
				this->timePosEnd_.clear();
			}

			void freeze()
			{
				this->frozen_ = true;
				this->initTimePosData();
			}

			void addFiring(const Time time, const NeuronId neuronId, const FiringReason firingReason)
			{
				//static std::mutex mutex_data;
				//std::lock_guard<std::mutex> lock(mutex_data);

				/* // following code checks if spikes are ordered in time
#if _DEBUG
				if (this->nFirings_ > 0) {
					if (time < this->firingTime_[this->nFirings_ - 1]) {
						std::cerr << "SpikeData1Sec::addFiring(): WARNING: provided firings has time " << time << " which is before the prevous firing time " << this->firingTime_[this->nFirings_ - 1] << std::endl;
						throw 1;
					}
				}
#endif
				*/
				if (this->nFirings_ >= this->firingNeuronId_.size())
				{
					this->firingTime_.resize(this->nFirings_ + 10000);
					this->firingNeuronId_.resize(this->nFirings_ + 10000);
					this->firingReason_.resize(this->nFirings_ + 10000);
				}
				else
				{
					this->firingTime_[this->nFirings_] = time;
					this->firingNeuronId_[this->nFirings_] = neuronId;
					this->firingReason_[this->nFirings_] = firingReason;
					this->nFirings_++;
				}
			}

			void addCaseOccurance(const CaseOccurance<Time>& caseOccurance)
			{
				this->caseOccurances_.push_back(caseOccurance);
			}

			void addCaseOccurances(const std::vector<CaseOccurance<Time>>& caseOccurances)
			{
				for (const CaseOccurance<Time>& caseOccurance : caseOccurances)
				{
					this->addCaseOccurance(caseOccurance);
				}
			}

			void setTimeSecond(const int second)
			{
				this->second_ = second;
			}

			void saveToFile(const std::string& filename) const
			{
				// lock mutex before accessing file
				//static std::mutex mutex_data;
				//std::lock_guard<std::mutex> lock(mutex_data);

				// create the directory
				const std::string tree = ::tools::file::getDirectory(filename);
				if (!::tools::file::mkdirTree(tree))
				{
					std::cerr << "SpikeData1Sec::saveToFile(): Unable to create directory " << tree << std::endl;
					throw std::runtime_error("unable to create directory");
				}

				FILE * const fs = fopen(filename.c_str(), "w");
				if (fs == nullptr)
				{
					std::cerr << "SpikeData1Sec::saveToFile: Error: could not write to file " << filename << std::endl;
					return;
				}
				fprintf(fs, "#spike <second> <nCases> <nSpikes>\n");
				fprintf(fs, "%d %u %u\n", this->second_, static_cast<unsigned int>(this->caseOccurances_.size()), this->nFirings_);


				fprintf(fs, "#caseOccurance <caseId> <startTimeInMs> <endTimeInMs> <caseLabel>\n");
				for (size_t i = 0; i < this->caseOccurances_.size(); ++i)
				{
					const CaseOccurance<Time> caseOccurance = this->caseOccurances_[i];
					if (std::is_integral<Time>::value)
					{
						fprintf(fs, "%d %zu %zu %d\n", caseOccurance.caseId_, static_cast<size_t>(caseOccurance.startTime_), static_cast<size_t>(caseOccurance.endTime_), caseOccurance.caseLabel_);
					}
					else if (std::is_floating_point<Time>::value)
					{
						fprintf(fs, "%d %f %f %d\n", caseOccurance.caseId_, static_cast<double>(caseOccurance.startTime_), static_cast<double>(caseOccurance.endTime_), caseOccurance.caseLabel_);
					}
					else
					{
						std::cerr << "SpikeData1Sec::loadFromFile(): unsupported Time type" << std::endl;
						//DEBUG_BREAK();
					}
				}

				fprintf(fs, "#spikeData <timeInMs> <neuronId> <firingReason> (NO_FIRE = 0, FIRE_PROPAGATED = 1, FIRE_RANDOM = 2, FIRE_CLAMPED = 3, FIRE_PROPAGATED_INCORRECT = 4, FIRE_PROPAGATED_CORRECT = 5)\n");
				for (unsigned int i = 0; i < this->nFirings_; ++i)
				{
					if (std::is_integral<Time>::value)
					{
						fprintf(fs, "%zu %u %d\n", static_cast<size_t>(this->firingTime_[i]), this->firingNeuronId_[i], static_cast<int>(this->firingReason_[i]));
					}
					else if (std::is_floating_point<Time>::value)
					{
						fprintf(fs, "%f %u %d\n", static_cast<double>(this->firingTime_[i]), this->firingNeuronId_[i], static_cast<int>(this->firingReason_[i]));
					}
					else
					{
						std::cerr << "SpikeData1Sec::loadFromFile(): unsupported Time type" << std::endl;
						//DEBUG_BREAK();
					}
				}
				fclose(fs);
			}

			void loadFromFile(const std::string& filename, const int second)
			{
				// lock mutex before accessing file
				//std::lock_guard<std::mutex> lock(mutex_data);

				std::stringstream fullFilenameStream;
				fullFilenameStream << filename << "." << second << ".txt";
				const std::string fullFilename = fullFilenameStream.str();
				std::ifstream inputFile(fullFilename);

				if (!inputFile.is_open())
				{
					std::cerr << "SpikeData1Sec::loadFromFile(): Unable to open file " << fullFilename << std::endl;
				}
				else
				{
					//	std::cout << "SpikeData1Sec::loadFromFile(): Opening file " << fullFilename << std::endl;

					this->clear();
					this->setTimeSecond(second);
					std::string line;

					//1] load the first content line
					int nFiringsLocal = 0;
					int nCaseOccurances = 0;
					if (::tools::loadNextLine(inputFile, line))
					{
						const std::vector<std::string> content = ::tools::file::split(line, ' ');
						if (content.size() >= 2)
						{
							this->second_ = ::tools::file::string2int(content[0]);
							nCaseOccurances = ::tools::file::string2int(content[1]);
							nFiringsLocal = ::tools::file::string2int(content[2]);
						}
						else
						{
							std::cerr << "SpikeData1Sec::loadFromFile(): ERROR A. line " << line << " has incorrect content" << std::endl;
						}
						if (this->second_ != second)
						{
							std::cerr << "SpikeData1Sec::loadFromFile() WARNING: loaded file " << fullFilename << ": the restored second did not match: second=" << this->second_ << std::endl;
						}
					}
					else
					{
						std::cerr << "SpikeData1Sec::loadFromFile(): ERROR A. file has too little content" << std::endl;
					}

					//2] load the case occurances
					for (int i = 0; i < nCaseOccurances; i++)
					{
						if (::tools::loadNextLine(inputFile, line))
						{
							const std::vector<std::string> content = ::tools::file::split(line, ' ');
							if (content.size() >= 4)
							{

								const CaseId caseId = CaseId(static_cast<CaseIdType>(::tools::file::string2int(content[0])));
								Time startTime;
								Time endTime;
								const CaseLabel caseLabel = CaseLabel(static_cast<CaseLabelType>(::tools::file::string2int(content[3])));

								if (std::is_integral<Time>::value)
								{
									startTime = static_cast<Time>(::tools::file::string2int(content[1]));
									endTime = static_cast<Time>(::tools::file::string2int(content[2]));
								}
								else if (std::is_floating_point<Time>::value)
								{
									startTime = static_cast<Time>(::tools::file::string2float(content[1]));
									endTime = static_cast<Time>(::tools::file::string2float(content[2]));
								}
								else
								{
									std::cerr << "SpikeData1Sec::loadFromFile(): unsupported Time type" << std::endl;
									//DEBUG_BREAK();
								}

								this->addCaseOccurance(CaseOccurance<Time>(caseId, startTime, endTime, caseLabel));
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

					//3] load the spike data
					for (int i = 0; i < nFiringsLocal; i++)
					{
						if (::tools::loadNextLine(inputFile, line))
						{
							const std::vector<std::string> content = ::tools::file::split(line, ' ');
							if (content.size() < 3)
							{
								std::cerr << "SpikeData1Sec::loadFromFile(): ERROR C. line " << line << " has incorrect content" << std::endl;
							}
							Time ms;
							const NeuronId neuronId = static_cast<NeuronId>(::tools::file::string2int(content[1]));
							const FiringReason firingReason = static_cast<FiringReason>(::tools::file::string2int(content[2]));

							if (std::is_integral<Time>::value)
							{
								ms = static_cast<Time>(::tools::file::string2int(content[0]));
							}
							else if (std::is_floating_point<Time>::value)
							{
								ms = static_cast<Time>(::tools::file::string2float(content[0]));
							}
							else
							{
								std::cerr << "SpikeData1Sec::loadFromFile(): unsupported Time type" << std::endl;
								//DEBUG_BREAK();
							}
							this->addFiring(ms, neuronId, firingReason);
						}
						else
						{
							std::cerr << "SpikeData1Sec::loadFromFile(): ERROR C. file has too little content" << std::endl;
						}
					}
				}
			}

			const std::vector<int> getTimePosBegin() const
			{
				if (!this->frozen_)
				{
					std::cerr << "SpikeData1Sec::getTimePosBegin() not frozen!" << std::endl;
					throw std::exception();
				}
				return this->timePosBegin_;
			}

			const std::vector<int> getTimePosEnd() const
			{
				if (!this->frozen_)
				{
					std::cerr << "SpikeData1Sec::getTimePosEnd() not frozen!" << std::endl;
					throw std::exception();
				}
				return this->timePosEnd_;
			}

			const std::vector<NeuronId> getFiringNeuronId() const
			{
				return this->firingNeuronId_;
			}

			const std::vector<Time> getFiringTime() const
			{
				return this->firingTime_;
			}

			unsigned int getNumberOfFirings() const
			{
				return this->nFirings_;
			}

			unsigned int getNumberOfFirings(const NeuronId neuronId) const
			{
				unsigned int count = 0;
				for (unsigned int i = 0; i < this->nFirings_; i++)
				{
					if (this->firingNeuronId_[i] == neuronId)
					{
						count++;
					}
				}
				return count;
			}

		private:

			int second_;
			bool frozen_;

			std::vector<CaseOccurance<Time>> caseOccurances_;
			std::vector<int> timePosBegin_;
			std::vector<int> timePosEnd_;

			void initTimePosData()
			{
				this->timePosBegin_.clear();
				this->timePosEnd_.clear();

				//std::cout << "initTimePosData: nFirings = " << this->nFirings_ << std::endl;
				if (this->nFirings_ > 0)
				{
					const Time lastTime = static_cast<Time>(this->firingTime_[this->nFirings_ - 1]);
					//	std::cout << "SpikeData1Sec::initTimePosData() firstTime=" << firstTime << "; lastTime=" << lastTime << std::endl;

					for (int time = 0; time < lastTime; ++time)
					{
						this->timePosBegin_.push_back(findBeginTimeIndex(time, this->firingTime_, this->nFirings_));
						this->timePosEnd_.push_back(findEndTimeIndex(time, this->firingTime_, this->nFirings_));
					}
				}
			}

			int findBeginTimeIndex(const int time, const std::vector<Time>& spikeTimeArray, const int spikeDataSize)
			{
				for (int i = 0; i < spikeDataSize; ++i)
				{
					if (spikeTimeArray[i] >= time)
					{
						return i;
					}
				}
				return spikeDataSize;
			}

			int findEndTimeIndex(const int time, const std::vector<Time>& spikeTimeArray, const int spikeDataSize)
			{
				for (int i = spikeDataSize - 1; i >= 0; --i)
				{
					if (spikeTimeArray[i] <= time)
					{
						return i;
					}
				}
				return 0;
			}
		};
	}
}