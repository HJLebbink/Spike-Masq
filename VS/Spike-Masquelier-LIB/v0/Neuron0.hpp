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

#ifdef _MSC_VER
#pragma warning (disable: 4710) // warning C4710: '*' : function not inlined
#pragma warning (disable: 4324) // warning C4324: '*' : structure was padded due to __declspec(align())
#pragma warning (disable: 4127) // warning C4127: conditional expression is constant
#endif

#include <stdio.h> // for FILE
#include <stdio.h>	// for printf
#include <stdlib.h>	// for malloc
#include <cstring>	// for memset, memcpy
#include <iostream> // for cerr and cout
#include <time.h>	// for time
#include <math.h>	// for tanh
#include <memory>
#include <bitset>

#include <intrin.h>

#include "../../Spike-Masquelier-LIB/v0/SpikeOptionsMasq.hpp"
#include "../../Spike-Masquelier-LIB/v0/SpikeTools.hpp"

namespace spike
{
	namespace v0
	{
		template <typename NET>
		class Neuron0
		{

		public:

			const SpikeNeuronId id;
			const SpikeNeuronId nAfferentNeurons;
			const NET& network_;

			//__declspec(align(16)) 
			TJData2 t_j_data2[SpikeOptionsMasq::T_J_LENGTH];

			int t_j_end;
			int t_j_begin;

			__declspec(align(16)) SpikePotential futurePotential[SpikeOptionsMasq::T_J_LENGTH];
			unsigned int futurePotentialStartKernelTime;
			unsigned int futurePotentialEndIndex;

			std::vector<Weight> weight_;
			std::vector<Weight> weightDelta_;

			SpikeTime lastEpspTime;
			SpikeTime nextEpspTime;
			SpikeTime endRefractoryPeriodTime;

			~Neuron0() = default;

			Neuron0() = delete;

			Neuron0(
				const SpikeNeuronId id,
				const SpikeNeuronId nAfferentNeurons,
				const NET& network)

				: id(id)
				, nAfferentNeurons(nAfferentNeurons)
				, network_(network)
				, options_(network.getOptions())

				, t_j_begin(0)
				, t_j_end(0)
				, lastEpspTime(LAST_INF_TIME)
				, nextEpspTime(LAST_INF_TIME)
				, endRefractoryPeriodTime(FIRST_INF_TIME)
				, maximumPotential(0)
			{
				this->weight_.resize(nAfferentNeurons);
				this->weightDelta_.resize(nAfferentNeurons);
				this->alreadyDepressed_.resize(nAfferentNeurons);
				this->alreadyPotentiated_.resize(nAfferentNeurons);

				this->initWeights();

				std::fill(this->alreadyDepressed_.begin(), this->alreadyDepressed_.end(), false);
				std::fill(this->alreadyPotentiated_.begin(), this->alreadyPotentiated_.end(), false);

				memset(this->futurePotential, 0, SpikeOptionsMasq::T_J_LENGTH * sizeof(SpikePotential));
				this->futurePotentialStartKernelTime = 0;
				this->futurePotentialEndIndex = 0;
			}

			Neuron0 & operator=(const Neuron0&) = delete;

			Neuron0(const Neuron0&) = delete;

			Neuron0 getId() const
			{
				return this->id;
			}

			SpikePotential calculatePotential(const SpikeTime spikeTime)
			{
				//	std::cout << "SpikeNeuron::calculatePotential(): t=" << t << std::endl;
				const KernelTime kernelTime = (KernelTime)(spikeTime >> (SpikeOptionsMasq::TIME_DENOMINATOR_POW - SpikeOptionsMasq::KERNEL_SCAN_INTERVAL_POW));
				const int index = static_cast<int>(kernelTime - this->futurePotentialStartKernelTime);

				::tools::assert::assert_msg((index >= 0) && (index < SpikeOptionsMasq::T_J_LENGTH), "SpikeNeuron::calculatePotential(): error; index=", index, "; kernelTime=", kernelTime, "; futurePotentialStartKernelTime=", futurePotentialStartKernelTime, "; KERNEL_SIZE=", SpikeOptionsMasq::KERNEL_SIZE);

				return this->futurePotential[index];
			}

			void initWeights()
			{
				//std::cout << "SpikeNeuron::initWeights()" << std::endl;
				for (SpikeNeuronId inputNeuronId = 0; inputNeuronId < this->nAfferentNeurons; ++inputNeuronId)
				{
					const int initialWeightInt = std::lroundf(::tools::random::rand_float(1.0) * SpikeOptionsMasq::POTENTIAL_DENOMINATOR);

					::tools::assert::assert_msg((inputNeuronId >= 0) && (inputNeuronId < nAfferentNeurons), "SpikeNeuron::initWeights() inputNeuronId=", inputNeuronId, "; nAfferentNeurons=", nAfferentNeurons);

					this->weight_[inputNeuronId] = static_cast<Weight>(initialWeightInt);
					if (initialWeightInt != this->weight_[inputNeuronId])
					{
						std::cerr << "SpikeNeuron::initWeights: initialWeightInt =" << initialWeightInt << " does not fit initialWeight. weight = "<< this->weight_[inputNeuronId] << std::endl;
					}
					//std::printf("INFO: init w: neuron %u has a pathway to neuron %u with weight %7.5f.\n", inputNeuronId, this->id, static_cast<float>(this->weight_[inputNeuronId]) / SpikeOptionsMasq::POTENTIAL_DENOMINATOR);


					#ifdef _DEBUG 
					if (false && (SpikeOptionsMasq::traceNeuronOn) && (this->id == SpikeOptionsMasq::traceNeuronId) && (inputNeuronId == SpikeOptionsMasq::traceNeuronId2))
					{
						std::printf("INFO: init w: n=%5u        at %8.3fms; neuron %u has a pathway to neuron %u with weight %7.5f.\n", this->id, 0.0, inputNeuronId, this->id, static_cast<float>(this->weight_[inputNeuronId]) / SpikeOptionsMasq::POTENTIAL_DENOMINATOR);
					}
					#endif 
				}
			}

			void fire(const SpikeTime spikeTime)
			{
				//	printf("SpikeNeuron::fire(): time = %f ms, (%d) \n", (float)spikeTime/TIME_DENOMINATOR, spikeTime);

				#ifdef _DEBUG 
				if (true && (SpikeOptionsMasq::traceNeuronOn) && (this->id == SpikeOptionsMasq::traceNeuronId))
				{
					std::printf("INFO: USAS:   n=%5u spikes at %8.3fms; [update state after spike]\n", this->id, static_cast<float>(spikeTime) / SpikeOptionsMasq::TIME_DENOMINATOR);
				}
				#endif 

				const KernelTime kernelTime = (KernelTime)(spikeTime >> (SpikeOptionsMasq::TIME_DENOMINATOR_POW - SpikeOptionsMasq::KERNEL_SCAN_INTERVAL_POW));

				this->maximumPotential = 0;
				this->lastEpspTime = spikeTime;
				this->endRefractoryPeriodTime = spikeTime + (this->options_.refractory_period* SpikeOptionsMasq::TIME_DENOMINATOR);

				this->futurePotentialStartKernelTime = kernelTime;
				this->futurePotentialEndIndex = SpikeOptionsMasq::KERNEL_SIZE;

				//	memcpy(this->futurePotential, this->network->cachedEta, KERNEL_SIZE*sizeof(SpikePotential));
				const SpikePotential * __restrict const etaArray = this->network_.cachedEta; // because c++ does not inline this properly ...
				for (int i = 0; i < SpikeOptionsMasq::KERNEL_SIZE; i++)
				{
					this->futurePotential[i] = etaArray[i];
				}
				//	memset(this->futurePotential + KERNEL_SIZE, 0, (T_J_LENGTH-KERNEL_SIZE)*sizeof(SpikePotential));
				for (int i = SpikeOptionsMasq::KERNEL_SIZE; i < (SpikeOptionsMasq::T_J_LENGTH - SpikeOptionsMasq::KERNEL_SIZE); i++)
				{
					this->futurePotential[i] = 0;
				}

				this->ltp(spikeTime);
				std::fill(this->alreadyDepressed_.begin(), this->alreadyDepressed_.end(), false);

				this->t_j_end = 0; // clear the t_j_list;
				this->t_j_begin = 0;

				this->network_.epspContainer->storeSpikeEvent(makeSpikeEvent(spikeTime, this->id));

				if (!this->options_.quiet)
				{
					printf("SpikeNeuron::fire(): %s\n", this->getProgressionResults(spikeTime).c_str());
				}
			}

			void commitEpsp(const SpikeTime presynapiticSpikeTime, const SpikeNeuronId originatingNeuronId)
			{
				//std::cout << "spike::v1::SpikeNeuron::commitEpsp: neuronId="<<this->id << "; presynapiticSpikeTime=" << presynapiticSpikeTime << "; originatingNeuronId=" << originatingNeuronId << std::endl;

				const KernelTime kernelTime = static_cast<KernelTime>(presynapiticSpikeTime >> (SpikeOptionsMasq::TIME_DENOMINATOR_POW - SpikeOptionsMasq::KERNEL_SCAN_INTERVAL_POW));

				::tools::assert::assert_msg((this->t_j_end >= 0) && (this->t_j_end < SpikeOptionsMasq::T_J_LENGTH), "SpikeNeuron::commitEpsp() t_j_end=", this->t_j_end, "; T_J_LENGTH=", SpikeOptionsMasq::T_J_LENGTH);

				this->t_j_data2[this->t_j_end].t_j_time = presynapiticSpikeTime;
				this->t_j_data2[this->t_j_end].t_j_epspAfferent = originatingNeuronId;
				this->t_j_end++;
				if (this->t_j_end >= (SpikeOptionsMasq::T_J_LENGTH - 100))
				{
					this->cleanup_t_j_array(kernelTime);
				}

				const int startIndex = static_cast<int>(kernelTime - this->futurePotentialStartKernelTime);
				this->updateEpsilon(startIndex, originatingNeuronId);

				if (this->futurePotential[startIndex] > this->maximumPotential)
				{
					this->maximumPotential = this->futurePotential[startIndex];
				}
				if (this->network_.dumper_ != nullptr)
				{
					this->network_.dumper_->dumpPotential(this->id, presynapiticSpikeTime, this->futurePotential[startIndex]);
				}
				this->futurePotentialEndIndex = startIndex + SpikeOptionsMasq::KERNEL_SIZE;
				if (this->futurePotentialEndIndex > (SpikeOptionsMasq::T_J_LENGTH - 100))
				{
					this->cleanup_futureEpsp();
				}

				this->ltd(presynapiticSpikeTime, originatingNeuronId);
			}

			void commitIpsp(const SpikeTime spikeTime)
			{

				// assume that this neuron cannot fire this moment (just after receiving an incoming ipsp
				this->nextEpspTime = LAST_INF_TIME;

				const KernelTime kernelTime = (KernelTime)(spikeTime >> (SpikeOptionsMasq::TIME_DENOMINATOR_POW - SpikeOptionsMasq::KERNEL_SCAN_INTERVAL_POW));
				const int startIndex = static_cast<int>(kernelTime - this->futurePotentialStartKernelTime);
				//	log.info("commitEpsp(): neuron "+this.id+" kernelTime="+kernelTime+"; futureEpspStartKernelTime="+futureEpspStartKernelTime+"; index="+index);

				if (startIndex < 0)
				{
					std::cerr << "SpikeNeuron::commitIpsp() startIndex=" << startIndex << std::endl; throw 1;
				}
				this->updateMu(startIndex);
				this->futurePotentialEndIndex = startIndex + SpikeOptionsMasq::KERNEL_SIZE;

				if (this->futurePotentialEndIndex > (SpikeOptionsMasq::T_J_LENGTH - 100))
				{
					this->cleanup_futureEpsp();
				}

				//	if (this.dumper != nullptr) {
				//		this.dumper.dumpPotential(this.id, t_k, this.calculatePotential(t_k));
				//	}
				//1] no weights update for ipsp
				//2] no need to search for next epsp
			}

			bool doesNeuronFire(const SpikeEvent inputEvent, const SpikeTime /*nextInputSpikeTime*/)
			{
				//	printf("SpikeNeuron::integrateEpsp(): n%d; t_j=%d; originatingNeuronId=%d; nextScheduledInputSpikeEvent=%s\n", this->id, t_j, originatingNeuronId, event2String(nextScheduledInputSpikeEvent).c_str());

				const SpikeTime presynapiticSpikeTime = getTimeFromSpikeEvent(inputEvent);
				const SpikeNeuronId originatingNeuronId = getOriginatingNeuronIdFromSpikeEvent(inputEvent);

				if (this->options_.beSmart)
				{
					#if _DEBUG
					if ((originatingNeuronId < 0) || (originatingNeuronId >= this->nAfferentNeurons))
					{
						std::cerr << "SpikeNeuron::doesNeuronFire() originatingNeuronId=" << originatingNeuronId << "; nAfferentNeurons=" << nAfferentNeurons << std::endl; throw 1;
					}
					#endif
					const SpikePotential w = this->weight_[originatingNeuronId];
					//if ((this->maximumPotential + w) < THRESHOLD_INT) {
					if ((this->maximumPotential + w) < this->options_.thresholdInt)
					{
						//std::cout << "SpikeNeuron::integrateEpsp() be smart optimization" << std::endl;
						this->maximumPotential += w;
						return false;
					}
				}

				if (this->endRefractoryPeriodTime > presynapiticSpikeTime)
				{
					// no need to calculate the potential simply because this neuron is in the refractory period
					//std::cout<< "integrateEpsp(): neuron "<<this->id <<" in refractory period: endRefractoryPeriodTime="<<this->endRefractoryPeriodTime<<"; current time="<<presynapiticSpikeTime << std::endl;
					return false;
				}

				const SpikePotential potential = this->calculatePotential(presynapiticSpikeTime);
				//std::cout << "SpikeNeuron::estimateNextEpspEvent(): potential=" << potential << "; threshold="<< this->options_.thresholdInt << std::endl;

				//if (potential > THRESHOLD_INT) {
				if (potential > this->options_.thresholdInt)
				{ // this neuron fires
					this->nextEpspTime = presynapiticSpikeTime;
					return true;
				}
				//	std::cout << "SpikeNeuron::estimateNextEpspEvent(): n"<<this->id <<"; could not find potential over threshold." << std::endl;
				return false;
			}

		private:

			const SpikeOptionsMasq options_;
			SpikePotential maximumPotential;
			std::vector<bool> alreadyDepressed_;
			std::vector<bool> alreadyPotentiated_;

			void updateWeight(const SpikeNeuronId neuronId, const WeightDelta wDelta)
			{
				::tools::assert::assert_msg(neuronId < nAfferentNeurons, "SpikeNeuron::updateWeight(): neuronId=", neuronId, "is not ok");

				int weightTmp = this->weight_[neuronId] + wDelta;
				if (weightTmp < 0)
				{
					weightTmp = 0;
				}
				else if (weightTmp > SpikeOptionsMasq::POTENTIAL_DENOMINATOR)
				{
					weightTmp = SpikeOptionsMasq::POTENTIAL_DENOMINATOR;
				}
				//		std::cout << "SpikeNeuron::updateWeight(): old weight="<<this->weight[neuronId]<<" new weight="<<(Weight)weightTmp <<std::endl;
				this->weight_[neuronId] = static_cast<Weight>(weightTmp);
			}

			void cleanup_t_j_array(const KernelTime kernelTime)
			{
				//	printf("SpikeNeuron::cleanup_t_j_array(): neuron = %d, kernelTime=%d\n", this->id, kernelTime);

				const int timeMinusKernelSize = kernelTime - SpikeOptionsMasq::KERNEL_SIZE;
				for (int i = this->t_j_end - 1; i >= this->t_j_begin; i--)
				{
					const SpikeTime time1 = this->t_j_data2[i].t_j_time;
					const KernelTime kernelTime2 = (KernelTime)(time1 >> (SpikeOptionsMasq::TIME_DENOMINATOR_POW - SpikeOptionsMasq::KERNEL_SCAN_INTERVAL_POW));
					if (kernelTime2 <= timeMinusKernelSize)
					{
						this->t_j_begin = i + 1;
						return;
					}
				}

				const int count = this->t_j_end - this->t_j_begin;
				//	printf("SpikeNeuron::cleanup_t_j_array() %d\n", this->id);
				//	std::cout << "SpikeNeuron::cleanup_t_j_array(): n "<<this->id <<": t_j=" << this->t_j_begin << "-"<< this->t_j_end << "/" <<t_j_length << "; count="<< count << std::endl;

				void * dest2 = this->t_j_data2;
				const void * src2 = this->t_j_data2 + t_j_begin;
				const size_t nByteToCopy = count * sizeof(TJData2);
				//	std::cout << "SpikeNeuron::cleanup_t_j_array(): neuron " << this->id << "src2=" <<src2<<"; nByteToCopy="<<nByteToCopy<< "; end address="<<this->t_j_data2+(t_j_length*sizeof(TJData2)) <<std::endl;
				memmove(dest2, src2, nByteToCopy);

				/*	for (int i=0; i<count; i++) {
				const int index = this->t_j_begin + i;
				this->t_j_data2[i] = this->t_j_data2[index];
				}
				*/
				this->t_j_begin = 0;
				this->t_j_end = count;

				// the following check is very dangerous if left out; the guts of this method will otherwise be overwritten...
				if (this->t_j_end >= SpikeOptionsMasq::T_J_LENGTH)
				{
					std::cout << "SpikeNeuron::cleanup_t_j_array() T_J_LENGTH=" << SpikeOptionsMasq::T_J_LENGTH << " is too small." << std::endl;
				}
			}

			void cleanup_futureEpsp()
			{
				//	printf("SpikeNeuron::cleanup_futureEpsp() neuron %d; futurePotentialStartKernelTime=%d; futurePotentialEndIndex=%d\n", this->id, futurePotentialStartKernelTime, futurePotentialEndIndex);
				//	std::cout << "SpikeNeuron::cleanup_futureEpsp() XX "<< this->id<< std::endl;
				const int cleanupSize = 1000;

				//BUG: the memmove or memset has a bug!
				//	memmove(this->futurePotential, this->futurePotential+cleanupSize, cleanupSize*sizeof(SpikePotential));
				for (int i = 0; i < (SpikeOptionsMasq::T_J_LENGTH - cleanupSize); i++)
				{
					this->futurePotential[i] = this->futurePotential[i + cleanupSize];
				}
				//	memset(this->futurePotential+(T_J_LENGTH-cleanupSize), 0, cleanupSize);
				for (int i = (SpikeOptionsMasq::T_J_LENGTH - cleanupSize); i < SpikeOptionsMasq::T_J_LENGTH; i++)
				{
					this->futurePotential[i] = 0;
				}
				this->futurePotentialStartKernelTime += cleanupSize;
				this->futurePotentialEndIndex -= cleanupSize;
				//	printf("SpikeNeuron::cleanup_futureEpsp() after: neuron %d; futurePotentialStartKernelTime=%d; futurePotentialEndIndex=%d\n", this->id, futurePotentialStartKernelTime, futurePotentialEndIndex);
			}

			std::string getProgressionResults(const SpikeTime time) const
			{
				const float timeInMs = ((float)time) / SpikeOptionsMasq::TIME_DENOMINATOR;
				//	log.info("passedTimeInMs= "+passedTimeInMs+"; timeInMs="+timeInMs+"; tUnit="+tUnit);
				const float percentageDone = (timeInMs / this->network_.t_max_inMs) * 100;

				/*
				float sum1 = 0;
				float sum2 = 0;
				for (int i=0; i<1000; i++) {
				sum1 += (float)this->weight[i]/POTENTIAL_DENOMINATOR;
				sum2 += (float)this->weight[i+1000]/POTENTIAL_DENOMINATOR;
				}
				//	std::cout<< "getProgressionResults(): sum1="<<sum1/1000 << " sum2=" << sum2/1000 << std::endl;
				*/

				std::string message;

				char buff[400];
				bool firedInPattern = false;
				const std::vector<CaseId> caseIds = this->network_.inputContainer_->getCaseIds();

				for (const CaseId caseId : caseIds)
				{
					const float latency = this->network_.inputContainer_->latency(timeInMs, caseId);
					if (latency > 0.0f)
					{
						firedInPattern = true;
						sprintf_s(buff, "at %.3f ms (%.1f%%) neuron %d: caseId %d, latency %f", timeInMs, percentageDone, this->id, caseId.val, latency);
					}
				}
				if (!firedInPattern)
				{
					sprintf_s(buff, "at %.3f ms (%.1f%%) neuron %d: caseId -, latency -", timeInMs, percentageDone, this->id);
				}
				std::string buffAsStdStr = buff;


				//	char buff[200];
				//	sprintf(buff, "at %.3f ms (t=%d); %.1f%% done; t_j=%d-%d/%d; t_k=%d-%d/%d", timeInMs, time, percentageDone, this->t_j_begin, this->t_j_end, t_j_length, this->t_k_begin, this->t_k_end, t_k_length);
				//	std::string buffAsStdStr = buff;

				return buffAsStdStr;
			}

			void ltd(const SpikeTime spikeTime, const SpikeNeuronId afferent)
			{
				::tools::assert::assert_msg(afferent < this->nAfferentNeurons, "SpikeNeuron::ltd() afferent=", afferent, "; which is not correct");

				if (this->alreadyDepressed_[afferent])
				{
					return;
				}
				this->alreadyDepressed_[afferent] = true;

				if (this->lastEpspTime == LAST_INF_TIME)
				{ // nothing to do if this neuron has not yet fired
					return;
				}

				const int index = spikeTime - this->lastEpspTime;

				::tools::assert::assert_msg(index >= 0, "SpikeNeuron::ltd(): index is too small. index=", index);
				//::tools::assert::assert_msg(index < LTD_KERNEL_SIZE, "SpikeNeuron::ltd(): index is too large. index=" << index << "; LTD_KERNEL_SIZE=" << LTD_KERNEL_SIZE);
				if (index < SpikeOptionsMasq::LTD_KERNEL_SIZE)
				{
					const WeightDelta wDelta = this->network_.cachedLtd[index];
					//	std::cout << "SpikeNeuron::integrateEpsp() this->lastEpspTime=" << this->lastEpspTime << "; LTD wDelta=" << wDelta << std::endl;
					this->updateWeight(afferent, wDelta);

					#ifdef _DEBUG 
					if (true && (SpikeOptionsMasq::traceNeuronOn) && (this->id == SpikeOptionsMasq::traceNeuronId) && (afferent == SpikeOptionsMasq::traceNeuronId2))
					{
						std::printf("INFO: LTD: A: n=%5u        at %8.3fms; neuron %u receives a spike from neuron %u, it but was %8.3fms too late (last=%5.3fms; ltd_index=%i; wD=%7.4f, newW=%7.4f)\n", this->id, static_cast<float>(spikeTime) / SpikeOptionsMasq::TIME_DENOMINATOR, this->id, afferent, static_cast<float>(index) / SpikeOptionsMasq::TIME_DENOMINATOR, static_cast<float>(this->lastEpspTime) / SpikeOptionsMasq::TIME_DENOMINATOR, index, static_cast<float>(wDelta) / SpikeOptionsMasq::POTENTIAL_DENOMINATOR, static_cast<float>(this->weight_[afferent]) / SpikeOptionsMasq::POTENTIAL_DENOMINATOR);
					}
					#endif 
				}
				//	if (this->network->dumper != nullptr) {
				//		this->network->dumper->dumpWeight(this->id, afferent, spikeTime, this->weight[afferent]);
				//	}
			}

			void ltp(const SpikeTime spikeTime)
			{
				const WeightDelta * __restrict const cachedLtpArray = this->network_.cachedLtp; // because c++ does not inline this propery...

				if (false)
				{
					std::fill(this->alreadyDepressed_.begin(), this->alreadyDepressed_.end(), false);
					for (int i = this->t_j_end - 1; i >= this->t_j_begin; --i)
					{
						::tools::assert::assert_msg((i >= 0) && (i < SpikeOptionsMasq::T_J_LENGTH), "SpikeNeuron::ltp() i = ", i, "; T_J_LENGTH = ", SpikeOptionsMasq::T_J_LENGTH);

						const SpikeNeuronId epspAfferent = this->t_j_data2[i].t_j_epspAfferent;
						::tools::assert::assert_msg(epspAfferent < this->nAfferentNeurons, "SpikeNeuron::ltp() epspAfferent=", epspAfferent, "; which is not correct");

						if (!this->alreadyPotentiated_[epspAfferent])
						{
							const int index = spikeTime - this->t_j_data2[i].t_j_time;
							if (index < 0)
							{
								this->t_j_begin = i + 1;
								break;
							}
							::tools::assert::assert_msg((index >= 0) && (index < SpikeOptionsMasq::LTP_KERNEL_SIZE), "SpikeNeuron::ltp(): index=", index, "; this should not happen!");

							const WeightDelta wDelta = cachedLtpArray[index];
							this->updateWeight(epspAfferent, wDelta); // do LTP
							this->alreadyPotentiated_[epspAfferent] = true;
						}
					}
				}
				else
				{
					std::fill(this->weightDelta_.begin(), this->weightDelta_.end(), static_cast<Weight>(0));

					for (int i = this->t_j_end - 1; i >= this->t_j_begin; i--)
					{
						::tools::assert::assert_msg((i >= 0) && (i < SpikeOptionsMasq::T_J_LENGTH), "SpikeNeuron::ltp() i=", i, "; T_J_LENGTH=", SpikeOptionsMasq::T_J_LENGTH);

						const SpikeNeuronId epspAfferent = this->t_j_data2[i].t_j_epspAfferent;
						::tools::assert::assert_msg(epspAfferent < this->nAfferentNeurons, "SpikeNeuron::ltp() epspAfferent=", epspAfferent, "; nAfferentNeurons=", nAfferentNeurons);

						if (this->weightDelta_[epspAfferent] == 0)
						{
							const int index = spikeTime - this->t_j_data2[i].t_j_time;
							if (index < 0)
							{
								this->t_j_begin = i + 1;
								break;
							}
							//::tools::assert::assert_msg((index >= 0) && (index < LTP_KERNEL_SIZE), "SpikeNeuron::ltp(): index = " << index << "; LTP_KERNEL_SIZE = " << LTP_KERNEL_SIZE << "; i = " << i);

							if (index < SpikeOptionsMasq::LTP_KERNEL_SIZE)
							{
								this->weightDelta_[epspAfferent] = cachedLtpArray[index];

								#ifdef _DEBUG 
								if (true && (SpikeOptionsMasq::traceNeuronOn) && (this->id == SpikeOptionsMasq::traceNeuronId) && (epspAfferent == SpikeOptionsMasq::traceNeuronId2))
								{
									std::printf("INFO: LTP: A: n=%5u        at %8.3fms; neuron %u receives a spike from neuron %u, it was %7.3fms before the received time %7.3fms; (oldW=%7.4f; wD=%7.5f; newW=?)\n", this->id, static_cast<float>(spikeTime) / SpikeOptionsMasq::TIME_DENOMINATOR, this->id, epspAfferent, static_cast<float>(index) / SpikeOptionsMasq::TIME_DENOMINATOR, static_cast<float>(this->t_j_data2[i].t_j_time) / SpikeOptionsMasq::TIME_DENOMINATOR, static_cast<float>(this->weight_[epspAfferent]) / SpikeOptionsMasq::POTENTIAL_DENOMINATOR, static_cast<float>(this->weightDelta_[epspAfferent]) / SpikeOptionsMasq::POTENTIAL_DENOMINATOR);
								}
								#endif 
							}

							//::tools::assert::assert_msg(this->weightDelta_[epspAfferent] >= 0, "SpikeNeuron::ltp() wDelta < 0; this may happen");
						}
					}
					__m128i * __restrict const wa = (__m128i*)this->weight_.data();
					__m128i * __restrict const wda = (__m128i*)this->weightDelta_.data();

					for (unsigned int i = 0; i < (nAfferentNeurons >> 3); i++)
					{
						::tools::assert::assert_msg((i >= 0) && (i < this->nAfferentNeurons), "SpikeNeuron::ltp() i=", i, "; nAfferentNeurons=", this->nAfferentNeurons);

						const __m128i wdai1 = _mm_stream_load_si128(&wda[i]);
						const __m128i wai = _mm_load_si128(&wa[i]);
						const __m128i wdai2 = _mm_adds_epu16(wdai1, wai);
						_mm_store_si128(&wa[i], wdai2);
					}
				}
			}

			void updateEpsilon(const int startIndex, const SpikeNeuronId originatingNeuronId)
			{
				if (true)
				{
					const SpikeEpsilon * __restrict const epsilonArray = this->network_.cachedEpsilon0; // because c++ does not inline this properly ...
					const Weight w = this->weight_[originatingNeuronId];

					#ifdef _DEBUG 
					if (false && (SpikeOptionsMasq::traceNeuronOn) && (this->id == SpikeOptionsMasq::traceNeuronId) && (originatingNeuronId == SpikeOptionsMasq::traceNeuronId2))
					{
						std::printf("INFO: UE:     n=%5u        at %8.3fms; neuron %u state is updated with weight %f\n", this->id, nanf, this->id, static_cast<float>(w) / SpikeOptionsMasq::POTENTIAL_DENOMINATOR);
					}
					#endif
					int index = startIndex;
					for (unsigned int i = 0; i < SpikeOptionsMasq::KERNEL_SIZE; ++i)
					{
						::tools::assert::assert_msg((index >= 0) && (index < SpikeOptionsMasq::T_J_LENGTH), "SpikeNeuron::updateEpsilon(): error index; index=", index, "; KERNEL_SIZE=", SpikeOptionsMasq::KERNEL_SIZE);
						::tools::assert::assert_msg(epsilonArray[i] < (1 << 16), "SpikeNeuron::updateEpsilon(): error epsilonArray; epsilonArray=", epsilonArray[i], " which is too big");
						::tools::assert::assert_msg(w < (1 << 16), "SpikeNeuron::updateEpsilon(): error w; w=", w, " which is too big");

						const int potentialDelta = (((unsigned int)(epsilonArray[i] * w)) >> SpikeOptionsMasq::POTENTIAL_DENOMINATOR_POW);
						::tools::assert::assert_msg(potentialDelta < (1 << 16), "SpikeNeuron::updateEpsilon(): error potentialDelta; potentialDelta=", potentialDelta, " which is too big");

						futurePotential[index] += potentialDelta;
						index++;
					}
				}
				else
				{
					const __m128i w = _mm_set1_epi16(this->weight_[originatingNeuronId]);
					__m128i * __restrict const fp = (__m128i*)this->futurePotential;
					__m128i * __restrict ep;

					switch (startIndex & 0x3)
					{
						case 0:
							ep = (__m128i*)this->network_.cachedEpsilon0;
							break;
						case 1:
							ep = (__m128i*)this->network_.cachedEpsilon1;
							break;
						case 2:
							ep = (__m128i*)this->network_.cachedEpsilon2;
							break;
						case 3:
							ep = (__m128i*)this->network_.cachedEpsilon3;
							break;
						default:
							// cannot be reached: just to suppress a warning assign ep to null
							ep = nullptr;
							throw 1;
					}

					int i2 = startIndex >> 2;

					//	futurePotential[index] += (((unsigned int)(epsilonArray[i] * weight))>>POTENTIAL_DENOMINATOR_POW);

					const int method = 5;
					switch (method)
					{
						case 5:
							for (int i = 0; i < (SpikeOptionsMasq::KERNEL_SIZE >> 3); (i++, i2 += 2))
							{

								//  Load 8 16-bit ushorts.
								//  epsilon = {a,b,c,d,e,f,g,h}
								const __m128i epsilon = _mm_stream_load_si128(&ep[i]);

								/*
								__m128i _mm_mulhi_epu16 (__m128i a, __m128i b);
								Multiplies the 8 unsigned 16-bit integers from a by the 8 unsigned 16-bit integers from b.
								r0 := (a0 * b0)[31:16]
								r1 := (a1 * b1)[31:16]
								...
								r7 := (a7 * b7)[31:16]
								*/
								const __m128i a = _mm_mulhi_epu16(epsilon, w);

								const __m128i f0 = _mm_load_si128(&fp[i2]);
								const __m128i f1 = _mm_load_si128(&fp[i2 + 1]);

								//  Convert to 32-bit integers
								//  b0 = {a,0,b,0,c,0,d,0}
								//  b1 = {e,0,f,0,g,0,h,0}
								const __m128i b0 = _mm_cvtepu16_epi32(a);
								const __m128i b1 = _mm_cvtepu16_epi32(_mm_unpackhi_epi64(a, a));

								const __m128i c0 = _mm_add_epi32(f0, b0);
								const __m128i c1 = _mm_add_epi32(f1, b1);

								_mm_store_si128(&fp[i2], c0);
								_mm_store_si128(&fp[i2 + 1], c1);
							}
							break;
						case 6:
							for (int i = 0; i < (SpikeOptionsMasq::KERNEL_SIZE >> 4); (i += 2, i2 += 4))
							{

								::tools::assert::assert_msg(((i + 0) >= 0) && ((i + 1) <= (SpikeOptionsMasq::KERNEL_SIZE >> 3)), "SpikeNeuron::updateEpsilonSse() i=", i, "; KERNEL_SIZE>>3=", (SpikeOptionsMasq::KERNEL_SIZE >> 3));
								::tools::assert::assert_msg(((i2 + 0) >= 0) || ((i2 + 3) <= (SpikeOptionsMasq::T_J_LENGTH >> 3)), "SpikeNeuron::updateEpsilonSse() i2=", i2, "; T_J_LENGTH>>3=", (SpikeOptionsMasq::T_J_LENGTH >> 3));

								const __m128i epsilon0 = _mm_stream_load_si128(&ep[i + 0]);
								const __m128i epsilon1 = _mm_stream_load_si128(&ep[i + 1]);

								/*
								_MM_HINT_T0	T0 (temporal data) - prefetch data into all levels of the cache hierarchy.
								_MM_HINT_T1	T1 (temporal data with respect to first level cache) - prefetch data into level 2 cache and higher.
								_MM_HINT_T2	T2 (temporal data with respect to second level cache) - prefetch data into level 2 cache and higher.
								_MM_HINT_NTA	NTA (non-temporal data with respect to all cache levels) - prefetch data into non-temporal cache structure and into a location close to the processor, minimizing cache pollution.
								*/
								_mm_prefetch((char *)&fp[i2 + 0], _MM_HINT_T0);

								const __m128i a0 = _mm_mulhi_epu16(epsilon0, w);
								const __m128i a1 = _mm_mulhi_epu16(epsilon1, w);

								const __m128i f0 = _mm_load_si128(&fp[i2 + 0]);
								const __m128i f1 = _mm_load_si128(&fp[i2 + 1]);
								const __m128i f2 = _mm_load_si128(&fp[i2 + 2]);
								const __m128i f3 = _mm_load_si128(&fp[i2 + 3]);

								//  Convert to 32-bit integers
								//  b0 = {a,0,b,0,c,0,d,0}
								//  b1 = {e,0,f,0,g,0,h,0}
								const __m128i b0 = _mm_cvtepu16_epi32(a0);
								const __m128i b1 = _mm_cvtepu16_epi32(_mm_unpackhi_epi64(a0, a0));
								const __m128i b2 = _mm_cvtepu16_epi32(a1);
								const __m128i b3 = _mm_cvtepu16_epi32(_mm_unpackhi_epi64(a1, a1));

								const __m128i c0 = _mm_add_epi32(f0, b0);
								const __m128i c1 = _mm_add_epi32(f1, b1);
								const __m128i c2 = _mm_add_epi32(f2, b2);
								const __m128i c3 = _mm_add_epi32(f3, b3);

								_mm_store_si128(&fp[i2 + 0], c0);
								_mm_store_si128(&fp[i2 + 1], c1);
								_mm_store_si128(&fp[i2 + 2], c2);
								_mm_store_si128(&fp[i2 + 3], c3);
							}
							break;
					}
				}
			}

			void updateMu(const int startIndex)
			{
				if (false)
				{
					const SpikePotential * __restrict const muArray = this->network_.cachedMu0; // because c++ does not inline this properly ...
					int index = startIndex;
					for (int i = 0; i < SpikeOptionsMasq::KERNEL_SIZE; i++)
					{
						#if _DEBUG
						if ((index < 0) || (index >= SpikeOptionsMasq::T_J_LENGTH))
						{
							std::cerr << "SpikeNeuron::commitIpsp() index=" << index << "; T_J_LENGTH=" << SpikeOptionsMasq::T_J_LENGTH << std::endl; throw 1;
						}
						#endif
						this->futurePotential[index] += muArray[i];
						index++;
					}
				}
				else
				{
					__m128i * __restrict const fp = (__m128i*)this->futurePotential;
					const __m128i * __restrict cachedMu;

					#if _DEBUG
					if (startIndex < 0)
					{
						std::cerr << "SpikeNeuron::updateMuSse() startIndex=" << startIndex << std::endl; throw 1;
					}
					#endif
					switch (startIndex & 0x3)
					{
						case 0:
							cachedMu = (__m128i*)this->network_.cachedMu0;
							break;
						case 1:
							cachedMu = (__m128i*)this->network_.cachedMu1;
							break;
						case 2:
							cachedMu = (__m128i*)this->network_.cachedMu2;
							break;
						case 3:
							cachedMu = (__m128i*)this->network_.cachedMu3;
							break;
						default:
							// cannot be reached: just to suppress a warning
							cachedMu = nullptr;
							throw 1;
					}

					const int offset = startIndex >> 2;

					for (int i = 0; i < (SpikeOptionsMasq::KERNEL_SIZE >> 2); i++)
					{

						// cachedMu contains 32-bits integers; thus one 128-bits xmm register is 4 integers.
						// futurePotential contains 32-bits integers; thus one 128-bits xmm register is 4 integers.

						const int index1 = i;
						const int index2 = offset + i;

						#if _DEBUG
						if ((index1 < 0) || (index1 > (SpikeOptionsMasq::KERNEL_SIZE >> 2)))
						{
							std::cerr << "SpikeNeuron::updateMuSse() index1=" << index1 << "; KERNEL_SIZE>>2=" << (SpikeOptionsMasq::KERNEL_SIZE >> 2) << std::endl; throw 1;
						}
						if ((index2 < 0) || (index2 > (SpikeOptionsMasq::T_J_LENGTH >> 2)))
						{
							std::cerr << "SpikeNeuron::updateMuSse()  index2=" << index2 << "; T_J_LENGTH>>2=" << (SpikeOptionsMasq::T_J_LENGTH >> 2) << std::endl; throw 1;
						}
						#endif

						const __m128i mu = _mm_load_si128(&cachedMu[index1]);

						#if _DEBUG
						//std::cout << "updateMu(): mu.m128i_i32[0]="<<mu.m128i_i32[0] << "; index1="<<index1<< std::endl; 
						#endif


						const __m128i f = _mm_load_si128(&fp[index2]);
						const __m128i c = _mm_add_epi32(mu, f);

						_mm_store_si128(&fp[index2], c);
					}
				}
			}
		};
	}
}