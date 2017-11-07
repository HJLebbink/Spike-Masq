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

#include "../Spike-DataSet-LIB/Options.hpp"
#include "SpikeTypes.hpp"

namespace spike
{
	namespace tools
	{
		class SpikeRuntimeOptions
			: public spike::dataset::Options<unsigned int>
		{
		public:

			// destructor
			~SpikeRuntimeOptions() = default;

			// constructor
			SpikeRuntimeOptions(const SpikeRuntimeOptions& other) = default;

			// constructor
			SpikeRuntimeOptions()
			{
				this->timeRoundingIntervalInMsPresent_ = false;
				this->randomCaseDurationInMsPresent_ = false;
				this->caseDurationInMsPresent_ = false;
				this->caseTailSilenceInMsPresent_ = false;
				this->randomSpikeHzPresent_ = false;
				this->refractoryPeriodInMsPresent_ = false;
				this->nSamplesPresent_ = false;
				this->startUpTimeInMsPresent_ = false;
				this->sampleTimeInMsPresent_ = false;

				this->dumpToFileOn_Spikes_ = false;
				this->dumpToFileOn_Topology_ = false;
				this->dumpToFileOn_State_ = false;
				this->dumpToFileOn_WeightDelta_ = false;
				this->dumpToFileOn_Group_ = false;

				this->dumpIntervalInSec_Spikes_ = 1 * 1 * 60;
				this->dumpIntervalInSec_Topology_ = 1 * 60 * 60;
				this->dumpIntervalInSec_State_ = 1 * 60 * 60;
				this->dumpIntervalInSec_WeightDelta_ = 1 * 60 * 60;
				this->dumpIntervalInSec_Group_ = 1 * 60 * 60;

				this->filenamePath_Topology_ = "C:/Temp/Spike/Izhikevich/Topology";
				this->filenamePath_Spikes_ = "C:/Temp/Spike/Izhikevich/Spikes";
				this->filenamePath_State_ = "C:/Temp/Spike/Izhikevich/State";
				this->filenamePath_WeightDelta_ = "C:/Temp/Spike/Izhikevich/WeightDelta";
				this->filenamePath_Group_ = "C:/Temp/Spike/Izhikevich/Group";

				this->filenamePrefix_Topology_ = "train";
				this->filenamePrefix_Spikes_ = "train";
				this->filenamePrefix_State_ = "train";
				this->filenamePrefix_WeightDelta_ = "train";
				this->filenamePrefix_Group_ = "train";
			}

			void setNumberOfSamples(const unsigned int value)
			{
				this->nSamples_ = value;
				this->nSamplesPresent_ = true;
			}

			unsigned int getNumberOfSamples() const
			{
				if (this->nSamplesPresent_)
				{
					return this->nSamples_;
				}
				std::cerr << "spike::tools::SpikeRuntimeOptions::getNumberOfSamples: number of samples is not set" << std::endl;
				throw std::runtime_error("nSamples is not set");
			}

			void setStartUpTimeInMs(const Ms value)
			{
				this->startUpTimeInMs_ = value;
				this->startUpTimeInMsPresent_ = true;
			}

			Ms getStartUpTimeInMs() const
			{
				if (this->startUpTimeInMsPresent_)
				{
					return this->startUpTimeInMs_;
				}
				std::cerr << "spike::tools::SpikeRuntimeOptions::getStartUpTimeInMs: _startUpTimeInMs is not set" << std::endl;
				throw std::runtime_error("startUpTimeInMs is not set");
			}

			void setSampleTimeInMs(const Ms value)
			{
				this->sampleTimeInMs_ = value;
				this->sampleTimeInMsPresent_ = true;
			}

			Ms getSampleTimeInMs() const
			{
				if (this->sampleTimeInMsPresent_)
				{
					return this->sampleTimeInMs_;
				}
				std::cerr << "spike::tools::SpikeRuntimeOptions::getSampleTimeInMs: _sampleTimeInMs is not set" << std::endl;
				throw std::runtime_error("sampleTimeInMs is not set");
			}

			void setTimeRoundingIntervalInMs(const Ms value)
			{
				this->timeRoundingIntervalInMs_ = value;
				this->timeRoundingIntervalInMsPresent_ = true;
			}

			Ms getTimeRoundingIntervalInMs() const
			{
				if (this->timeRoundingIntervalInMsPresent_)
				{
					return this->timeRoundingIntervalInMs_;
				}
				std::cerr << "spike::tools::SpikeRuntimeOptions::getTimeRoundingIntervalInMs: timeRoundingIntervalInMs is not set" << std::endl;
				throw std::runtime_error("timeRoundingIntervalInMs is not set");
			}

			void setRandomCaseDurationInMs(const Ms value)
			{
				this->randomCaseDurationInMs_ = value;
				this->randomCaseDurationInMsPresent_ = true;
			}

			Ms getRandomCaseDurationInMs() const
			{
				if (this->randomCaseDurationInMsPresent_)
				{
					return this->randomCaseDurationInMs_;
				}
				std::cerr << "spike::tools::SpikeRuntimeOptions::getRandomCaseDurationInMs: randomCaseDurationInMs is not set" << std::endl;
				throw std::runtime_error("randomCaseDurationInMs is not set");
			}

			void setCaseDurationInMs(const Ms value)
			{
				this->caseDurationInMs_ = value;
				this->caseDurationInMsPresent_ = true;
			}

			Ms getCaseDurationInMs() const
			{
				if (this->caseDurationInMsPresent_)
				{
					return this->caseDurationInMs_;
				}
				std::cerr << "spike::tools::SpikeRuntimeOptions::getCaseDurationInMs: caseDurationInMs is not set" << std::endl;
				throw std::runtime_error("caseDurationInMs is not set");
			}

			void setCaseTailSilenceInMs(const Ms value)
			{
				this->caseTailSilenceInMs_ = value;
				this->caseTailSilenceInMsPresent_ = true;
			}

			Ms getCaseTailSilenceInMs() const
			{
				if (this->caseTailSilenceInMsPresent_)
				{
					return this->caseTailSilenceInMs_;
				}
				std::cerr << "spike::tools::SpikeRuntimeOptions::getCaseTailSilenceInMs: caseTailSilenceInMs is not set" << std::endl;
				throw std::runtime_error("caseTailSilenceInMs is not set");
			}

			void setRandomSpikeHz(const double value)
			{
				this->randomSpikeHz_ = value;
				this->randomSpikeHzPresent_ = true;
			}

			double getRandomSpikeHz() const
			{
				if (this->randomSpikeHzPresent_)
				{
					return this->randomSpikeHz_;
				}
				std::cerr << "spike::tools::SpikeRuntimeOptions::getRandomSpikeHz: randomSpikeHz is not set" << std::endl;
				throw std::runtime_error("randomSpikeHz is not set");
			}

			void setCorrectNeuronSpikeHz(const double value)
			{
				this->correctNeuronSpikeHz_ = value;
				this->correctNeuronSpikeHzPresent_ = true;
			}

			double getCorrectNeuronSpikeHz() const
			{
				if (this->correctNeuronSpikeHzPresent_)
				{
					return this->correctNeuronSpikeHz_;
				}
				std::cerr << "spike::tools::SpikeRuntimeOptions::getCorrectNeuronSpikeHz: correctNeuronSpikeHz is not set" << std::endl;
				throw std::runtime_error("correctNeuronSpikeHz is not set");
			}

			void setRefractoryPeriodInMs(const Ms value)
			{
				this->refractoryPeriodInMs_ = value;
				this->refractoryPeriodInMsPresent_ = true;
			}

			Ms getRefractoryPeriodInMs() const
			{
				if (this->refractoryPeriodInMsPresent_)
				{
					return this->refractoryPeriodInMs_;
				}
				std::cerr << "spike::tools::SpikeRuntimeOptions::getRefractoryPeriodInMs: _refractoryPeriodInMs is not set" << std::endl;
				throw std::runtime_error("refractoryPeriodInMs is not set");
			}

			bool isDumpToFileOn_Spikes()	const
			{
				return this->dumpToFileOn_Spikes_;
			}
			bool isDumpToFileOn_Topology()	const
			{
				return this->dumpToFileOn_Topology_;
			}
			bool isDumpToFileOn_State()		const
			{
				return this->dumpToFileOn_State_;
			}
			bool isDumpToFileOn_WeightDelta()		const
			{
				return this->dumpToFileOn_WeightDelta_;
			}
			bool isDumpToFileOn_Group()		const
			{
				return this->dumpToFileOn_Group_;
			}

			unsigned int getDumpIntervalInSec_Spikes()		const
			{
				return this->dumpIntervalInSec_Spikes_;
			}
			unsigned int getDumpIntervalInSec_Topology()	const
			{
				return this->dumpIntervalInSec_Topology_;
			}
			unsigned int getDumpIntervalInSec_State()		const
			{
				return this->dumpIntervalInSec_State_;
			}
			unsigned int getDumpIntervalInSec_WeightDelta()	const
			{
				return this->dumpIntervalInSec_WeightDelta_;
			}
			unsigned int getDumpIntervalInSec_Group()		const
			{
				return this->dumpIntervalInSec_Group_;
			}

			void setDumpIntervalInSec_Spikes(const unsigned int sec)
			{
				this->dumpIntervalInSec_Spikes_ = sec; this->dumpToFileOn_Spikes_ = (sec > 0);
			}
			void setDumpIntervalInSec_Topology(const unsigned int sec)
			{
				this->dumpIntervalInSec_Topology_ = sec; this->dumpToFileOn_Topology_ = (sec > 0);
			}
			void setDumpIntervalInSec_State(const unsigned int sec)
			{
				this->dumpIntervalInSec_State_ = sec; this->dumpToFileOn_State_ = (sec > 0);
			}
			void setDumpIntervalInSec_WeightDelta(const unsigned int sec)
			{
				this->dumpIntervalInSec_WeightDelta_ = sec; this->dumpToFileOn_WeightDelta_ = (sec > 0);
			}
			void setDumpIntervalInSec_Group(const unsigned int sec)
			{
				this->dumpIntervalInSec_Group_ = sec; this->dumpToFileOn_Group_ = (sec > 0);
			}

			std::string getFilenamePath_Spikes() const
			{
				return this->filenamePath_Spikes_;
			}
			std::string getFilenamePath_Topology() const
			{
				return this->filenamePath_Topology_;
			}
			std::string getFilenamePath_State() const
			{
				return this->filenamePath_State_;
			}
			std::string getFilenamePath_WeightDelta() const
			{
				return this->filenamePath_WeightDelta_;
			}
			std::string getFilenamePath_Group() const
			{
				return this->filenamePath_Group_;
			}

			void setFilenamePath_Spikes(const std::string& filename)
			{
				this->filenamePath_Spikes_ = filename;
			}
			void setFilenamePath_Topology(const std::string& filename)
			{
				this->filenamePath_Topology_ = filename;
			}
			void setFilenamePath_State(const std::string& filename)
			{
				this->filenamePath_State_ = filename;
			}
			void setFilenamePath_WeightDelta(const std::string& filename)
			{
				this->filenamePath_WeightDelta_ = filename;
			}
			void setFilenamePath_Group(const std::string& filename)
			{
				this->filenamePath_Group_ = filename;
			}

			std::string getFilenamePrefix_Spikes() const
			{
				return this->filenamePrefix_Spikes_;
			}
			std::string getFilenamePrefix_Topology() const
			{
				return this->filenamePrefix_Topology_;
			}
			std::string getFilenamePrefix_State() const
			{
				return this->filenamePrefix_State_;
			}
			std::string getFilenamePrefix_WeightDelta() const
			{
				return this->filenamePrefix_WeightDelta_;
			}
			std::string getFilenamePrefix_Group() const
			{
				return this->filenamePrefix_Group_;
			}

			void setFilenamePrefix_Spikes(const std::string& prefix)
			{
				this->filenamePrefix_Spikes_ = prefix;
			}
			void setFilenamePrefix_Topology(const std::string& prefix)
			{
				this->filenamePrefix_Topology_ = prefix;
			}
			void setFilenamePrefix_State(const std::string& prefix)
			{
				this->filenamePrefix_State_ = prefix;
			}
			void setFilenamePrefix_WeightDelta(const std::string& prefix)
			{
				this->filenamePrefix_WeightDelta_ = prefix;
			}
			void setFilenamePrefix_Group(const std::string& prefix)
			{
				this->filenamePrefix_Group_ = prefix;
			}

		private:

			// the number of samples taken to compute performance
			unsigned int nSamples_;
			bool nSamplesPresent_;

			Ms startUpTimeInMs_;
			bool startUpTimeInMsPresent_;

			Ms sampleTimeInMs_;
			bool sampleTimeInMsPresent_;

			Ms timeRoundingIntervalInMs_;
			bool timeRoundingIntervalInMsPresent_;

			// duration of the random case
			Ms randomCaseDurationInMs_;
			bool randomCaseDurationInMsPresent_;

			// duration of (regular) cases
			bool caseDurationInMsPresent_;
			Ms caseDurationInMs_;

			// duration of silence at the end of (regular) cases
			bool caseTailSilenceInMsPresent_;
			Ms caseTailSilenceInMs_;

			// spike Hz of random spikes in the random case
			double randomSpikeHz_;
			bool randomSpikeHzPresent_;

			double correctNeuronSpikeHz_;
			bool correctNeuronSpikeHzPresent_;

			// refractory period
			Ms refractoryPeriodInMs_;
			bool refractoryPeriodInMsPresent_;

			bool dumpToFileOn_Spikes_;
			bool dumpToFileOn_Topology_;
			bool dumpToFileOn_State_;
			bool dumpToFileOn_WeightDelta_;
			bool dumpToFileOn_Group_;

			unsigned int dumpIntervalInSec_Spikes_;
			unsigned int dumpIntervalInSec_Topology_;
			unsigned int dumpIntervalInSec_State_;
			unsigned int dumpIntervalInSec_WeightDelta_;
			unsigned int dumpIntervalInSec_Group_;

			std::string filenamePath_Spikes_;
			std::string filenamePath_Topology_;
			std::string filenamePath_State_;
			std::string filenamePath_WeightDelta_;
			std::string filenamePath_Group_;

			std::string filenamePrefix_Spikes_;
			std::string filenamePrefix_Topology_;
			std::string filenamePrefix_State_;
			std::string filenamePrefix_WeightDelta_;
			std::string filenamePrefix_Group_;
		};
	}
}