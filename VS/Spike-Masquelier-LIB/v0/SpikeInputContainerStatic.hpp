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
#include <deque>
#include <memory>
#include <algorithm> // for std::min
#include <iostream> // for cerr and cout

#include "../../Spike-Masquelier-LIB/v0/SpikeOptionsMasq.hpp"
#include "../../Spike-Masquelier-LIB/v0/SpikeTools.hpp"

namespace spike
{
	namespace v0
	{
		class SpikeInputContainerStatic
		{
		public:

			~SpikeInputContainerStatic() = default;

			SpikeInputContainerStatic() = delete;

			SpikeInputContainerStatic(
				const std::shared_ptr<const SpikeOptionsMasq>& options)
				: options_(options)
				, pos_(0)
				, length_(0)
				, lastPos_(0)
			{
			}

			void setInputFilename(const std::string& inputFilename)
			{
				this->inputFilename_ = inputFilename;
			}

			void load(const unsigned int numberOfInputEvents)
			{
				//	std::cout << "SpikeInputContainer::load() ================================" << std::endl;
				std::cout << "SpikeInputContainerStatic::load() setting input file = " << this->inputFilename_ << std::endl;

				std::string line;
				std::ifstream inputFile(this->inputFilename_);
				if (inputFile.is_open())
				{

					// get the number of lines in the input file
					unsigned int counter = 0;
					while (inputFile.good())
					{
						std::getline(inputFile, line);
						counter++;
					}

					this->length_ = std::min(numberOfInputEvents, counter);
					this->spikeEvent_.resize(this->length_);

					// reset the input file
					inputFile.clear();
					inputFile.seekg(0, inputFile.beg);

					// load the content of the input file
					unsigned int index = 0;
					for (unsigned int i = 0; i < this->length_; ++i)
					{
						if (inputFile.good())
						{
							std::getline(inputFile, line);
							if (line.size() > 1)
							{
								std::vector<std::string> content = split(line, ' ');

								if (content.size() == 2)
								{
									const float timeInMs = string2float(content[0]) * 1000;
									const SpikeTime time = static_cast<SpikeTime>(std::lroundf(timeInMs * SpikeOptionsMasq::TIME_DENOMINATOR));
									const SpikeNeuronId afferent = static_cast<SpikeNeuronId>(string2int(content[1]) - 1);
									//std::cout << time << " " << afferent << std::endl;

									if (afferent > 0000)
									{
										this->spikeEvent_[index] = makeSpikeEvent(time, afferent);
										index++;
									}
								}
								else
								{
									std::cerr << "SpikeInputContainerStatic::load() line " << line << " has incorrect content" << std::endl;
								}
							}
						}
						else
						{
							std::cout << "SpikeInputContainerStatic::load(): something went wrong" << std::endl;
						}
					}
					this->lastPos_ = index;
				}
				else
				{
					std::cout << "SpikeInputContainerStatic::load(): Unable to open file" << std::endl;
				}
				this->maxTimeInMs_ = (float)getTimeFromSpikeEvent(this->spikeEvent_[this->lastPos_ - 1]) / SpikeOptionsMasq::TIME_DENOMINATOR;
				std::cout << "SpikeInputContainerStatic::load() maxTimeInMs = " << this->maxTimeInMs_ << " ms" << std::endl;

				inputFile.close();

				//	std::cout << "SpikeInputContainerStatic::load() ================================" << std::endl;
			}

			void saveToFile(
				const std::string& /*filename*/) const
			{
				// see SpikeInputContainerStatic::saveToFile
			}

			SpikeEvent removeNextEvent()
			{
				const SpikeEvent nextEvent = this->spikeEvent_[this->pos_];
				this->pos_++;
				return nextEvent;
			}

			SpikeEvent getNextEvent()
			{
				if (this->isEmpty())
				{
					return this->spikeEvent_[this->lastPos_ - 1];
				}
				else
				{
					return this->spikeEvent_[this->pos_];
				}
			}

			bool isEmpty()
			{
				return (this->pos_ >= this->lastPos_);
			}

			float getMaxTimeInMs() const
			{
				return this->maxTimeInMs_;
			}

		private:

			std::shared_ptr<const SpikeOptionsMasq> options_;
			float maxTimeInMs_;

			unsigned int pos_;
			unsigned int lastPos_;
			unsigned int length_;
			std::vector<SpikeEvent> spikeEvent_;

			std::string inputFilename_;

			static std::vector<std::string> split(
				const std::string& s,
				const char delim)
			{
				std::vector<std::string> elems;
				std::stringstream ss(s);
				std::string item;
				while (std::getline(ss, item, delim))
				{
					elems.push_back(item);
				}
				return elems;
			}
			static float string2float(const std::string& s)
			{
				return static_cast<float>(atof((char*)s.c_str()));
			}
			static int string2int(const std::string& s)
			{
				return static_cast<int>(atof((char*)s.c_str()));
			}
		};
	}
}