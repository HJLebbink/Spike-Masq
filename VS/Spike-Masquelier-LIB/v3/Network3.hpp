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
#include <memory>
#include <ratio>
#include <sstream>		// for std::ostringstream
#include <iostream>     // std::cout, std::fixed
#include <iomanip>      // std::setprecision
#include <math.h>		// for log2
#include <limits>		// std::numeric_limits

#include "omp.h"

#include "../../Spike-DataSet-LIB/Translations.hpp"

#include "../../Spike-Tools-LIB/DumperSpikes.hpp"
#include "../../Spike-Tools-LIB/DumperState.hpp"
#include "../../Spike-Tools-LIB/DumperTopology.hpp"
#include "../../Spike-Tools-LIB/SpikeNetworkPerformance.hpp"

#include "Types.hpp"
#include "SpikeOptionsStatic.hpp"
#include "Topology.hpp"
#include "Synapses.hpp"
#include "IncommingSpikeQueue.hpp"
#include "SpikeHistory.hpp"
#include "SpikeStreamDataSet.hpp"

namespace spike
{
	namespace v3
	{
		using namespace ::spike::tools;

		template <typename Topology_i, typename SpikeStream_i>
		struct State
		{
			using Topology = Topology_i;
			using SpikeStream = SpikeStream_i;
			using Options = typename Topology_i::Options;

			Options options_;

			std::shared_ptr<SpikeStream> spikeStream_;

			TimeInSec currentTimeInSec_;

			TimeInMs lastTimeStateDumped_ = 0;
			DumperState<TimeInMs, Voltage, Options> dumperState_;
			DumperSpikes<TimeInMs> dumperSpikes_;
			SpikeSet1Sec<TimeInMs> spikeSet_;
			DumperTopology<Topology> dumperTopology_;
			SpikeNetworkPerformance<Topology, TimeInMs> spikeNetworkPerformance_;

			std::shared_ptr<Topology> topology_;
			Synapses<Topology> synapses_;

			IncommingSpikeQueue<Topology> incommingSpikes_;
			std::array<PostSynapticSpike, Options::nNeurons> nextRandomPostSynapticSpike_;

			std::array<KernelTime, Options::nNeurons> lastSpikeTime_;
			SpikeHistory4<Topology> endRefractoryPeriods_;

			std::vector<Voltage> cachedThreshold_;
			std::vector<Voltage> cachedEpsilon_;
			std::vector<Voltage> cachedEta_;
			std::vector<float> cachedLtp_;
			std::vector<float> cachedLtd_;

			size_t nSpikesPropagatedLastSec_; // number of spikes of type propagated in the current second
			size_t nSpikesRandomLastSec_; // number of spikes of type random in the current second

			// constructor
			State() = delete;

			State(const State&)
			{
			}

			// constructor
			State(
				const Options& options,
				const SpikeRuntimeOptions& spikeRuntimeOptions
			)
				: options_(options)
				, dumperSpikes_(DumperSpikes<TimeInMs>(spikeRuntimeOptions))
				, dumperState_(DumperState<TimeInMs, Voltage, Options>(spikeRuntimeOptions))
				, dumperTopology_(DumperTopology<Topology>(spikeRuntimeOptions))

				, cachedThreshold_(std::vector<Voltage>(static_cast<size_t>(Options::toKernelTime(static_cast<TimeInMs>(Options::kernelRangeThresholdInMs)))))
				, cachedEpsilon_(std::vector<Voltage>(static_cast<size_t>(Options::toKernelTime(static_cast<TimeInMs>(Options::kernelRangeEpsilonInMs)))))
				, cachedEta_(std::vector<Voltage>(static_cast<size_t>(Options::toKernelTime(static_cast<TimeInMs>(Options::kernelRangeEtaInMs)))))
				, cachedLtp_(std::vector<float>(static_cast<size_t>(Options::toKernelTime(static_cast<TimeInMs>(Options::kernelRangeStdpInMs)))))
				, cachedLtd_(std::vector<float>(static_cast<size_t>(Options::toKernelTime(static_cast<TimeInMs>(Options::kernelRangeStdpInMs)))))
			{
				this->lastSpikeTime_.fill(Options::toKernelTime(-1000));
				this->initCachedData();
			}

			// copy assignment operator
			State& operator= (const State& rhs)
			{
				std::cout << "State::copy assignment" << std::endl;
				this->nElements_ = rhs.nElements_;
				this->selectionSize_ = rhs.selectionSize_;
				this->indices_ = rhs.indices_;
				this->atEnd_ = rhs.atEnd_;
				return *this;	// by convention, always return *this
			}


			void initCachedData()
			{
				for (KernelTime i = 0; i < Options::toKernelTime(Options::kernelRangeEtaInMs); ++i)
				{
					const TimeInMs t = Options::toTimeInMs(i);
					this->cachedEta_[i] = this->options_.eta_f(t);
				}
				for (KernelTime i = 0; i < Options::toKernelTime(Options::kernelRangeStdpInMs); ++i)
				{
					const TimeInMs t = Options::toTimeInMs(i);
					this->cachedLtd_[i] = this->options_.calcWeightDeltaLtd(0, t);
					this->cachedLtp_[i] = this->options_.calcWeightDeltaLtp(0, t);
				}
				for (KernelTime i = 0; i < Options::toKernelTime(Options::kernelRangeThresholdInMs); ++i)
				{
					const TimeInMs t = Options::toTimeInMs(i);
					this->cachedThreshold_[i] = this->options_.threshold_f(t);
				}
				for (KernelTime i = 0; i < Options::toKernelTime(Options::kernelRangeEpsilonInMs); ++i)
				{
					const TimeInMs t = Options::toTimeInMs(i);
					this->cachedEpsilon_[i] = this->options_.epsilon_f(t);
				}

				// make the cached data continue at the last moment, ie. make them zero for the last time moment.
				const Voltage lastEta = this->cachedEta_[Options::toKernelTime(Options::kernelRangeEtaInMs) - 1];
				const Voltage lastEpsilon = this->cachedEpsilon_[Options::toKernelTime(Options::kernelRangeEpsilonInMs) - 1];
				const Voltage lastThreshold = this->cachedThreshold_[Options::toKernelTime(Options::kernelRangeThresholdInMs) - 1];

				for (KernelTime i = 0; i < Options::toKernelTime(Options::kernelRangeEtaInMs); ++i)
				{
					this->cachedEta_[i] -= lastEta;
				}
				for (KernelTime i = 0; i < Options::toKernelTime(Options::kernelRangeEpsilonInMs); ++i)
				{
					this->cachedEpsilon_[i] -= lastEpsilon;
					if (this->cachedEpsilon_[i] < 0) this->cachedEpsilon_[i] = 0;
				}
				for (KernelTime i = 0; i < Options::toKernelTime(Options::kernelRangeThresholdInMs); ++i)
				{
					this->cachedThreshold_[i] = this->cachedThreshold_[i] - lastThreshold + 1.0f;
				}
			}
		};



		template <typename Topology_i, typename SpikeStream_i>
		class Network3
		{
		public:

			using Topology = Topology_i;
			using SpikeStream = SpikeStream_i;
			using Options = typename Topology_i::Options;

			// constructor
			Network3()
				: state_(State<Topology, SpikeStream>())
			{
			}

			// constructor
			Network3(
				const Options& options,
				const SpikeRuntimeOptions& SpikeRuntimeOptions
			)
				: state_(State<Topology, SpikeStream>(options, SpikeRuntimeOptions))
			{
			}


			void setSpikeStream(const std::shared_ptr<SpikeStream>& spikeStream)
			{
				this->state_.spikeStream_ = spikeStream;
			}

			void setTopology(const std::shared_ptr<Topology>& topology)
			{
				this->state_.topology_ = topology;
				this->state_.synapses_.init(topology);
			}

			const std::shared_ptr<const Topology> getTopology() const
			{
				this->updatePathways(this->state_.topology_);
				return this->state_.topology_;
			}

			void mainLoop(const TimeInSec nSeconds, const bool useConfusionMatrix)
			{
				KernelTime currentTime = 0;

				const KernelTime minDelay = Options::toKernelTime(static_cast<TimeInMs>(this->state_.options_.minDelay));
				::tools::assert::assert_msg(minDelay < (Options::toKernelTime(Options::refractoryPeriod)), "minDelay has to be smaller than the refractory period");

				//this->state_.spikeStream_->start();

				for (const NeuronId& neuronId : Topology::iterator_AllNeurons())
				{
					Network3::updateNextRandomPostSynapticSpike(this->state_, neuronId, currentTime);
				}

				for (TimeInSec sec = 0; sec < nSeconds; ++sec)
				{
					const clock_t t1 = clock();

					{	// reset non-essential reporting counters 
						this->state_.nSpikesPropagatedLastSec_ = 0;
						this->state_.nSpikesRandomLastSec_ = 0;
					}
					{	// check for KernelTime overflow
						if (currentTime > 1000000000)
						{
							const TimeInSec t2 = static_cast<TimeInSec>(currentTime / (Options::nSubMs * 1000));
							const KernelTime t3 = Options::toKernelTime(static_cast<TimeInMs>(1000 * t2));
							this->state_.currentTimeInSec_ += t2;
							currentTime -= t3;
							this->substractTime(t3);
						}
					}

					const bool dumpSpikes = this->state_.dumperSpikes_.dumpTest(sec);
					const bool dumpState = this->state_.dumperState_.dumpTest(sec);
					{	// reset dump state
						if (dumpSpikes) this->state_.spikeSet_.clear();
						if (dumpState) this->state_.dumperState_.clear();
					}

					KernelTime currentTimeSubSecond = currentTime % Options::toKernelTime(1000);
					//std::cout << "spike::v3::Network3::mainLoop: A: currentTimeSubSecond=" << currentTimeSubSecond << "; sec=" << sec << "; currentTime=" << currentTime << std::endl;

					while (currentTimeSubSecond < (Options::toKernelTime(1000)))
					{

						const KernelTime maxAdvanceTime = currentTime + minDelay;
						this->advanceTime(maxAdvanceTime);

						this->findAndFireNeuronA(currentTime, maxAdvanceTime, dumpSpikes, dumpState);
						currentTime += minDelay;
						currentTimeSubSecond += minDelay;

						this->state_.spikeStream_->advanceCurrentTime(minDelay);
						//std::cout << "spike::v3::Network3::mainLoop: B: currentTimeThisSecond=" << currentTimeThisSecond << "; sec=" << sec << "; currentTime=" << currentTime << "; maxAdvanceTime=" << maxAdvanceTime << std::endl;
					}
					{
						{
							if (dumpSpikes) this->state_.dumperSpikes_.dump(sec, "train", this->state_.spikeSet_, this->state_.spikeStream_->getCaseUsage());
							if (useConfusionMatrix)
							{
								const auto caseUsage = this->state_.spikeStream_->getCaseUsage();

								this->state_.spikeNetworkPerformance_.addPerformanceMnist28x28(caseUsage, this->state_.spikeSet_, true);

								if (((sec % (1 * 60)) == 0) && (sec > 0))
								{
									std::cout << this->state_.spikeNetworkPerformance_.toStringConfusionMatrix() << std::endl;
									//std::cout << this->spikeNetworkPerformance_.toStringPerformance() << std::endl;
									std::cout << "avg precision " << this->state_.spikeNetworkPerformance_.getAveragePrecision() << std::endl;
									this->state_.spikeNetworkPerformance_.clear();
								}
							}
							if (dumpState) this->state_.dumperState_.dump(sec, "train");
							if (this->state_.dumperTopology_.dumpTest(sec)) this->state_.dumperTopology_.dump(sec, "train", this->getTopology());
						}
						{	// reporting non-essential progress
							const float wExc = this->getAverageOutgoingWeightExcitatory();
							const float wSensor = this->getAverageOutgoingWeightSensor();
							const float wMotor = this->getAverageIncommingWeightMotor();

							const clock_t t2 = clock();
							const double diff = static_cast<double>(t2 - t1);
							printf("spike::v3::mainloop: time %4u/%u s sim, %5.0f ms wall; nSpikes %5zu prop, %5zu rand; w_out_sensor %5.4f; w_out_exc %5.4f; w_in_motor %5.4f\n", sec, nSeconds, diff, this->state_.nSpikesPropagatedLastSec_, this->state_.nSpikesRandomLastSec_, wSensor, wExc, wMotor);
						}
					}
				}
			}

			void printKernels(const std::string& filename) const
			{
				// create the directory
				const std::string tree = ::tools::file::getDirectory(filename);
				if (!::tools::file::mkdirTree(tree))
				{
					std::cerr << "spike::v3::Network3::printKernels: Unable to create directory " << tree << std::endl;
					throw std::runtime_error("unable to create directory");
				}

				FILE * const fs = fopen(filename.c_str(), "w");
				if (fs == nullptr)
				{
					std::cerr << "spike::v3::Network3::printKernels: Error: could not write to file " << filename << std::endl;
					return;
				}

				const TimeInMs m = std::max(std::max(Options::kernelRangeEtaInMs, Options::kernelRangeThresholdInMs), std::max(Options::kernelRangeEpsilonInMs, Options::kernelRangeStdpInMs));

				fprintf(fs, "#kernels <timeInMs> <eta> <epsilon> <threshold> <ltp> <ltd>\n");
				for (KernelTime i = 0; i < Options::toKernelTime(m); ++i)
				{
					fprintf(fs, "%f ", Options::toTimeInMs(i));
					(i < Options::toKernelTime(Options::kernelRangeEtaInMs)) ? fprintf(fs, "%f ", this->state_.cachedEta_[i]) : fprintf(fs, "NaN ");
					(i < Options::toKernelTime(Options::kernelRangeEpsilonInMs)) ? fprintf(fs, "%f ", this->state_.cachedEpsilon_[i]) : fprintf(fs, "NaN ");
					(i < Options::toKernelTime(Options::kernelRangeThresholdInMs)) ? fprintf(fs, "%f ", this->state_.cachedThreshold_[i]) : fprintf(fs, "NaN ");
					(i < Options::toKernelTime(Options::kernelRangeStdpInMs)) ? fprintf(fs, "%f ", this->state_.cachedLtp_[i]) : fprintf(fs, "NaN ");
					(i < Options::toKernelTime(Options::kernelRangeStdpInMs)) ? fprintf(fs, "%f ", this->state_.cachedLtd_[i]) : fprintf(fs, "NaN ");
					fprintf(fs, "\n");
				}
				fclose(fs);
			}

		private:

			State<Topology, SpikeStream> state_;

			Voltage static calcVoltage(const State<Topology, SpikeStream>& state, const NeuronId neuronId, const KernelTime kerneltime)
			{
				KernelTime endRefractoryPeriod = std::get<0>(state.endRefractoryPeriods_.getSpikes(neuronId));
				KernelTime timeSinceLastRefreactoryPeriod = kerneltime - endRefractoryPeriod;

				//std::cout << "spike::v3::Network3::calcVoltage: neuronId=" << neuronId << "; kerneltime=" << kerneltime << "; endRefractoryPeriod=" << endRefractoryPeriod << "; timeSinceLastRefreactoryPeriod = " << timeSinceLastRefreactoryPeriod << std::endl;

				if (timeSinceLastRefreactoryPeriod <= 0)
				{
					return Options::minVoltage;
				}
				Voltage voltage = (timeSinceLastRefreactoryPeriod < Options::toKernelTime(Options::kernelRangeEtaInMs)) ? state.cachedEta_[timeSinceLastRefreactoryPeriod] : 0;

				std::tuple<const IncommingSpike * const, unsigned int, unsigned int> tuple = state.incommingSpikes_.getPastAndNearFutureSpikes(neuronId);
				const IncommingSpike * const incommingSpikes = std::get<0>(tuple);
				const size_t startPos = std::get<1>(tuple);
				const size_t endPos = std::get<2>(tuple);

				if (false && Options::tranceNeuronOn && (neuronId == Options::tranceNeuron))
				{
					printf("spike::v3::Network3::calcVoltage: A: neuronId=%u; kerneltime=%u; eta=%f; number of Epsilon=%zu\n", neuronId, kerneltime, voltage, endPos - startPos);
				}

				for (size_t i = startPos; i < endPos; ++i)
				{
					const IncommingSpike& incommingSpike = incommingSpikes[i];
					const KernelTime incommingTimeRelative = kerneltime - incommingSpike.kerneltime;

					if (incommingTimeRelative >= 0)
					{
						if (incommingTimeRelative < Options::toKernelTime(Options::kernelRangeEpsilonInMs))
						{
							const Voltage epsilon = state.cachedEpsilon_[incommingTimeRelative];
							const Voltage delta = incommingSpike.efficacy * epsilon;
							voltage += delta;

							if (false && Options::tranceNeuronOn && (neuronId == Options::tranceNeuron))
							{
								printf("spike::v3::Network3::calcVoltage: B: neuronId=%4u; kerneltime=%5u; incommingTime=%5u; origin=%4u; epsilon=%f; delta=%f; voltage=%f\n", neuronId, kerneltime, incommingSpike.kerneltime, incommingSpike.origin, epsilon, delta, voltage);
							}
						}
					}
					else
					{
						//std::cout << "calcVoltage: neuronId=" << neuronId << "; incommingSpike.kerneltime=" << incommingSpike.kerneltime << "; kerneltime=" << kerneltime << std::endl;
						//TODO: can this even happen?? Yes, sometimes it does...
						break;
					}
				}
				return voltage;
			}

			Voltage static calcThreshold(const State<Topology, SpikeStream>& state, const NeuronId neuronId, const KernelTime kerneltime)
			{
				Voltage threshold = Options::minimalThreshold;
				const std::tuple<KernelTime, KernelTime, KernelTime, KernelTime> tuple = state.endRefractoryPeriods_.getSpikes(neuronId);

				const KernelTime previousSpikeTimeRelative0 = kerneltime - std::get<0>(tuple);
				if ((previousSpikeTimeRelative0 >= 0) && (previousSpikeTimeRelative0 < Options::toKernelTime(Options::kernelRangeThresholdInMs)))
				{
					threshold *= state.cachedThreshold_[previousSpikeTimeRelative0];
				}
				const KernelTime previousSpikeTimeRelative1 = kerneltime - std::get<1>(tuple);
				if ((previousSpikeTimeRelative1 >= 0) && (previousSpikeTimeRelative1 < Options::toKernelTime(Options::kernelRangeThresholdInMs)))
				{
					threshold *= state.cachedThreshold_[previousSpikeTimeRelative1];
				}
				const KernelTime previousSpikeTimeRelative2 = kerneltime - std::get<2>(tuple);
				if ((previousSpikeTimeRelative2 >= 0) && (previousSpikeTimeRelative2 < Options::toKernelTime(Options::kernelRangeThresholdInMs)))
				{
					threshold *= state.cachedThreshold_[previousSpikeTimeRelative2];
				}
				const KernelTime previousSpikeTimeRelative3 = kerneltime - std::get<3>(tuple);
				if ((previousSpikeTimeRelative3 >= 0) && (previousSpikeTimeRelative3 < Options::toKernelTime(Options::kernelRangeThresholdInMs)))
				{
					threshold *= state.cachedThreshold_[previousSpikeTimeRelative3];
				}
				//if (threshold != 8) std::cout << "spike::v3::Network3::calcThreshold: threshold=" << threshold << std::endl;
				return threshold;
			}

			void advanceTime(const KernelTime futureTime)
			{
				const std::tuple<const IncommingSpike * const, size_t, size_t> tuple = this->state_.incommingSpikes_.advanceCurrentTime(futureTime, this->state_.endRefractoryPeriods_);
				const IncommingSpike * const nearFutureSpikes = std::get<0>(tuple);
				const size_t startPos = std::get<1>(tuple);
				const size_t endPos = std::get<2>(tuple);

				//ltd
				for (size_t i = startPos; i < endPos; ++i)
				{
					const IncommingSpike& incommingSpike = nearFutureSpikes[i];
					const NeuronId origin = incommingSpike.origin;

					if (!Topology::isInhNeuron(origin))
					{

						const NeuronId destination = incommingSpike.destination;
						const KernelTime incommingTime = incommingSpike.kerneltime;

						// get the time that this destination neuron has spiked
						const KernelTime timeDiff = incommingTime - this->state_.lastSpikeTime_[destination];

						if ((timeDiff >= 0) && (timeDiff < Options::toKernelTime(Options::kernelRangeStdpInMs)))
						{
							const float wD = this->state_.cachedLtd_[timeDiff];
							//std::cout << "spike::v3::Network3::advanceTime: LTD: neuron " << origin << " contributes at " << incommingTime << " to neuron " << destination << "; Neuron " << destination << " last spiked at " << spikeTime << "; weight decrease " << wD << std::endl;
							this->state_.synapses_.decWeight(origin, destination, wD);
							//if (dumpWeightDelta) this->dumperWeightDelta_.store_WeightDelta(t, origin, destination, -wD);
						}
					}
				}
			}

			void findAndFireNeuronA(const KernelTime currentTime, const KernelTime maxAdvanceTime, const bool dumpSpikes, const bool dumpState)
			{
				if (dumpSpikes)
				{
					if (dumpState)
					{
						this->findAndFireNeuronB<true, true>(currentTime, maxAdvanceTime);
					}
					else
					{
						this->findAndFireNeuronB<true, false>(currentTime, maxAdvanceTime);
					}
				}
				else
				{
					if (dumpState)
					{
						this->findAndFireNeuronB<false, true>(currentTime, maxAdvanceTime);
					}
					else
					{
						this->findAndFireNeuronB<false, false>(currentTime, maxAdvanceTime);
					}
				}
			}

			template <bool dumpSpikes, bool dumpState>
			void findAndFireNeuronB(const KernelTime currentTime, const KernelTime maxAdvanceTime)
			{
				if (Options::useOpenMP)
				{
					/*
					State<Topology, SpikeStream>& state = this->state_;
					const int nThreads = std::min(static_cast<int>(Options::maxNumberOfThreads), omp_get_num_procs());
					#pragma omp parallel for num_threads(nThreads) default(none) shared(state, currentTime, maxAdvanceTime)// schedule(dynamic,1)
					for (int neuronId = 0; neuronId < static_cast<int>(Options::nNeurons); ++neuronId) {
						Network3::testAndFireNeuron<dumpSpikes, dumpState>(state, static_cast<NeuronId>(neuronId), currentTime, maxAdvanceTime);
					}
					*/
				}
				else
				{
					for (const NeuronId neuronId : Topology::iterator_SensorNeurons())
					{
						Network3::testAndFire_SensorNeuron<dumpSpikes, dumpState>(this->state_, neuronId, currentTime, maxAdvanceTime);
					}
					for (const NeuronId neuronId : Topology::iterator_ExcInhNeurons())
					{
						Network3::testAndFire_ExcInhNeuron<dumpSpikes, dumpState>(this->state_, neuronId, currentTime, maxAdvanceTime);
					}
					for (const NeuronId neuronId : Topology::iterator_MotorNeurons())
					{
						Network3::testAndFire_MotorNeuron<dumpSpikes, dumpState>(this->state_, neuronId, currentTime, maxAdvanceTime);
					}
				}
			}

			template <bool dumpSpikes, bool dumpState>
			void static testAndFire_SensorNeuron(State<Topology, SpikeStream>& state, const NeuronId neuronId, const KernelTime currentTime, const KernelTime maxAdvanceTime)
			{
				const std::tuple<bool, KernelTime> tuple = state.spikeStream_->getNextSpikeTimeAndAdvance(neuronId, maxAdvanceTime);
				if (std::get<0>(tuple))
				{
					const KernelTime firingTime = std::get<1>(tuple);
					::tools::assert::assert_msg(firingTime <= maxAdvanceTime, "spike::v3::Network:testAndFire_SensorNeuron: firingTime ", firingTime, " is larger than maxAdvanceTime ", maxAdvanceTime);
					::tools::assert::assert_msg(firingTime >= currentTime, "spike::v3::Network:testAndFire_SensorNeuron: firingTime ", firingTime, " is smaller than current time ", currentTime);

					if (true && Options::tranceNeuronOn && (neuronId == Options::tranceNeuron))
					{
						printf("spike::v3::Network:testAndFire_SensorNeuron: SPIKE: neuron=%u; currentTime=%u; fireTime=%u\n", neuronId, currentTime, firingTime);
					}

					Network3::fire<dumpSpikes, dumpState>(state, currentTime, PostSynapticSpike(firingTime, neuronId, FiringReason::FIRE_CLAMPED));
				}
			}

			template <bool dumpSpikes, bool dumpState>
			void static testAndFire_ExcInhNeuron(State<Topology, SpikeStream>& state, const NeuronId neuronId, const KernelTime currentTime, const KernelTime maxAdvanceTime)
			{
				const KernelTime endRefractoryPeriod = std::get<0>(state.endRefractoryPeriods_.getSpikes(neuronId));
				if (endRefractoryPeriod < maxAdvanceTime)
				{
					const std::tuple<bool, KernelTime, Voltage, Voltage> firingTimeRange = Network3::approximateThresholdCrossingRange(state, neuronId, currentTime, maxAdvanceTime);
					if (std::get<0>(firingTimeRange))
					{
						if (true && Options::tranceNeuronOn && (neuronId == Options::tranceNeuron))
						{
							printf("spike::v3::Network3::testAndFire_ExcInhNeuron: SPIKE: neuron=%u; currentTime=%u; fireTime=%u; voltage=%f, threshold=%f\n", neuronId, currentTime, std::get<1>(firingTimeRange), std::get<2>(firingTimeRange), std::get<3>(firingTimeRange));
						}
						const KernelTime firingTime = std::get<1>(firingTimeRange);
						::tools::assert::assert_msg(firingTime >= currentTime, "firingTime ", firingTime, " is smaller than current time ", currentTime);
						::tools::assert::assert_msg(firingTime <= maxAdvanceTime, "firingTime ", firingTime, " is larger than maxAdvanceTime ", maxAdvanceTime);

						Network3::fire<dumpSpikes, dumpState>(state, currentTime, PostSynapticSpike(firingTime, neuronId, FiringReason::FIRE_PROPAGATED));
					}
					else
					{
						// see if the neuron fires randomly
						const PostSynapticSpike& randomSpike = state.nextRandomPostSynapticSpike_[neuronId];
						if (randomSpike.kerneltime < maxAdvanceTime)
						{
							Network3::fire<dumpSpikes, dumpState>(state, currentTime, randomSpike);
						}
					}
				}
			}

			template <bool dumpSpikes, bool dumpState>
			void static testAndFire_MotorNeuron(State<Topology, SpikeStream>& state, const NeuronId neuronId, const KernelTime currentTime, const KernelTime maxAdvanceTime)
			{
				const KernelTime endRefractoryPeriod = std::get<0>(state.endRefractoryPeriods_.getSpikes(neuronId));
				if (endRefractoryPeriod < maxAdvanceTime)
				{
					const std::tuple<bool, KernelTime, Voltage, Voltage> firingTimeRange = Network3::approximateThresholdCrossingRange(state, neuronId, currentTime, maxAdvanceTime);
					if (std::get<0>(firingTimeRange))
					{
						if (true && Options::tranceNeuronOn && (neuronId == Options::tranceNeuron))
						{
							printf("spike::v3::Network:testAndFire_MotorNeuron: SPIKE: neuron=%u; currentTime=%u; fireTime=%u; voltage=%f, threshold=%f\n", neuronId, currentTime, std::get<1>(firingTimeRange), std::get<2>(firingTimeRange), std::get<3>(firingTimeRange));
						}
						const KernelTime firingTime = std::get<1>(firingTimeRange);
						::tools::assert::assert_msg(firingTime >= currentTime, "firingTime ", firingTime, " is smaller than current time ", currentTime);
						::tools::assert::assert_msg(firingTime <= maxAdvanceTime, "firingTime ", firingTime, " is larger than maxAdvanceTime ", maxAdvanceTime);


						FiringReason firingReason;

						const CaseLabel currentCaseLabel = state.spikeStream_->getCurrentLabel();
						if (currentCaseLabel == NO_CASE_LABEL)
						{
							firingReason = FiringReason::FIRE_PROPAGATED;
						}
						else
						{
							const NeuronId correctNeuron = Topology::translateToMotorNeuronId(currentCaseLabel);
							firingReason = (correctNeuron == neuronId) ? FiringReason::FIRE_PROPAGATED_CORRECT : FiringReason::FIRE_PROPAGATED_INCORRECT;
						}

						//printf("spike::v3::Network:testAndFire_MotorNeuron: SPIKE: neuron=%u; firingReason=%u; currentTime=%u; fireTime=%u; voltage=%f, threshold=%f\n", neuronId, static_cast<int>(firingReason), currentTime, std::get<1>(firingTimeRange), std::get<2>(firingTimeRange), std::get<3>(firingTimeRange));

						Network3::fire<dumpSpikes, dumpState>(state, currentTime, PostSynapticSpike(firingTime, neuronId, firingReason));
					}
					else
					{
						// see if the neuron fires randomly
						const PostSynapticSpike& randomSpike = state.nextRandomPostSynapticSpike_[neuronId];
						if (randomSpike.kerneltime < maxAdvanceTime)
						{
							Network3::fire<dumpSpikes, dumpState>(state, currentTime, randomSpike);
						}
					}
				}
			}

			template <bool dumpSpikes, bool dumpState>
			void static fire(State<Topology, SpikeStream>& state, const KernelTime currentTime, const PostSynapticSpike nextPostSynapticSpike)
			{
				const NeuronId neuronId = nextPostSynapticSpike.neuronId;
				const KernelTime fireTime = nextPostSynapticSpike.kerneltime;

				::tools::assert::assert_msg(fireTime >= currentTime, "spike::v3::Network3::fire: neuron ", neuronId, " fires at ", fireTime, ", but this is before the currentTime ", currentTime, "; firing reason=", static_cast<int>(nextPostSynapticSpike.firingReason));

				if (false && Options::tranceNeuronOn && (neuronId == Options::tranceNeuron))
				{
					printf("spike::v3::Network3::fire: neuron %u fires at %5u\n", neuronId, fireTime);
				}

				if (nextPostSynapticSpike.firingReason == FiringReason::FIRE_PROPAGATED)
				{
					state.nSpikesPropagatedLastSec_++;
				}
				else
				{
					state.nSpikesRandomLastSec_++;
				}

				//1] update the last spike time of this neuron;
				state.lastSpikeTime_[neuronId] = fireTime;
				const KernelTime endRefractoryPeriod = fireTime + Options::toKernelTime(Options::refractoryPeriod);
				state.endRefractoryPeriods_.addSpike(neuronId, endRefractoryPeriod);

				//2] update the next random spike for this neurons such that this random spike is in the future after the refractory period.
				Network3::updateNextRandomPostSynapticSpike(state, neuronId, endRefractoryPeriod);

				//3] a spike remove the contributions to the state of all previous incomming spikes.
				state.incommingSpikes_.cleanup(neuronId);

				{	//4] for all outgoing pathways shedule a incomming spike somewhere in the future
					for (const NeuronId& destination : state.synapses_.getDestinations(neuronId))
					{
						const KernelTime delay = state.synapses_.getDelay(neuronId, destination);

						::tools::assert::assert_msg(delay >= Options::toKernelTime(static_cast<TimeInMs>(Options::minDelay)), "spike::v3::Network3::fire: delay is too small; delay=", delay);
						const KernelTime arrivalTime = fireTime + delay;

						if (false && Options::tranceNeuronOn && (neuronId == Options::tranceNeuron))
						{
							printf("spike::v3::Network3::fire: neuron %u fires at %5u: delivers spike at neuron %u at time %5u\n", neuronId, fireTime, destination, arrivalTime);
						}

						//for LTD: store at what time a spike is received at destination
						state.synapses_.setLastDeliverTime(neuronId, destination, arrivalTime);
						const float weight = state.synapses_.getWeight(neuronId, destination);
						state.incommingSpikes_.sheduleIncommingSpike(arrivalTime, neuronId, destination, weight);
					}
				}
				{	//5] for all contributing spike of the current spike: increase their weights.
					for (const NeuronId& contributingNeuronId : state.synapses_.getOrigins(neuronId))
					{
						if (!Topology::isInhNeuron(contributingNeuronId))
						{ // only update weights of excitatory neurons
							const KernelTime contributionTime = state.synapses_.getLastDeliverTime(contributingNeuronId, neuronId);
							const KernelTime timeDiff = fireTime - contributionTime;

							if ((timeDiff >= 0) && (timeDiff < Options::toKernelTime(Options::kernelRangeStdpInMs)))
							{
								const float wD = state.cachedLtp_[timeDiff];
								//std::cout << "spike::v3::Network3::fire: LTP: neuron " << neuronId << " fires at " << fireTime << "; neuron " << contributingNeuronId << " contributed at time " << contributionTime << "; timeDiff="<<timeDiff<<"; weight increase " << wD << std::endl;
								if (nextPostSynapticSpike.firingReason == FiringReason::FIRE_PROPAGATED_INCORRECT)
								{
									state.synapses_.decWeight(contributingNeuronId, neuronId, wD);
								}
								else if (nextPostSynapticSpike.firingReason == FiringReason::FIRE_PROPAGATED_CORRECT)
								{
									state.synapses_.incWeight(contributingNeuronId, neuronId, 10 * wD);
								}
								else
								{
									state.synapses_.incWeight(contributingNeuronId, neuronId, wD);
									//if (dumpWeightDelta) this->dumperWeightDelta_.store_WeightDelta(fireTime, contributingNeuronId, neuronId, wD);
								}
							}
						}
					}
				}
				{	// dump spikes and state
					if (dumpSpikes)
					{
						const TimeInMs t = Options::toTimeInMs(fireTime);
						const TimeInSec sec = static_cast<TimeInSec>(t / 1000);
						const TimeInMs timeInMs = t - (1000 * sec);
						state.spikeSet_.addFiring(timeInMs, neuronId, nextPostSynapticSpike.firingReason);
					}
					if (dumpState)
					{
						const TimeInMs t = Options::toTimeInMs(fireTime);
						const TimeInSec sec = static_cast<TimeInSec>(t / 1000);
						const TimeInMs timeInMs = t - (1000 * sec);
						if ((state.lastTimeStateDumped_ + 0.01) < t)
						{
							for (const NeuronId neuronId2 : Topology::iterator_AllNeurons())
							{
								Voltage v = Network3::calcVoltage(state, neuronId2, fireTime);
								const Voltage threshold = Network3::calcThreshold(state, neuronId2, fireTime);
								if (v <= state.options_.minVoltage) v = nanf("");
								state.dumperState_.store(neuronId2, timeInMs, v, threshold);
							}
							state.lastTimeStateDumped_ = t;
						}
					}
				}
			}

			std::tuple<bool, KernelTime, Voltage, Voltage> static approximateThresholdCrossingRange(
				const State<Topology, SpikeStream>& state,
				const NeuronId neuronId,
				const KernelTime startTime,
				const KernelTime endTime)
			{
				::tools::assert::assert_msg(startTime <= endTime, "spike::v3::Network: approximateThresholdCrossingRange");
				KernelTime t0 = startTime;
				KernelTime t2 = endTime;

				const Voltage v2 = Network3::calcVoltage(state, neuronId, t2);
				const Voltage threshold2 = Network3::calcThreshold(state, neuronId, t2);
				if (v2 <= threshold2)
				{

					if (true && Options::tranceNeuronOn && (neuronId == Options::tranceNeuron))
					{
						const Voltage v0 = Network3::calcVoltage(state, neuronId, t0);
						const Voltage threshold0 = Network3::calcThreshold(state, neuronId, t0);
						printf("spike::v3::Network3::approximateThresholdCrossingRange: A: neuron=%u; t0=%u; v0=%f; threshold0=%f; t2=%u; v2=%f; threshold2=%f; \n", neuronId, t0, v0, threshold0, t2, v2, threshold2);
					}
					return std::make_tuple(false, 0, v2, threshold2);
				}

				const Voltage v0 = Network3::calcVoltage(state, neuronId, t0);
				const Voltage threshold0 = Network3::calcThreshold(state, neuronId, t0);
				const Voltage diff = v0 - threshold0;
				if (diff > 0.01)
				{
					// this happens when another neuron that has an older incomming spike spike after the start starttime of this neuronId 
					printf("spike::v3::Network3::approximateThresholdCrossingRange:: ERROR: neuron=%4u; t0=%4u; v0=%f; higher than threshold=%f; amount over %f\n", neuronId, t0, v0, threshold0, diff);
					return std::make_tuple(true, startTime, v0, threshold0);
				}

				if (true && Options::tranceNeuronOn && (neuronId == Options::tranceNeuron))
				{
					printf("spike::v3::Network3::approximateThresholdCrossingRange: B: neuron=%u; t0=%u; v0=%f; threshold0=%f; t2=%u; v2=%f; threshold2=%f\n", neuronId, t0, v0, threshold0, t2, v2, threshold2);

					for (int tn = startTime; tn < endTime; ++tn)
					{
						Voltage vn = Network3::calcVoltage(state, neuronId, tn);
						Voltage thresholdn = Network3::calcThreshold(state, neuronId, tn);
						printf("spike::v3::Network3::approximateThresholdCrossingRange: B2: neuron=%u; tn=%u; vn=%f; thresholdn=%f\n", neuronId, tn, vn, thresholdn);
					}
				}

				Voltage v1;
				Voltage threshold1;

				size_t i;
				for (i = 200; i > 0; --i)
				{
					const KernelTime t1 = t0 + ((t2 - t0) / 2);

					v1 = Network3::calcVoltage(state, neuronId, t1);
					threshold1 = Network3::calcThreshold(state, neuronId, t1);

					if (Options::tranceNeuronOn && (neuronId == Options::tranceNeuron))
					{
						//printf("spike::v3::Network3::approximateThresholdCrossingRange: C: neuron=%u; t1=%u; v1=%f; threshold1=%f\n", neuronId, t1, v1, threshold1);
					}

					if (v1 > threshold1)
					{
						t0 = t1;
					}
					else
					{
						t2 = t1;
					}
					const KernelTime timeDiff = t2 - t0;
					if (timeDiff <= 1)
					{
						//std::cout << "findAproximeThresholdCrossing: new timeDifference is sufficient small." << std::endl;
						break;
					}
				}

				//std::cout << "spike::v3::Network3::findAproximeThresholdCrossing i=" << i << std::endl;
				//std::cout << "spike::v3::Network3::findAproximeThresholdCrossing: neuronId=" << neuronId << "; new timeBefore=" << t0 << "; timeAfter=" << t2 << "; voltage=" << v1 << "; threshold=" << threshold << std::endl;

				//				__debugbreak();
				return std::make_tuple(true, t2, v1, threshold1);
			}

			float getAverageOutgoingWeight(const NeuronId neuronId) const
			{
				double sum = 0;
				size_t counter = 0;
				for (const NeuronId& destination : this->state_.synapses_.getDestinations(neuronId))
				{
					sum += this->state_.synapses_.getWeight(neuronId, destination);
					counter++;
				}
				return static_cast<float>(((sum == 0) || (counter == 0)) ? 0 : (sum / counter));
			}

			float getAverageIncommingWeight(const NeuronId neuronId) const
			{
				double sum = 0;
				size_t counter = 0;
				for (const NeuronId& origin : this->state_.synapses_.getOrigins(neuronId))
				{
					sum += this->state_.synapses_.getWeight(origin, neuronId);
					counter++;
				}
				return static_cast<float>(((sum == 0) || (counter == 0)) ? 0 : (sum / counter));
			}

			float getAverageOutgoingWeightExcitatory() const
			{
				double sum = 0;
				size_t counter = 0;

				for (const NeuronId neuronId : Topology::iterator_ExcNeurons())
				{
					const float averageWeight = this->getAverageOutgoingWeight(neuronId);
					//std::cout << "Spike_Network_State::getAverageOutgoingWeightExcitatory: neuronId=" << neuronId << "; average weight " << averageWeight << std::endl;
					sum += averageWeight;
					counter++;
				}
				return static_cast<float>(((counter == 0) || (sum == 0)) ? 0 : (sum / counter));
			}

			float getAverageOutgoingWeightSensor() const
			{
				double sum = 0;
				size_t counter = 0;

				for (const NeuronId neuronId : Topology::iterator_SensorNeurons())
				{
					const float averageWeight = this->getAverageOutgoingWeight(neuronId);
					sum += averageWeight;
					counter++;
				}
				return static_cast<float>(((counter == 0) || (sum == 0)) ? 0 : (sum / counter));
			}

			float getAverageIncommingWeightMotor() const
			{
				double sum = 0;
				size_t counter = 0;

				for (const NeuronId neuronId : Topology::iterator_MotorNeurons())
				{
					const float averageWeight = this->getAverageIncommingWeight(neuronId);
					sum += averageWeight;
					counter++;
				}
				return static_cast<float>(((counter == 0) || (sum == 0)) ? 0 : (sum / counter));
			}

			void substractTime(const KernelTime time)
			{
				std::cout << "spike::v3::Network3::substractTime: time=" << time << std::endl;

				this->state_.lastTimeStateDumped_ -= time;
				for (const NeuronId neuronId : Topology::iterator_AllNeurons())
				{
					this->state_.lastSpikeTime_[neuronId] -= time;
					this->state_.nextRandomPostSynapticSpike_[neuronId].kerneltime -= time;
				}
				this->state_.incommingSpikes_.substractTime(time);
				this->state_.endRefractoryPeriods_.substractTime(time);
				this->state_.spikeStream_->substractTime(time);
			}

			void updatePathways(const std::shared_ptr<Topology>& topology) const
			{
				topology->clearPathways();
				for (const NeuronId origin : Topology::iterator_AllNeurons())
				{
					for (const NeuronId& destination : this->state_.synapses_.getDestinations(origin))
					{
						const Efficacy efficacy = this->state_.synapses_.getWeight(origin, destination);
						const KernelTime delay = this->state_.synapses_.getDelay(origin, destination);
						topology->addPathway(origin, destination, static_cast<Delay>(Options::toTimeInMs(delay)), efficacy);
					}
				}
			}

			void static updateNextRandomPostSynapticSpike(
				State<Topology, SpikeStream>& state,
				const NeuronId neuronId,
				const KernelTime currentTime)
			{
				state.nextRandomPostSynapticSpike_[neuronId].neuronId = neuronId;
				state.nextRandomPostSynapticSpike_[neuronId].kerneltime = Network3::getNextRandomSpikeTime(neuronId, currentTime);
				state.nextRandomPostSynapticSpike_[neuronId].firingReason = FiringReason::FIRE_RANDOM;
				//std::cout << "updateNextRandomPostSynapticSpike: " << this->nextRandomPostSynapticSpike_.toString() << std::endl;
			}

			KernelTime static getNextRandomSpikeTime(const NeuronId /*neuronId*/, const KernelTime lastSpikeTime)
			{
				const float targetHz = Options::randomSpikeHz;
				const double averageTimeBetweenSpikes = 2000.0 / targetHz;
				const double r = static_cast<double>(rand() + 1) / RAND_MAX;
				const TimeInMs timeDelta = static_cast<TimeInMs>(averageTimeBetweenSpikes * r);


				KernelTime nextRandomSpikeTime;
				if (std::isfinite(timeDelta))
				{
					nextRandomSpikeTime = lastSpikeTime + Options::toKernelTime(timeDelta);
				}
				else
				{
					nextRandomSpikeTime = lastSpikeTime + 1;
				}

				//std::cout << "spike::v3::Network3::getNextRandomSpikeTime: neuronId=" << neuronId << "; nextRandomSpikeTime = " << nextRandomSpikeTime << "; targetHz=" << targetHz << "; averageTimeBetweenSpikes=" << averageTimeBetweenSpikes << "; r=" << r << "; timeDelta=" << timeDelta << std::endl;
				return nextRandomSpikeTime;
			}
		};
	}
}