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

#include <stdio.h>   // for FILE
#include <vector>
#include <string>
#include <memory>
#include <algorithm> // max
#include <iostream>  // for cerr and cout

#include "../../Spike-Tools-LIB/SpikeRuntimeOptions.hpp"
#include "../../Spike-Tools-LIB/Kernel.hpp"

#include "../../Spike-Masquelier-LIB/v0/SpikeTools.hpp"
#include "../../Spike-Masquelier-LIB/v0/SpikeOptionsMasq.hpp"
#include "../../Spike-Masquelier-LIB/v0/Neuron0.hpp"
#include "../../Spike-Masquelier-LIB/v0/Dumper0.hpp"
#include "../../Spike-Masquelier-LIB/v0/DumperSpikesMasquelier.hpp"
#include "../../Spike-Masquelier-LIB/v0/SpikeEpspContainer.hpp"

namespace spike
{
	namespace v0
	{
		template <typename InputContainer>
		class Network0
		{
		public:

			using THIS_NET = Network0;

			const std::shared_ptr<InputContainer> inputContainer_;
			const std::shared_ptr<SpikeEpspContainer> epspContainer;
			const std::shared_ptr<Dumper> dumper_;

			#if (defined(_MSC_VER) || defined(__ICC))
			#	define MY_ALIGN __declspec(align(16))
			#else
			#	define MY_ALIGN __attribute__ ((aligned(16)))
			#endif

			MY_ALIGN SpikeEpsilon cachedEpsilon0[SpikeOptionsMasq::KERNEL_SIZE];
			MY_ALIGN SpikeEpsilon cachedEpsilon1[SpikeOptionsMasq::KERNEL_SIZE];
			MY_ALIGN SpikeEpsilon cachedEpsilon2[SpikeOptionsMasq::KERNEL_SIZE];
			MY_ALIGN SpikeEpsilon cachedEpsilon3[SpikeOptionsMasq::KERNEL_SIZE];

			MY_ALIGN SpikePotential cachedMu0[SpikeOptionsMasq::KERNEL_SIZE];
			MY_ALIGN SpikePotential cachedMu1[SpikeOptionsMasq::KERNEL_SIZE];
			MY_ALIGN SpikePotential cachedMu2[SpikeOptionsMasq::KERNEL_SIZE];
			MY_ALIGN SpikePotential cachedMu3[SpikeOptionsMasq::KERNEL_SIZE];

			MY_ALIGN SpikePotential cachedEta[SpikeOptionsMasq::KERNEL_SIZE];;
			MY_ALIGN WeightDelta cachedLtp[SpikeOptionsMasq::LTP_KERNEL_SIZE];
			MY_ALIGN WeightDelta cachedLtd[SpikeOptionsMasq::LTD_KERNEL_SIZE];

			float t_max_inMs;
			SpikeTime t_max;

			std::vector<std::shared_ptr<Neuron0<THIS_NET>>> neurons_;
			std::vector<std::vector<SpikeNeuronId>> excitatoryPath;
			std::vector<std::vector<SpikeNeuronId>> inhibitoryPath;
			std::vector<SpikeNeuronId> inputNeuronIds;
			std::vector<SpikeNeuronId> hiddenNeuronIds;

			~Network0() = default;

			Network0() = delete;

			Network0(
				const unsigned int iMax,
				const unsigned int hMax,
				const SpikeOptionsMasq& options,
				const SpikeRuntimeOptions& spikeOptions,
				const std::shared_ptr<InputContainer>& inputContainer,
				const std::shared_ptr<SpikeEpspContainer>& epspContainer,
				const std::shared_ptr<Dumper>& dumper,
				const std::shared_ptr<DumperSpikesMasquelier>& dumperSpikesMasquelier
			)
				: iMax(iMax)
				, hMax(hMax)
				, options_(options)
				, spikeRuntimeOptions_(spikeOptions)
				, inputContainer_(inputContainer)
				, epspContainer(epspContainer)
				, dumper_(dumper)
				, dumperSpikesMasquelier_(dumperSpikesMasquelier)
				, neurons_(std::vector<std::shared_ptr<Neuron0<THIS_NET>>>(static_cast<size_t>(iMax + hMax)))
				, excitatoryPath(std::vector<std::vector<SpikeNeuronId>>(static_cast<size_t>(iMax + hMax)))
				, inhibitoryPath(std::vector<std::vector<SpikeNeuronId>>(static_cast<size_t>(iMax + hMax)))
			{
				this->initCachedData();

				for (unsigned int i = 0; i < hMax; ++i)
				{
					const SpikeNeuronId hiddenNeuronId = (SpikeNeuronId)(iMax + i);
					this->neurons_[hiddenNeuronId] = std::make_unique<Neuron0<THIS_NET>>(hiddenNeuronId, (SpikeNeuronId)iMax, *this);
				}

				this->epspEvent = LAST_INF_EVENT;
			}

			Network0 & operator=(const Network0&) = delete;

			Network0(const Network0&) = delete;

			const SpikeOptionsMasq& getOptions() const
			{
				return this->options_;
			}

			SpikeEpsilon epsilon(const KernelTime x) const
			{
				//	if ((x<=0) || (x >= Neuron.kernelSize)) return 0;
				return this->cachedEpsilon0[x];
			}

			SpikePotential mu(const KernelTime x) const
			{
				//	if ((x < 0) || (x >= Neuron.kernelSize)) return 0;
				return this->cachedMu0[x];
			}

			SpikePotential eta(const KernelTime x) const
			{
				//	if ((x < 0) || (x >= options->kernelSize)) return 0;
				return this->cachedEta[x];
			}

			void initFull()
			{
				this->inputNeuronIds = std::vector<SpikeNeuronId>(this->iMax);
				for (size_t inputNeuronId = 0; inputNeuronId < this->iMax; ++inputNeuronId)
				{
					this->inputNeuronIds[inputNeuronId] = static_cast<SpikeNeuronId>(inputNeuronId);
				}

				this->hiddenNeuronIds = std::vector<SpikeNeuronId>(this->hMax);
				for (size_t i = 0; i < this->hMax; ++i)
				{
					this->hiddenNeuronIds[i] = static_cast<SpikeNeuronId>(i + iMax);
				}

				// initialize the input neurons
				for (std::vector<SpikeNeuronId>::size_type i = 0; i < this->inputNeuronIds.size(); ++i)
				{
					this->excitatoryPath[i] = this->hiddenNeuronIds;
				}

				// initialize the hidden neurons
				for (std::vector<SpikeNeuronId>::size_type i = 0; i < this->hiddenNeuronIds.size(); ++i)
				{
					const SpikeNeuronId hiddenNeuronId1 = this->hiddenNeuronIds[i];
					// set the inhibitory paths
					std::vector<SpikeNeuronId> hiddenNeuronIdsTmp = std::vector<SpikeNeuronId>();
					for (std::vector<SpikeNeuronId>::size_type j = 0; j < this->hiddenNeuronIds.size(); ++j)
					{
						const SpikeNeuronId hiddenNeuronId2 = this->hiddenNeuronIds[j];
						if (hiddenNeuronId1 != hiddenNeuronId2)
						{
							hiddenNeuronIdsTmp.push_back(static_cast<SpikeNeuronId>(hiddenNeuronId2));
						}
					}
					this->inhibitoryPath[hiddenNeuronId1] = hiddenNeuronIdsTmp;
				}
			}

			void executeInputSpikesSerial()
			{
				const bool dumpSpikeToFile = true;

				while (!this->local_noNextEvent())
				{

					const SpikeEvent inputEvent = this->inputContainer_->removeNextEvent();
					const SpikeNeuronId originatingNeuronId = getOriginatingNeuronIdFromSpikeEvent(inputEvent);

					if (dumpSpikeToFile) this->dumperSpikesMasquelier_->addNextSpike(originatingNeuronId, getTimeFromSpikeEvent(inputEvent), FiringReason::FIRE_RANDOM);

					const SpikeTime nextInputSpikeTime = getTimeFromSpikeEvent(this->inputContainer_->getNextEvent());

					#ifdef _DEBUG 
					if (true && (SpikeOptionsMasq::traceNeuronOn) && (originatingNeuronId == SpikeOptionsMasq::traceNeuronId2))
					{
						std::printf("INFO: ACS:    n=%5u spikes at %8.3fms; neuron %u is sensor neuron.\n", originatingNeuronId, static_cast<float>(nextInputSpikeTime) / SpikeOptionsMasq::TIME_DENOMINATOR, originatingNeuronId);
					}
					#endif 

					//std::cout << "SpikeNetwork::executeInputSpikesSerial(): time " << getTimeFromSpikeEvent(inputEvent) << "; originatingNeuronId="<<originatingNeuronId<< std::endl;
					bool aNeuronHasFired = false;

					const std::vector<SpikeNeuronId>& excitatoryVector = this->excitatoryPath[originatingNeuronId];
					//std::cout << "executeInputSpikesSerial: originatingNeuronId=" << originatingNeuronId << "; excitatoryVector.size()=" << excitatoryVector.size() << std::endl;

					for (std::vector<SpikeNeuronId>::size_type i = 0; i < excitatoryVector.size(); ++i)
					{
						const SpikeNeuronId destinationNeuronId = excitatoryVector[i];

						const auto& neuron = this->neurons_[destinationNeuronId];
						//std::cout << "executeInputSpikesSerial: origin=" << originatingNeuronId << "; destination=" << destinationNeuronId << std::endl;

						::tools::assert::assert_msg(neuron != NULL, "spike::v1::SpikeNetwork::executeInputSpikesSerial: B destinationNeuronId = ", destinationNeuronId, " has no associated neuron");

						//std::cout << "executeInputSpikesSerial" << std::endl;

						if (neuron->doesNeuronFire(inputEvent, nextInputSpikeTime))
						{
							aNeuronHasFired = true;
						}
						neuron->commitEpsp(getTimeFromSpikeEvent(inputEvent), originatingNeuronId); // But: the delay for the different receiving neurons is the same???
					}

					if (aNeuronHasFired)
					{
						SpikeTime firstEpspTime = LAST_INF_TIME;
						SpikeNeuronId selectedNeuronId = (SpikeNeuronId)-1;

						for (std::vector<SpikeNeuronId>::size_type i = 0; i < this->hiddenNeuronIds.size(); i++)
						{
							const SpikeNeuronId hiddenNeuronId = hiddenNeuronIds[i];
							const SpikeTime t = this->neurons_[hiddenNeuronId]->nextEpspTime;
							if (t < firstEpspTime)
							{
								firstEpspTime = t;
								selectedNeuronId = hiddenNeuronId;
							}
						}

						//std::cerr << "SpikeNetwork::executeInputSpikesSerial(): a neuron has fired: time " << firstEpspTime << "; originatingNeuronId="<<selectedNeuronId<< std::endl;

						const SpikeTime nextEventTime = getTimeFromSpikeEvent(this->inputContainer_->getNextEvent());
						if (firstEpspTime <= nextEventTime)
						{
							const auto neuron = this->neurons_[selectedNeuronId];
							::tools::assert::assert_msg(neuron != NULL, "SpikeNetwork::executeInputSpikesSerial(): B selectedNeuronId=", selectedNeuronId, " has no associated neuron");

							neuron->nextEpspTime = LAST_INF_TIME;
							neuron->fire(firstEpspTime);

							if (dumpSpikeToFile) this->dumperSpikesMasquelier_->addNextSpike(selectedNeuronId, firstEpspTime, FiringReason::FIRE_PROPAGATED);

							const std::vector<SpikeNeuronId> inhibitoryVector = this->inhibitoryPath[selectedNeuronId];
							for (std::vector<SpikeNeuronId>::size_type i = 0; i < inhibitoryVector.size(); ++i)
							{
								const SpikeNeuronId destinationNeuronId = inhibitoryVector[i];
								const auto neuron2 = this->neurons_[destinationNeuronId];
								::tools::assert::assert_msg(neuron2 != NULL, "SpikeNetwork::executeInputSpikesSerial(): A selectedNeuronId=", selectedNeuronId, " has no associated neuron");

								#if _DEBUG
								const KernelTime kernelTime = (firstEpspTime >> (SpikeOptionsMasq::TIME_DENOMINATOR_POW - SpikeOptionsMasq::KERNEL_SCAN_INTERVAL_POW));
								const int startIndex = kernelTime - neuron2->futurePotentialStartKernelTime;
								//	log.info("commitEpsp(): neuron "+this.id+" kernelTime="+kernelTime+"; futureEpspStartKernelTime="+futureEpspStartKernelTime+"; index="+index);
								if (startIndex < 0)
								{
									std::cerr << "SpikeNeuron::commitIpsp() startIndex=" << startIndex << "futurePotentialStartKernelTime=" << neuron->futurePotentialStartKernelTime << "; kernelTime=" << kernelTime << "; firstEpspTime=" << firstEpspTime << std::endl;
									throw 1;
								}
								#endif
								neuron2->commitIpsp(firstEpspTime);
							}
						}
					}
				}
			}

			void printCachedKernel()
			{
				const unsigned int longestKernelTime = std::max(SpikeOptionsMasq::KERNEL_SIZE, std::max(SpikeOptionsMasq::LTP_KERNEL_SIZE, SpikeOptionsMasq::LTP_KERNEL_SIZE));

				for (unsigned int kernelTime = 0; kernelTime < (longestKernelTime - 1); kernelTime++)
				{
					const float time = static_cast<float>(kernelTime) / SpikeOptionsMasq::TIME_DENOMINATOR;

					float epsilon = nan;
					float mu = nan;
					float eta = nan;
					float ltp = nan;
					float ltd = nan;

					if (kernelTime < SpikeOptionsMasq::KERNEL_SIZE) epsilon = static_cast<float>(this->cachedEpsilon0[kernelTime]) / SpikeOptionsMasq::POTENTIAL_DENOMINATOR;
					if (kernelTime < SpikeOptionsMasq::KERNEL_SIZE) mu = static_cast<float>(this->cachedMu0[kernelTime]) / SpikeOptionsMasq::POTENTIAL_DENOMINATOR;
					if (kernelTime < SpikeOptionsMasq::KERNEL_SIZE) eta = static_cast<float>(this->cachedEta[kernelTime]) / SpikeOptionsMasq::POTENTIAL_DENOMINATOR;
					if (kernelTime < SpikeOptionsMasq::LTP_KERNEL_SIZE) ltp = static_cast<float>(this->cachedLtp[kernelTime]) / SpikeOptionsMasq::POTENTIAL_DENOMINATOR;
					if (kernelTime < SpikeOptionsMasq::LTD_KERNEL_SIZE) ltd = static_cast<float>(this->cachedLtd[kernelTime]) / SpikeOptionsMasq::POTENTIAL_DENOMINATOR;

					std::printf("time=%7.3f; epsilon=%17.10f; mu=%17.10f; eta=%17.10f; ltd=%15.10f; ltp=%15.10f\n", time, epsilon, mu, eta, ltp, ltd);
				}
			}

		private:

			const std::shared_ptr<DumperSpikesMasquelier> dumperSpikesMasquelier_;

			const unsigned int iMax;
			const unsigned int hMax;

			const SpikeOptionsMasq options_;
			const SpikeRuntimeOptions spikeRuntimeOptions_;

			SpikeEvent epspEvent;

			float epsilon_f(const float x) const
			{
				if ((x < 0) || (x > SpikeOptionsMasq::T_MAX_OPTIMIZATION)) return 0;
				return (this->options_.k * (exp(-x / SpikeOptionsMasq::tau_m) - exp(-x / this->options_.tau_s)));
			}

			SpikeEpsilon epsilon_o(const float x) const
			{
				const int realValue = std::lroundf(this->epsilon_f(x) * SpikeOptionsMasq::POTENTIAL_DENOMINATOR);
				const SpikePotential returnValue = (SpikePotential)realValue;
				if (abs(realValue - returnValue) > 1)
				{
					std::cerr << "SpikeNetwork::epsilon_o(): realValue=" << realValue << "; returnValue=" << returnValue << std::endl;
				}
				return static_cast<SpikeEpsilon>(returnValue);
			}

			float mu_f(const float x) const
			{
				if ((x < 0) || (x > SpikeOptionsMasq::T_MAX_OPTIMIZATION)) return 0;
				return -options_.alpha * options_.threshold * this->epsilon_f(x);
			}

			SpikePotential mu_o(const float x) const
			{
				const int realValue = std::lroundf(this->mu_f(x) * SpikeOptionsMasq::POTENTIAL_DENOMINATOR);
				const SpikePotential returnValue = (SpikePotential)realValue;
				if (abs(realValue - returnValue) > 1)
				{
					std::cerr << "SpikeNetwork::mu_o(): realValue=" << realValue << "; returnValue=" << returnValue << std::endl;
				}
				return returnValue;
			}

			float eta_f(const float x) const
			{
				if ((x < 0) || (x > SpikeOptionsMasq::T_MAX_OPTIMIZATION)) return 0;
				return (this->options_.threshold * (this->options_.k1 * expf(-x / SpikeOptionsMasq::tau_m) - this->options_.k2 * (expf(-x / SpikeOptionsMasq::tau_m) - expf(-x / this->options_.tau_s))));
			}

			SpikePotential eta_o(const float x) const
			{
				const int realValue = std::lroundf(eta_f(x) * SpikeOptionsMasq::POTENTIAL_DENOMINATOR);
				const SpikePotential returnValue = (SpikePotential)realValue;
				if (abs(realValue - returnValue) > 1)
				{
					std::cerr << "SpikeNetwork::mu_o(): realValue=" << realValue << "; returnValue=" << returnValue << std::endl;
				}
				return returnValue;
			}

			bool local_noNextEvent() const
			{
				return this->inputContainer_->isEmpty();
			}

			void initCachedData()
			{

				for (int i = 0; i < (SpikeOptionsMasq::KERNEL_SIZE - 4); i++)
				{
					const float timeInMs0 = (float)i / SpikeOptionsMasq::KERNEL_SCAN_INTERVAL;
					const float timeInMs1 = (float)(i + 1) / SpikeOptionsMasq::KERNEL_SCAN_INTERVAL;
					const float timeInMs = (timeInMs0 + timeInMs1) / 2;
					const SpikeEpsilon value = this->epsilon_o(timeInMs);
					this->cachedEpsilon0[i + 0] = value;
					this->cachedEpsilon1[i + 1] = value;
					this->cachedEpsilon2[i + 2] = value;
					this->cachedEpsilon3[i + 3] = value;
					//	if ((this->cachedEpsilon[time] == 0) && (time > 0)) log.warn("constructor cachedEpsilon["+time+"]=0");
				}
				this->cachedEpsilon0[SpikeOptionsMasq::KERNEL_SIZE - 1] = 0;
				this->cachedEpsilon0[SpikeOptionsMasq::KERNEL_SIZE - 2] = 0;
				this->cachedEpsilon0[SpikeOptionsMasq::KERNEL_SIZE - 3] = 0;
				this->cachedEpsilon0[SpikeOptionsMasq::KERNEL_SIZE - 4] = 0;

				this->cachedEpsilon1[0] = 0;
				this->cachedEpsilon1[SpikeOptionsMasq::KERNEL_SIZE - 1] = 0;
				this->cachedEpsilon1[SpikeOptionsMasq::KERNEL_SIZE - 2] = 0;
				this->cachedEpsilon1[SpikeOptionsMasq::KERNEL_SIZE - 3] = 0;

				this->cachedEpsilon2[0] = 0;
				this->cachedEpsilon2[1] = 0;
				this->cachedEpsilon2[SpikeOptionsMasq::KERNEL_SIZE - 1] = 0;
				this->cachedEpsilon2[SpikeOptionsMasq::KERNEL_SIZE - 2] = 0;

				this->cachedEpsilon3[0] = 0;
				this->cachedEpsilon3[1] = 0;
				this->cachedEpsilon3[2] = 0;
				this->cachedEpsilon3[SpikeOptionsMasq::KERNEL_SIZE - 1] = 0;


				/////////////////////////////////////////////////////////////////
				// init MU

				for (int i = 0; i < (SpikeOptionsMasq::KERNEL_SIZE - 4); i++)
				{
					const float timeInMs0 = (float)i / SpikeOptionsMasq::KERNEL_SCAN_INTERVAL;
					const float timeInMs1 = (float)(i + 1) / SpikeOptionsMasq::KERNEL_SCAN_INTERVAL;
					const float timeInMs = (timeInMs0 + timeInMs1) / 2;
					const SpikePotential value = this->mu_o(timeInMs);
					this->cachedMu0[i + 0] = value;
					this->cachedMu1[i + 1] = value;
					this->cachedMu2[i + 2] = value;
					this->cachedMu3[i + 3] = value;
				}
				this->cachedMu0[SpikeOptionsMasq::KERNEL_SIZE - 1] = 0;
				this->cachedMu0[SpikeOptionsMasq::KERNEL_SIZE - 2] = 0;
				this->cachedMu0[SpikeOptionsMasq::KERNEL_SIZE - 3] = 0;
				this->cachedMu0[SpikeOptionsMasq::KERNEL_SIZE - 4] = 0;

				this->cachedMu1[0] = 0;
				this->cachedMu1[SpikeOptionsMasq::KERNEL_SIZE - 1] = 0;
				this->cachedMu1[SpikeOptionsMasq::KERNEL_SIZE - 2] = 0;
				this->cachedMu1[SpikeOptionsMasq::KERNEL_SIZE - 3] = 0;

				this->cachedMu2[0] = 0;
				this->cachedMu2[1] = 0;
				this->cachedMu2[SpikeOptionsMasq::KERNEL_SIZE - 1] = 0;
				this->cachedMu2[SpikeOptionsMasq::KERNEL_SIZE - 2] = 0;

				this->cachedMu3[0] = 0;
				this->cachedMu3[1] = 0;
				this->cachedMu3[2] = 0;
				this->cachedMu3[SpikeOptionsMasq::KERNEL_SIZE - 1] = 0;


				/////////////////////////////////////////////////////////////////
				for (int i = 0; i < SpikeOptionsMasq::KERNEL_SIZE; i++)
				{
					const float timeInMs0 = (float)i / SpikeOptionsMasq::KERNEL_SCAN_INTERVAL;
					const float timeInMs1 = (float)(i + 1) / SpikeOptionsMasq::KERNEL_SCAN_INTERVAL;
					const float timeInMs = (timeInMs0 + timeInMs1) / 2;
					this->cachedEta[i] = this->eta_o(timeInMs);
					//if ((this->cachedEta[time] == 0) && (time > 0)) log.warn("constructor cachedEta["+time+"]=0");
				}

				// initialize cached ltp / ltd kernels
				for (int i = 0; i < SpikeOptionsMasq::LTP_KERNEL_SIZE; i++)
				{
					const float timeInMs = (float)i / SpikeOptionsMasq::TIME_DENOMINATOR;
					const float trueValue = spike::tools::stdp(0, timeInMs, this->options_.alpha_plus, this->options_.alpha_minus, SpikeOptionsMasq::tau_plus, SpikeOptionsMasq::tau_minus);
					const int roundedValue = std::lroundf(trueValue*SpikeOptionsMasq::POTENTIAL_DENOMINATOR);
					#if _DEBUG 
					if (roundedValue < 0)
					{
						std::cout << "SpikeNeuron::initCachedData ltp: wDelta= " << roundedValue << "; this may happen" << std::endl; throw 1;
					}
					#endif
					this->cachedLtp[i] = static_cast<WeightDelta>(roundedValue);
				}
				for (int i = 0; i < SpikeOptionsMasq::LTD_KERNEL_SIZE; i++)
				{
					const float timeInMs = (float)i / SpikeOptionsMasq::TIME_DENOMINATOR;
					const float trueValue = spike::tools::stdp(timeInMs, 0, this->options_.alpha_plus, this->options_.alpha_minus, SpikeOptionsMasq::tau_plus, SpikeOptionsMasq::tau_minus);
					const int roundedValue = std::lroundf(trueValue*SpikeOptionsMasq::POTENTIAL_DENOMINATOR);
					#if _DEBUG 
					if (roundedValue > 0)
					{
						std::cout << "SpikeNeuron::initCachedData ltd: i=" << i << "; timeInMs=" << timeInMs << "; wDelta=" << roundedValue << "; this may happen" << std::endl; throw 1;
					}
					#endif
					this->cachedLtd[i] = static_cast<WeightDelta>(roundedValue);
				}
			}
		};
	}
}