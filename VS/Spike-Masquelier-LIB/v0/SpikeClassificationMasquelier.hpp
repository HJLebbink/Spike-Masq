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
#include <iostream> // for cerr and cout
#include <fstream>		// std::ifstream

#include "../../Spike-Masquelier-LIB/v0/SpikeOptionsMasq.hpp"
#include "../../Spike-Masquelier-LIB/v0/SpikeTools.hpp"

#include "../../Spike-Tools-LIB/file.ipp"

namespace spike
{
	namespace v0
	{
		class SpikeClassificationMasquelier
		{
		public:

			SpikeClassificationMasquelier() = default;
			~SpikeClassificationMasquelier() = default;

			SpikeClassificationMasquelier(const SpikeClassificationMasquelier& other) = default;

			void load(const std::string& inputFilename)
			{
				std::cout << "INFO: SpikeClassificationMasquelier::load ================================" << std::endl;
				std::cout << "INFO: SpikeClassificationMasquelier::load: setting input file = " << inputFilename << std::endl;

				std::string line;
				//	std::ios::sync_with_stdio(true);
				std::ifstream inputFile = std::ifstream(inputFilename);
				if (inputFile.is_open())
				{

					std::getline(inputFile, line);
					this->patternDurationInMs_ = parsePatternDuration(line);
					std::cout << "INFO: SpikeClassificationMasquelier::load: patternDurationInMs = " << this->patternDurationInMs_ << " ms" << std::endl;
					std::getline(inputFile, line);
					this->totalTimeInMs_ = parseTotalTime(line);
					//this->t_max = (int) this->t_max_inMs * options->time_denominator;
					std::cout << "INFO: SpikeClassificationMasquelier::load: totalTimeInMs = " << this->totalTimeInMs_ << " ms" << std::endl;

					this->fillBuffer(inputFile);
				}
				else
				{
					std::cout << "ERROR: Unable to open file" << std::endl;
					throw 12;
				}
				std::cout << "INFO: SpikeClassificationMasquelier::load ================================" << std::endl;
			}

			float latency(const float spikeTimeInMs, const CaseId caseId) const
			{
				#if _DEBUG
				if (this->patternDurationInMs_ <= 0) std::cerr << "SpikeClassificationMasquelier::latency patternDurationInMs_ is too small" << std::endl;
				#endif
				std::vector<float> patternStartTimes;
				switch (caseId.val)
				{
					case 1:
						patternStartTimes = this->pattern1StartTimes_;
						break;
					case 2:
						patternStartTimes = this->pattern2StartTimes_;
						break;
					case 3:
						patternStartTimes = this->pattern3StartTimes_;
						break;
					default:
						std::cerr << "SpikeClassificationMasquelier::latency() something went wrong" << std::endl;
				}

				for (size_t i = 0; i < patternStartTimes.size(); i++)
				{
					const float startTime = patternStartTimes[i];
					//std::cout<<"latency() startTime="<<startTime << std::endl;
					if (spikeTimeInMs > startTime)
					{
						if (spikeTimeInMs < (startTime + this->patternDurationInMs_))
						{
							return spikeTimeInMs - startTime;
						}
						else
						{
						}
					}
					else
					{
						return 0;
					}
				}
				return 0;
			}

			const std::vector<CaseId> getCaseIds() const
			{
				std::vector<CaseId> v(3);
				v[0] = CaseId(1);
				v[1] = CaseId(2);
				v[2] = CaseId(3);
				return v;
			}

		private:

			float patternDurationInMs_;
			float totalTimeInMs_;

			std::vector<float> pattern1StartTimes_;
			std::vector<float> pattern2StartTimes_;
			std::vector<float> pattern3StartTimes_;

			void fillBuffer(std::ifstream& inputFile)
			{
				//	std::cout << "SpikeInputContainer::fillBuffer()" << std::endl;
				std::string line;
				while (inputFile.good())
				{
					std::getline(inputFile, line);
					if (line.size() > 1)
					{
						std::vector<std::string> content = ::tools::file::split(line, ' ');

						if (content.size() == 2)
						{
							const float timeInMs = ::tools::file::string2float(content[0]) * 1000;
							const int patternId = ::tools::file::string2int(content[1]);
							//			std::cout << timeInMs << " " << patternId << std::endl;

							if (patternId == 1)
							{
								this->pattern1StartTimes_.push_back(timeInMs);
							}
							else if (patternId == 2)
							{
								this->pattern2StartTimes_.push_back(timeInMs);
							}
							else if (patternId == 3)
							{
								this->pattern3StartTimes_.push_back(timeInMs);
							}
							else
							{
								std::cerr << "SpikeClassification::fillBuffer() something went wrong" << std::endl;
							}
						}
						else
						{
							std::cerr << "SpikeClassification::fillBuffer() line " << line << " has incorrect content" << std::endl;
						}
					}
				}
			}

			static float parsePatternDuration(const std::string& s)
			{
				const std::string label = "patternDuration";
				std::vector<std::string> content = ::tools::file::split(s, '=');
				if (content[0] == label)
				{
					const float returnValue = ::tools::file::string2float(content[1]) * 1000;
					//std::cout<< "parsePatternDuration(): returnValue="<<returnValue<<"; content[1]="<<content[1]<<std::endl;
					return returnValue;
				}
				std::cerr << "parsePatternDuration(strLine=" << s << "): expected " << label << std::endl;
				return 0;
			}

			static float parseTotalTime(const std::string& s)
			{
				const std::string label = "totalTime";
				std::vector<std::string> content = ::tools::file::split(s, '=');
				if (content[0] == label)
				{
					const float returnValue = ::tools::file::string2float(content[1]);
					//std::cout<< "parseTotalTime(): returnValue="<<returnValue<<"; content[1]="<<content[1]<<std::endl;
					return returnValue;
				}
				std::cerr << "parseTotalTime(strLine=" << s << "): expected " << label << std::endl;
				return 0;
			}
		};
	}
}