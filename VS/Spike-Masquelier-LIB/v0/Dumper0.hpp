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
#include <unordered_map>
#include <fstream>
#include <iostream> // for cerr and cout
#include <memory>

#include "../../Spike-Tools-LIB/SpikeRuntimeOptions.hpp"

#include "../../Spike-Masquelier-LIB/v0/SpikeOptionsMasq.hpp"
#include "../../Spike-Masquelier-LIB/v0/SpikeTools.hpp"

namespace spike
{
	namespace v0
	{
		using namespace spike::tools;

		class Dumper
		{
		public:

			Dumper(
				const SpikeOptionsMasq& options,
				const SpikeRuntimeOptions& spikeOptions)
				: options_(options)
				, spikeOptions_(spikeOptions)
				, sec_(0)
			{
			}

			// copy constructor
			Dumper(const Dumper& other) = delete;

			~Dumper()
			{
				this->close();
			}


			// copy assignment
			Dumper& operator= (const Dumper&) = delete;


			void setWeightOutputFilename(const std::string& weightOutputFilename)
			{
				this->weightOutputFile_ = std::ofstream(weightOutputFilename, std::ofstream::out);// , std::ofstream::out);
				if (!this->weightOutputFile_.is_open())
				{
					std::cerr << "Dumper::setWeightOutputFilename(): could not open file " << weightOutputFilename << std::endl;
				}
			}

			void setPotentialOutputFilename(const std::string& potentialOutputFilename)
			{
				this->potentialOutputFile_ = std::ofstream(potentialOutputFilename, std::ofstream::out);
				if (!this->potentialOutputFile_.is_open())
				{
					std::cerr << "SpikeDumper::setPotentialOutputFilename(): could not open file " << potentialOutputFilename << std::endl;
				}
			}

			void dumpPotential(
				const SpikeNeuronId neuronId,
				const SpikeTime time,
				const int potential)
			{
				if (time == 0) return;
				const float timeInMs = static_cast<float>(time) / SpikeOptionsMasq::TIME_DENOMINATOR;
				const float potentialFloat = (float)potential / SpikeOptionsMasq::POTENTIAL_DENOMINATOR;

				//	if (this->lastUpdatedTimePotential[neuronId] != nullptr) {
				//	std::cout << "SpikeDumper::dumpPotential(): A lastUpdatedTimePotential[neuronId] = " << lastUpdatedTimePotential[neuronId] << "; current time "<< timeInMs << std::endl;
				if ((this->lastUpdatedTimePotential_[neuronId]) > (timeInMs - 0.1f))
				{
					//		std::cout << "SpikeDumper::dumpPotential(): B lastUpdatedTimePotential[neuronId] = " << lastUpdatedTimePotential[neuronId] << "; current time "<< timeInMs << std::endl;
					return;
				}
				//	}

				this->lastUpdatedTimePotential_[neuronId] = timeInMs;
				if (this->potentialOutputFile_.is_open())
				{
					this->potentialOutputFile_ << timeInMs << " " << neuronId << " " << potentialFloat << std::endl;
				}
				else
				{
					std::cerr << "SpikeDumper::dumpPotential(): this->potentialOutputFile is not open" << std::endl;
				}
			}

			void dumpWeight(
				const SpikeNeuronId neuronId,
				const SpikeNeuronId presynapticNeuronId,
				const SpikeTime time,
				const Weight weight)
			{
				if (time == 0) return;
				const float timeInMs = (float)time / SpikeOptionsMasq::TIME_DENOMINATOR;
				const float weightFloat = (float)weight / SpikeOptionsMasq::POTENTIAL_DENOMINATOR;

				if (this->lastUpdatedTimeWeight_[neuronId] != NULL)
				{ //TODO: comparing a float with NULL, this must be a bug!
//	std::cout << "SpikeDumper::dumpPotential(): A lastUpdatedTimeWeight[neuronId] = " << lastUpdatedTimeWeight[neuronId] << "; current time "<< timeInMs << std::endl;
					if ((this->lastUpdatedTimeWeight_[neuronId]) > (timeInMs - 0.1f))
					{
						//std::cout << "SpikeDumper::dumpPotential(): B lastUpdatedTimeWeight[neuronId] = " << lastUpdatedTimeWeight[neuronId] << "; current time "<< timeInMs << std::endl;
						return;
					}
				}

				this->lastUpdatedTimeWeight_[neuronId] = timeInMs;
				if (this->weightOutputFile_.is_open())
				{
					this->weightOutputFile_ << timeInMs << " " << neuronId << " " << presynapticNeuronId << " " << weightFloat << std::endl;
				}
				else
				{
					std::cerr << "SpikeDumper::dumpWeight(): this->weightOutputFile is not open" << std::endl;
				}
			}

			void close()
			{
				this->weightOutputFile_.close();
				this->potentialOutputFile_.close();
			}

		private:

			std::string spikeOutputFilenamePrefix_;
			unsigned int sec_;

			SpikeOptionsMasq options_;
			SpikeRuntimeOptions spikeOptions_;

			std::ofstream weightOutputFile_;
			std::ofstream potentialOutputFile_;

			using hashmap = std::unordered_map < SpikeNeuronId, float >;
			hashmap lastUpdatedTimePotential_;
			hashmap lastUpdatedTimeWeight_;
		};
	}
}