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

#include <vector>
#include <tuple>
#include <string>
#include <algorithm>    // std::sort
#include <memory>

#include "../../Spike-Tools-LIB/DumperState.hpp"
#include "../../Spike-Tools-LIB/SpikeRuntimeOptions.hpp"
#include "../../Spike-Tools-LIB/random.ipp"
#include "../../Spike-Tools-LIB/stats.ipp"

#include "SpikeOptionsStatic.hpp"
#include "Types.hpp"

namespace spike
{
	namespace v3
	{
		namespace experiment
		{
			using namespace spike::tools;

			namespace local
			{
				struct IncommingPotential
				{
					float timeInMs;
					float potential;

					// default constructor
					IncommingPotential()
						: timeInMs(-1)
						, potential(0)
					{
					}

					// constructor
					IncommingPotential(
						const float timeInMs,
						const float potential)
						: timeInMs(timeInMs)
						, potential(potential)
					{
					}

					bool operator< (const IncommingPotential& spike) const
					{
						if (this->timeInMs < spike.timeInMs)
						{
							return true;
						}
						else if (this->timeInMs > spike.timeInMs)
						{
							return false;
						}
						else
						{
							return (this->potential > spike.potential);
						}
					}
					bool operator== (const IncommingPotential& spike) const
					{
						return ((this->timeInMs == spike.timeInMs) && (this->potential == spike.potential));
					}
					bool operator!= (const IncommingPotential& spike) const
					{
						return ((this->timeInMs != spike.timeInMs) || (this->potential != spike.potential));
					}
				};

				template <typename Options>
				Voltage calcVoltage(
					const float timeInMs,
					const std::vector<IncommingPotential>& incommingExcPotential,
					const std::vector<IncommingPotential>& incommingInhPotential,
					const std::vector<float>& spikeTimes,
					const Options& options)
				{
					const float lastSpikeTime = spikeTimes.back();
					const float endLastRefractoryPeriod = lastSpikeTime + static_cast<float>(Options::refractoryPeriod);

					if (timeInMs < endLastRefractoryPeriod)
					{
						return Options::minVoltage;
					}

					Voltage voltage = 0;

					const float lastSpikeTimeRelative = timeInMs - lastSpikeTime;
					if (lastSpikeTimeRelative < Options::kernelRangeEtaInMs)
					{
						voltage = options.eta_f(lastSpikeTimeRelative);
					}

					for (const IncommingPotential& ip : incommingInhPotential)
					{
						if (ip.timeInMs >= endLastRefractoryPeriod)
						{
							const float incommingTimeRelative = timeInMs - ip.timeInMs;
							if (incommingTimeRelative >= 0)
							{
								if (incommingTimeRelative < Options::kernelRangeEpsilonInMs)
								{
									const Voltage mu = options.mu_f(incommingTimeRelative);
									const Voltage delta = 100 * ip.potential * mu;

									if (delta > 0) __debugbreak();

									voltage += delta;
								}
							}
							else
							{
								//std::cout << "calcVoltage: neuronId=" << neuronId << "; incommingSpike.kerneltime=" << incommingSpike.kerneltime << "; kerneltime=" << kerneltime << std::endl;
								break;
							}
						}
					}

					for (const IncommingPotential& ip : incommingExcPotential)
					{
						if (ip.timeInMs >= endLastRefractoryPeriod)
						{
							const float incommingTimeRelative = timeInMs - ip.timeInMs;
							if (incommingTimeRelative >= 0)
							{
								if (incommingTimeRelative < Options::kernelRangeEpsilonInMs)
								{
									const Voltage epsilon = options.epsilon_f(incommingTimeRelative);
									const Voltage delta = ip.potential * epsilon;

									if (delta < 0) __debugbreak();

									voltage += delta;
								}
							}
							else
							{
								//std::cout << "calcVoltage: neuronId=" << neuronId << "; incommingSpike.kerneltime=" << incommingSpike.kerneltime << "; kerneltime=" << kerneltime << std::endl;
								break;
							}
						}
					}
					//std::cout << "calcVoltage: time = "<< timeInMs << ": returning voltage " << voltage << std::endl;
					return voltage;
				}

				float getSpikeTimeIncInMs(const float spikeHz)
				{
					const float z = (1000 * 2) / spikeHz;
					return ::tools::random::rand_float(z);
				}

				std::vector<IncommingPotential> createExcitatoryPotential(
					const float simulationTimeInMs,
					const size_t nSensorNeurons,
					const float spikeHz,
					const std::vector<float>& delay,
					const std::vector<float>& weight)
				{
					std::vector<local::IncommingPotential> incommingPotential;
					for (unsigned int neuronId = 0; neuronId < nSensorNeurons; ++neuronId)
					{
						for (float nextSpikeTimeInMs = local::getSpikeTimeIncInMs(spikeHz); nextSpikeTimeInMs < simulationTimeInMs; nextSpikeTimeInMs += local::getSpikeTimeIncInMs(spikeHz))
						{
							incommingPotential.push_back(local::IncommingPotential(nextSpikeTimeInMs + delay[neuronId], weight[neuronId]));
						}
					}
					std::sort(incommingPotential.begin(), incommingPotential.end());
					return incommingPotential;
				}

				std::vector<IncommingPotential> createInhibitoryPotential(
					const std::vector<float>& spikeTimes,
					const float weight,
					const float delay)
				{
					std::vector<local::IncommingPotential> incommingPotential;
					for (const float spikeTime : spikeTimes)
					{
						incommingPotential.push_back(local::IncommingPotential(spikeTime + delay, weight));
					}
					return incommingPotential;
				}

				std::tuple<std::vector<float>, std::vector<float>> createNetwork(const size_t nSensorNeurons)
				{
					std::vector<float> delay(nSensorNeurons);
					std::vector<float> weight(nSensorNeurons);
					for (size_t i = 0; i < nSensorNeurons; ++i)
					{
						delay[i] = static_cast<float>(::tools::random::rand_int32(1, 20));
						weight[i] = ::tools::random::rand_float(1.0); // weigth between 0 and 1
					}
					return std::make_tuple(delay, weight);
				}

				// calculate the spike times given random inputs
				template <typename Options, typename DumperState>
				std::vector<float> getSpikeTimes(
					const std::vector<IncommingPotential>& incommingExcPotential,
					const std::vector<IncommingPotential>& incommingInhPotential,
					const float simulationTimeInMs,
					const float simulationDeltaInMs,
					const float threshold,
					const bool dumperStateOn,
					DumperState& dumperState,
					const unsigned int traceNeuronId)
				{
					const Options options;

					// calculate when the neuron fires.
					std::vector<float> spikeTimes;
					spikeTimes.push_back(-100000); // add one dummy spike from a very far past to make the algorithm smoother.

					for (float currentTime = 0; currentTime < simulationTimeInMs; currentTime += simulationDeltaInMs)
					{
						const Voltage v = local::calcVoltage(currentTime, incommingExcPotential, incommingInhPotential, spikeTimes, options);
						if (v >= threshold)
						{
							//std::cout << "spike::v3::experiment::experiment1: spike at " << currentTime << std::endl;
							spikeTimes.push_back(currentTime);
						}
						if (dumperStateOn) dumperState.store(traceNeuronId, currentTime, v, threshold);
					}

					spikeTimes.erase(spikeTimes.begin()); // remove the first dummy spike
					return spikeTimes;
				}
			}

			template <typename Options>
			void experiment1(const SpikeRuntimeOptions& spikeRuntimeOptions)
			{
				const float threshold = 50;
				const size_t nSensorNeurons = 1000;
				const float spikeHz = 10;

				const float simulationTimeInMs = 1000;
				const float simulationDeltaInMs = 0.01;

				const size_t nSamples = 10;
				std::vector<float> nSpikes(nSamples);

				for (size_t i = 0; i < nSamples; ++i)
				{
					printf("spike::v3::experiment::experiment1: sample %lu.\n", i);

					// create network
					const auto result = local::createNetwork(nSensorNeurons);
					const std::vector<float> delay = std::get<0>(result);
					const std::vector<float> weight = std::get<1>(result);

					// create the incomming potential
					const std::vector<IncommingPotential> incommingPotential = local::createIncommingPotential(simulationTimeInMs, nSensorNeurons, spikeHz, delay, weight);

					// calculate the spiketimes
					const std::vector<float> spikeTimes = local::getSpikeTimes<Options>(incommingPotential, simulationTimeInMs, simulationDeltaInMs, threshold, nSensorNeurons, spikeHz);
					nSpikes[i] = spikeTimes.size();
				}
				std::cout << "experiment1: nSensorNeurons=" << nSensorNeurons << "; spikeHz=" << spikeHz << "; threshold = " << threshold << "; mean = " << ::tools::stats::mean(nSpikes) << "; stdev = " << ::tools::stats::stdev(nSpikes) << std::endl;
			}


			template <typename Options>
			void experiment5(const SpikeRuntimeOptions& spikeRuntimeOptions)
			{
				const float threshold = 70;
				const size_t nSensorNeurons = 1000;
				const float spikeHz = 10;

				const float simulationTimeInMs = 1000;
				const float simulationDeltaInMs = 0.01;
				const float inhibitionDelayInMs = 1;

				const size_t nSamples = 1;
				std::vector<float> nSpikes2(nSamples);
				std::vector<float> nSpikes3(nSamples);
				std::vector<float> nOverlaps2(nSamples);
				std::vector<float> nOverlaps3(nSamples);

				for (size_t i = 0; i < nSamples; ++i)
				{
					printf("spike::v3::experiment::experiment5: sample %zu.\n", i);
					const auto results = experiment5_getSpikeTimes<Options>(inhibitionDelayInMs, simulationTimeInMs, simulationDeltaInMs, threshold, nSensorNeurons, spikeHz, spikeRuntimeOptions);
					nSpikes2[i] = static_cast<float>(results[0]);
					nOverlaps2[i] = static_cast<float>(results[1]);
					nSpikes3[i] = static_cast<float>(results[2]);
					nOverlaps3[i] = static_cast<float>(results[3]);
				}

				std::cout << "experiment5: nSensorNeurons=" << nSensorNeurons << "; spikeHz=" << spikeHz << "; threshold = " << threshold << std::endl;
				std::cout << "spikes2:  mean = " << ::tools::stats::mean(nSpikes2) << "; stdev = " << ::tools::stats::stdev(nSpikes2) << std::endl;
				std::cout << "overlap2: mean = " << ::tools::stats::mean(nOverlaps2) << "; stdev = " << ::tools::stats::stdev(nOverlaps2) << std::endl;
				std::cout << "spikes3:  mean = " << ::tools::stats::mean(nSpikes3) << "; stdev = " << ::tools::stats::stdev(nSpikes3) << std::endl;
				std::cout << "overlap3: mean = " << ::tools::stats::mean(nOverlaps3) << "; stdev = " << ::tools::stats::stdev(nOverlaps3) << std::endl;
			}


			// get 1] the spikes count; 2] spike count if preceded by an incomming spike within time interval t
			template <typename Options>
			std::array<unsigned int, 4> experiment5_getSpikeTimes(
				const float inhibitionDelayInMs,
				const float simulationTimeInMs,
				const float simulationDeltaInMs,
				const float threshold,
				const size_t nSensorNeurons,
				const float spikeHz,
				const SpikeRuntimeOptions& spikeRuntimeOptions)
			{
				const bool dumperStateOn = true;
				DumperState<float, Voltage, Options> dumperState(spikeRuntimeOptions);
				const float inhWeight = -1;

				// create network and spike potentials
				const auto result1 = local::createNetwork(nSensorNeurons);
				const std::vector<float> delay1 = std::get<0>(result1);
				const std::vector<float> weight1 = std::get<1>(result1);
				const std::vector<local::IncommingPotential> excPotIn1 = local::createExcitatoryPotential(simulationTimeInMs, nSensorNeurons, spikeHz, delay1, weight1);

				// create network and spike potentials
				const auto result2 = local::createNetwork(nSensorNeurons);
				const std::vector<float> delay2 = std::get<0>(result2);
				const std::vector<float> weight2 = std::get<1>(result2);
				const std::vector<local::IncommingPotential> excPotIn2 = local::createExcitatoryPotential(simulationTimeInMs, nSensorNeurons, spikeHz, delay2, weight2);

				// calculate the spiketimes
				const std::vector<local::IncommingPotential> inhSpikesEmpty;
				const std::vector<float> spikeTimes1 = local::getSpikeTimes<Options>(excPotIn1, inhSpikesEmpty, simulationTimeInMs, simulationDeltaInMs, threshold, dumperStateOn, dumperState, 0);
				const std::vector<float> spikeTimes2 = local::getSpikeTimes<Options>(excPotIn2, inhSpikesEmpty, simulationTimeInMs, simulationDeltaInMs, threshold, dumperStateOn, dumperState, 1);

				const std::vector<local::IncommingPotential> inhSpikes = local::createInhibitoryPotential(spikeTimes1, inhWeight, inhibitionDelayInMs);
				const std::vector<float> spikeTimes3 = local::getSpikeTimes<Options>(excPotIn2, inhSpikes, simulationTimeInMs, simulationDeltaInMs, threshold, dumperStateOn, dumperState, 2);

				if (dumperStateOn) dumperState.dump(0, "experiment");

				unsigned int nOverlaps2 = 0;
				unsigned int nOverlaps3 = 0;

				for (const float spikeTime1 : spikeTimes1)
				{
					for (const float spikeTime2 : spikeTimes2)
					{
						if ((spikeTime1 < spikeTime2) && (spikeTime1 >= (spikeTime2 - inhibitionDelayInMs)))
						{
							//std::cout << "spike::v3::experiment:experiment5_getSpikeTimes: found overlap: t1=" << spikeTime1 << "; t2=" << spikeTime2 << std::endl;
							nOverlaps2++;
						}
					}
					for (const float spikeTime3 : spikeTimes3)
					{
						if ((spikeTime1 < spikeTime3) && (spikeTime1 >= (spikeTime3 - inhibitionDelayInMs)))
						{
							//std::cout << "spike::v3::experiment:experiment5_getSpikeTimes: found overlap: t1=" << spikeTime1 << "; t2=" << spikeTime2 << std::endl;
							nOverlaps3++;
						}
					}
				}
				const unsigned int nSpikes2 = static_cast<unsigned int>(spikeTimes2.size());
				const unsigned int nSpikes3 = static_cast<unsigned int>(spikeTimes3.size());
				return{ nSpikes2, nOverlaps2, nSpikes3, nOverlaps3 };
			}
		}
	}
}