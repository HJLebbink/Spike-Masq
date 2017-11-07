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
#include <set>
#include <deque>
#include <iostream> // for cerr and cout
#include <memory>

#include "../../../matio-1.5.2/src/matio.h"

#include "../../Spike-Tools-Lib/file.ipp"
#include "../../Spike-Tools-LIB/SpikeRuntimeOptions.hpp"

#include "../../Spike-Masquelier-LIB/v0/SpikeOptionsMasq.hpp"
#include "../../Spike-Masquelier-LIB/v0/SpikeTools.hpp"
#include "../../Spike-Masquelier-LIB/v0/SpikeClassificationMasquelier.hpp"

namespace spike
{
	namespace v0
	{
		class SpikeInputContainerMatlab
		{
		public:

			static constexpr double MIN_TIME_BETWEEN_SPIKES_IN_SEC = 0.001; // original MasquelierCode has: MIN_TIME_BETWEEN_SPIKES_IN_SEC=0;

			using TimeType = float;


			// destructor
			~SpikeInputContainerMatlab() = default;

			// constructor
			SpikeInputContainerMatlab() = delete;

			// constructor
			SpikeInputContainerMatlab(
				const SpikeOptionsMasq& options,
				const SpikeRuntimeOptions& /*spikeOptions*/
			)
				: options_(options)
				, pos_(0)
			{
			}

			// copy constructor
			SpikeInputContainerMatlab(const SpikeInputContainerMatlab&) = delete;

			// assignment
			SpikeInputContainerMatlab & operator=(const SpikeInputContainerMatlab&) = delete;

			void setInputFilename(const std::string& inputFilename)
			{
				this->inputFilename_ = inputFilename;
			}

			void setClassificationFilename(const std::string& filename)
			{
				this->classificationFilename_ = filename;
			}

			void load(const TimeType timeInMs)
			{
				this->loadInputData(timeInMs);
				this->loadClassification(timeInMs);
			}

			float latency(const float spikeTimeInMs, const CaseId caseId) const
			{
				return this->spikeClassificationMasquelier_.latency(spikeTimeInMs, caseId);
			}

			void saveToFile(
				const std::string& filename) const
			{
				// mutex to protect file access
				//static std::mutex mutex;

				// lock mutex before accessing file
				//std::lock_guard<std::mutex> lock(mutex);

				// create the directory
				const std::string tree = ::tools::file::getDirectory(filename);
				if (!::tools::file::mkdirTree(tree))
				{
					std::cerr << "spike::masquelier::SpikeInputContainer::saveToFile(): Unable to create directory " << tree << std::endl;
					throw std::runtime_error("unable to create directory");
				}

				FILE * const fs = fopen(filename.c_str(), "w");
				if (fs == nullptr)
				{
					std::cerr << "spike::masquelier::SpikeInputContainer::saveToFile: Error: could not write to file " << filename << std::endl;
					return;
				}

				fprintf(fs, "#spike <second> <nCases> <nSpikes>\n");
				const unsigned int second = 0;
				const unsigned int nCases = 0;
				const unsigned int nSpikes = this->getNumberOfSpikes();
				fprintf(fs, "%d %d %d\n", second, nCases, nSpikes);

				fprintf(fs, "#caseOccurance <caseId> <startTimeInMs> <endTimeInMs>\n");

				fprintf(fs, "#spikeData <timeInMs> <neuronId> <firingReason> (UNKNOWN = 0, PROPAGATED = 1, RANDOM = 2, CASE = 3, PROPAGATED_AND_CASE = 4, FORCED = 5)\n");

				for (unsigned int i = 0; i < nSpikes; ++i)
				{
					const SpikeEvent spikeEvent = this->spikeEvent_[i];
					const SpikeNeuronId neuronId = getOriginatingNeuronIdFromSpikeEvent(spikeEvent);
					const SpikeTime spikeTime = getTimeFromSpikeEvent(spikeEvent);
					const float timeInMsFloat = static_cast<float>(spikeTime) / SpikeOptionsMasq::TIME_DENOMINATOR;
					const unsigned int timeInMs = static_cast<unsigned int>(trunc(timeInMsFloat));
					fprintf(fs, "%d %d %d\n", timeInMs, neuronId, 5);
				}
				fclose(fs);
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
					return this->spikeEvent_.back();
				}
				else
				{
					return this->spikeEvent_[this->pos_];
				}
			}

			unsigned int getNumberOfSpikes() const
			{
				return static_cast<unsigned int>(this->spikeEvent_.size());
			}

			bool isEmpty()
			{
				return (this->pos_ == this->getNumberOfSpikes());
			}

			float getMaxTimeInMs() const
			{
				return this->maxTimeInMs_;
			}

			const std::set<SpikeNeuronId> getNeuronIds() const
			{
				std::set<SpikeNeuronId> neuronIds;
				for (unsigned int index = 0; index < this->getNumberOfSpikes(); ++index)
				{
					const SpikeEvent spikeEvent = this->spikeEvent_[index];
					neuronIds.insert(getTimeFromSpikeEvent(spikeEvent));
				}
				return neuronIds;
			}

			const std::vector<CaseId> getCaseIds() const
			{
				return this->spikeClassificationMasquelier_.getCaseIds();
			}


		private:

			const SpikeOptionsMasq options_;
			float maxTimeInMs_;

			unsigned int pos_;

			std::vector<SpikeEvent> spikeEvent_;

			std::string inputFilename_;
			std::string classificationFilename_;

			SpikeClassificationMasquelier spikeClassificationMasquelier_;


			void loadClassification(const TimeType /*maxTimeInMs*/)
			{
				this->spikeClassificationMasquelier_.load(this->classificationFilename_);
			}

			void loadInputData(const TimeType maxTimeInMs)
			{
				std::vector<double> lastSpikeTime(2000, -1);
				size_t nIgnoredSpikes = 0;

				const std::string variableName1 = "spikeList";
				const std::string variableName2 = "afferentListDouble";

				mat_t *mat;
				matvar_t *matvar1;
				matvar_t *matvar2;

				mat = Mat_Open(this->inputFilename_.c_str(), MAT_ACC_RDONLY);
				if (mat)
				{
					matvar1 = Mat_VarRead(mat, (char*)variableName1.c_str());
					matvar2 = Mat_VarRead(mat, (char*)variableName2.c_str());

					if ((matvar1 == NULL) || (matvar2 == NULL))
					{
						std::cerr << "spike::masquelier::SpikeInputContainerMatlab::load():error" << std::endl;
						throw 1;
					}
					else
					{
						Mat_VarReadDataAll(mat, matvar1);
						Mat_VarReadDataAll(mat, matvar2);

						const size_t length1 = matvar1->dims[1];
						const size_t length2 = matvar2->dims[1];

						if (length1 != length2)
						{
							std::cerr << "spike::masquelier::SpikeInputContainerMatlab::load(): sizes do not match " << std::endl;
							throw std::runtime_error("spike::masquelier::SpikeInputContainerMatlab::load(): sizes do not match");
						}

						//this->length_ = (numberOfInputEvents > length1) ? static_cast<unsigned int>(length1) : numberOfInputEvents;
						//this->spikeEvent_.resize(this->length_);

						char * data1 = (char *)matvar1->data;
						char * data2 = (char *)matvar2->data;
						const size_t stride1 = Mat_SizeOf(matvar1->data_type);
						const size_t stride2 = Mat_SizeOf(matvar2->data_type);


						const SpikeTime maxTime = static_cast<SpikeTime>(std::lround(maxTimeInMs * SpikeOptionsMasq::TIME_DENOMINATOR));
						size_t i = 0;

						bool continueLoading = true;
						while (continueLoading && (i < length1))
						{
							if ((i & 0xFFFFF) == 0) std::cout << "spike::masquelier::SpikeInputContainerMatlab::load: loaded " << i << " spikes" << std::endl;

							const double timeInSec = *(double *)(data1 + (i*stride1));
							const double afferentDouble = *(double *)(data2 + (i*stride2));

							const SpikeTime time = static_cast<SpikeTime>(std::lround(((timeInSec * 1000) + 0) * SpikeOptionsMasq::TIME_DENOMINATOR)); // plus x to simulate a x ms delay

							if (time > maxTime)
							{
								continueLoading = false;
								break;
							}
							else
							{
								const SpikeNeuronId afferent = static_cast<SpikeNeuronId>(std::lround(afferentDouble)) - 1; // minus one because the matlab file starts counting neurons at 1.

								const double timeBetweenPreviousSpike = timeInSec - lastSpikeTime[afferent];
								if (timeBetweenPreviousSpike >= MIN_TIME_BETWEEN_SPIKES_IN_SEC)
								{
									this->storeEvent(time, afferent);
									lastSpikeTime[afferent] = timeInSec;
								}
								else
								{
									nIgnoredSpikes++;
									//printf("spike:masquelier::SpikeInputContainerMatlab: neuron %u; previous spike at %f sec, current spike at %f sec; time between=%f sec\n", afferent, lastSpikeTime[afferent], timeInSec, timeBetweenPreviousSpike);
								}
								i++;
							}
						}
					}
					Mat_VarFree(matvar1);
					Mat_VarFree(matvar2);
					Mat_Close(mat);
				}
				else
				{
					std::cerr << "spike::masquelier::SpikeInputContainerMatlab::load:: could not load file " << this->inputFilename_ << std::endl;
					throw std::runtime_error("spike::masquelier::SpikeInputContainerMatlab::load:: could not load file");
				}
				this->maxTimeInMs_ = (float)getTimeFromSpikeEvent(this->spikeEvent_.back()) / SpikeOptionsMasq::TIME_DENOMINATOR;
				std::cout << "spike::masquelier::SpikeInputContainerMatlab::load: done loading " << this->spikeEvent_.size() << " spikes; number of ignored spikes " << nIgnoredSpikes << std::endl;
			}

			void storeEvent(const SpikeTime time, const SpikeNeuronId afferent)
			{
				const SpikeEvent se = makeSpikeEvent(time, afferent);
				//std::cout << "spike::masquelier::SpikeInputContainerMatlab::load: SpikeEvent=" << event2String(se) << std::endl;
				this->spikeEvent_.push_back(std::move(se));
				//std::cout<< "timeInSec="<<timeInSec << "; timeInMs="<< (float)time/TIME_DONOMINATOR << "; time = " << time << "; afferent="<<afferent << std::endl;
			}
		};
	}
}