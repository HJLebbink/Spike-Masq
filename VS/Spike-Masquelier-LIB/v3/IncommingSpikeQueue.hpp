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

#include "SpikeOptionsStatic.hpp"
#include "Types.hpp"
#include "SpikeHistory.hpp"

namespace spike 
{
	namespace v3 
	{
		/*
		template <KernelTime maxAdvanceTime, size_t nFutureWindows>
		class IncommingSpikeQueue
		{
		//nearFutureWindow is the time between the currentTime and the currentTime plus the minDelay
		public:

		// constructor
		IncommingSpikeQueue()
		: spikesPastDestination_(std::vector<IncommingSpike>(Options::nNeurons * maxNumberOfSpikes))
		, spikesTmp_(std::vector<IncommingSpike>(Options::nNeurons * maxNumberOfSpikes))

		, futureSpikes_(std::vector<IncommingSpike>(Options::nNeurons * maxNumberOfSpikes))
		, nearFutureStart_(0)
		, farFutureStart_(0)
		, farFutureEnd_(0)
		{
		for (NeuronId neuronId = 0; neuronId < Options::nNeurons; ++neuronId) {
		this->cleanup(neuronId);
		}
		}

		void sheduleIncommingSpike(const KernelTime kerneltime, const NeuronId origin, const NeuronId destination, const Efficacy efficacy)
		{
		BOOST_ASSERT_MSG_HJ(kerneltime != NO_KERNEL_TIME, "not allowed to shedule NO_KERNEL_TIME");

		this->futureSpikes_[this->farFutureEnd_] = IncommingSpike(kerneltime, origin, destination, efficacy);
		this->farFutureEnd_++;

		if (this->farFutureEnd_ >= (Options::nNeurons * maxNumberOfSpikes)) {
		if (this->nearFutureStart_ < 1000) {
		std::cout << "spike::v3::IncommingSpikeQueue::sheduleIncommingSpike: too many spikes" << std::endl;
		this->nearFutureStart_ = 0;
		this->farFutureStart_ = 0;
		this->farFutureEnd_ = 0;
		} else {
		const size_t displacement = this->nearFutureStart_;
		this->nearFutureStart_ = 0;
		this->farFutureStart_ -= displacement;
		this->farFutureEnd_ -= displacement;

		for (size_t i = 0; i < this->farFutureEnd_; ++i) {
		this->futureSpikes_[i] = this->futureSpikes_[i + displacement];
		}
		}
		}
		}

		const std::tuple<const IncommingSpike * const, unsigned int, unsigned int> getPastSpikes(const NeuronId neuronId) const
		{
		// the past spike also contain future spike that cannot be reorded, accept for by random future spikes
		const std::tuple<unsigned int, unsigned int>& tuple = this->pastAndNearFutureSpikesStartEndPos_[neuronId];
		return std::make_tuple(this->spikesPastDestination_.data(), std::get<0>(tuple), std::get<1>(tuple));
		}

		//remove all past spikes for the provided neuronId
		void cleanup(const NeuronId neuronId)
		{
		const unsigned int beginPos = this->absoluteBeginPos(neuronId);
		this->pastAndNearFutureSpikesStartEndPos_[neuronId] = std::make_tuple(beginPos, beginPos);
		}

		// return the near spikes that have been added by advancing the time
		const std::tuple<const IncommingSpike * const, size_t, size_t> advanceCurrentTime(
		const SpikeHistory4<Topology>& endRefractoryPeriods)
		{
		this->currentFutureWindow_++;
		this->currentFutureWindowModulo_++;

		if (this->currentFutureWindowModulo_ >= nFutureWindows) {
		this->currentFutureWindowModulo_ = 0;
		}

		this->cleanupPast(this->getCurrentTime());// advance time for the past spikes

		// advance time for the nearFuture and farFuture spikes

		this->nearFutureStart_ = this->farFutureStart_;

		size_t newFarFutureStart = this->farFutureStart_;
		for (size_t i = this->farFutureStart_; i < this->farFutureEnd_; ++i) {
		const IncommingSpike& spike = this->futureSpikes_[i];
		if (spike.kerneltime <= kerneltime) {
		newFarFutureStart++;

		if (spike.kerneltime >= std::get<0>(endRefractoryPeriods.getSpikes(spike.destination))) { // do not add incomming spikes that occur during the refactory period
		this->addToPast(spike.destination, spike);
		} else {
		//std::cout << "spike::v3::IncommingSpikeQueue::advanceCurrentTime: IncommingSpike=" << next.toString() << "; endOfRefractoryPeriod=" << endOfRefractoryPeriod << std::endl;
		}
		} else {
		break;
		}
		}
		this->farFutureStart_ = newFarFutureStart;

		advanceCurrentTime is fundamentally broken! When scheduling lists of IncommingSpikes should be kept of mutliples of maxAdvanceTime

		const bool doCorrectnessAnalysis = true;
		if (doCorrectnessAnalysis) {
		for (size_t i = this->nearFutureStart_; i < this->farFutureStart_; ++i) {
		printf("spike::v3::Network::advanceCurrentTime: nearFuture: t=%4u; origin=%4u; destination=%4u; efficacy=%f\n", this->futureSpikes_[i].kerneltime, this->futureSpikes_[i].origin, this->futureSpikes_[i].destination, this->futureSpikes_[i].efficacy);
		}
		for (size_t i = this->farFutureStart_; i < this->farFutureEnd_; ++i) {
		printf("spike::v3::Network::advanceCurrentTime: farFuture: t=%4u; origin=%4u; destination=%4u; efficacy=%f\n", this->futureSpikes_[i].kerneltime, this->futureSpikes_[i].origin, this->futureSpikes_[i].destination, this->futureSpikes_[i].efficacy);
		}
		}
		return std::make_tuple(this->futureSpikes_.data(), this->nearFutureStart_, this->farFutureStart_);
		}

		void addToPast(const NeuronId neuronId, const IncommingSpike spike)
		{
		std::tuple<unsigned int, unsigned int>& tuple = this->pastAndNearFutureSpikesStartEndPos_[neuronId];
		unsigned int endPos = std::get<1>(tuple);
		this->spikesPastDestination_[endPos] = spike;
		endPos++;
		std::get<1>(tuple) = endPos;

		if (endPos >= this->absoluteEndPos(neuronId)) {
		const unsigned int beginPos = std::get<0>(tuple);
		const unsigned int newBeginPos = this->absoluteBeginPos(neuronId);

		const unsigned int length = endPos - beginPos;
		if (length < maxNumberOfSpikes) {
		for (unsigned int i = 0; i < length; ++i) {
		this->spikesPastDestination_[newBeginPos + i] = this->spikesPastDestination_[beginPos + i];
		}
		this->pastAndNearFutureSpikesStartEndPos_[neuronId] = std::make_tuple(newBeginPos, newBeginPos + length);
		} else {
		std::cout << "spike::v3::IncommingSpikeQueue::addToPast: too many spikes" << std::endl;
		this->pastAndNearFutureSpikesStartEndPos_[neuronId] = std::make_tuple(newBeginPos, newBeginPos);
		}
		}
		}

		private:

		static const size_t maxNumberOfSpikes = 10000;


		size_t currentFutureWindow_ = 0;
		size_t currentFutureWindowModulo_ = 0;

		std::vector<IncommingSpike> spikesPastDestination_;
		std::vector<IncommingSpike> spikesTmp_;

		std::array<std::tuple<unsigned int, unsigned int>, Options::nNeurons> pastAndNearFutureSpikesStartEndPos_;

		//std::priority_queue<IncommingSpike, std::vector<IncommingSpike>, CompareIncommingSpike> spikesQueue_;
		std::vector<IncommingSpike> futureSpikes_;
		size_t nearFutureStart_;
		size_t farFutureStart_;
		size_t farFutureEnd_;

		size_t index(const NeuronId neuronId, const size_t pos)
		{
		return (neuronId * Options::nNeurons) + pos;
		}

		unsigned int absoluteBeginPos(const NeuronId neuronId) const
		{
		return (neuronId * maxNumberOfSpikes);
		}
		unsigned int absoluteEndPos(const NeuronId neuronId) const
		{
		return (neuronId * maxNumberOfSpikes) + Options::nNeurons;
		}

		void cleanupPast(const KernelTime kerneltime)
		{

		const KernelTime timeHorizon = kerneltime - (Options::kernelRangeEpsilonInMs * Options::nSubMs);
		for (NeuronId neuronId = 0; neuronId < Options::nNeurons; ++neuronId) {
		std::tuple<unsigned int, unsigned int>& tuple = this->pastAndNearFutureSpikesStartEndPos_[neuronId];
		size_t startPos = std::get<0>(tuple);
		const size_t endPos = std::get<1>(tuple);

		while (startPos < endPos) {
		if (this->spikesPastDestination_[startPos].kerneltime <= timeHorizon) {
		startPos++;
		} else {
		break;
		}
		}
		std::get<0>(tuple) = static_cast<unsigned int>(startPos);

		//000000013FBA2578  cmp         r8,r10
		//000000013FBA257B  jae         labelEND
		//000000013FBA257D  mov         eax,r8d
		//000000013FBA2580  shl         rax,4
		//000000013FBA2584  add         rax,qword ptr [rbx]
		//labelLOOP:
		//000000013FBA2587  cmp         dword ptr [rax],r9d
		//000000013FBA258A  jg          labelEND
		//000000013FBA258C  inc         r8
		//000000013FBA258F  add         rax,10h
		//000000013FBA2593  cmp         r8,r10
		//000000013FBA2596  jb          labelLOOP
		//labelEND:
		//000000013FBA2598  mov         dword ptr [r11+4],r8d
		}
		}

		size_t determineFutureWindow()
		{
		return 0;
		}

		size_t getCurrentTime() const
		{
		return his->currentFutureWindow_ * maxAdvanceTime;
		}
		};
		*/

		template <typename Topology_i>
		class IncommingSpikeQueue {
		public:

			using Topology = Topology_i;
			using Options = typename Topology_i::Options;

			// constructor
			IncommingSpikeQueue()
				: pastAndNearFutureSpikes_(std::vector<IncommingSpike>(Options::nNeurons * maxNumberOfSpikes))
				, spikesTmp_(std::vector<IncommingSpike>(Options::nNeurons * maxNumberOfSpikes))

				, farFutureSpikes_(std::vector<IncommingSpike>(Options::nNeurons * maxNumberOfSpikes))
				, farFutureSpikesLength_(0)

				, nearFutureSpikes_(std::vector<IncommingSpike>(Options::nNeurons * maxNumberOfSpikes))
				, nearFutureSpikesLength_(0)
				, currentTime_(0)
			{
				for (NeuronId neuronId = 0; neuronId < Options::nNeurons; ++neuronId) {
					this->cleanup(neuronId);
				}
			}

			void sheduleIncommingSpike(const KernelTime kerneltime, const NeuronId origin, const NeuronId destination, const Efficacy efficacy)
			{
				::tools::assert::assert_msg(kerneltime != NO_KERNEL_TIME, "not allowed to shedule NO_KERNEL_TIME");
				this->farFutureSpikes_[this->farFutureSpikesLength_] = IncommingSpike(kerneltime, origin, destination, efficacy);
				this->farFutureSpikesLength_++;
				if (this->farFutureSpikesLength_ >= (Options::nNeurons * maxNumberOfSpikes)) {
					std::cout << "spike::v3::IncommingSpikeQueue::sheduleIncommingSpike: too many spikes" << std::endl;
					__debugbreak();
					this->farFutureSpikesLength_ = 0;
				}
			}

			const std::tuple<const IncommingSpike * const, unsigned int, unsigned int> getPastAndNearFutureSpikes(const NeuronId neuronId) const
			{
				// the past spike also contain future spike that cannot be reorded, accept for by random future spikes
				const std::tuple<unsigned int, unsigned int>& tuple = this->pastAndNearFutureSpikesStartEndPos_[neuronId];
				return std::make_tuple(this->pastAndNearFutureSpikes_.data(), std::get<0>(tuple), std::get<1>(tuple));
			}

			void cleanup(const NeuronId neuronId)
			{
				const unsigned int beginPos = this->absoluteBeginPos(neuronId);
				this->pastAndNearFutureSpikesStartEndPos_[neuronId] = std::make_tuple(beginPos, beginPos);
			}

			// return the near spikes that have been added by advancing the time
			const std::tuple<const IncommingSpike * const, size_t, size_t> advanceCurrentTime(
				const KernelTime futureTime,
				const SpikeHistory4<Topology>& endRefractoryPeriods)
			{
				if (Options::tranceNeuronOn) {
					printf("spike::v3::IncommingSpikeQueueSlow::advanceCurrentTime: currentTime %5u; advancing time to %5u\n", this->currentTime_, futureTime);
				}
				this->cleanupPast(futureTime);// advance time for the past spikes

				{ // advance time for the nearFuture and farFuture spikes
					size_t newNearFutureSpikesLength = 0;
					size_t newFarFutureSpikesLength = 0;

					for (size_t i = 0; i < this->farFutureSpikesLength_; ++i) {
						const IncommingSpike& spike = this->farFutureSpikes_[i];
						if (spike.kerneltime < futureTime) {
							::tools::assert::assert_msg(spike.kerneltime >= this->currentTime_, "spike::v3::IncommingSpikeQueueSlow::advanceCurrentTime: adding a spike to the past that should have been added to the past much earlier. \ncurrentTime=", this->currentTime_, "; futureTime=", futureTime, "; spike.time=", spike.kerneltime, "; spike.origin=", spike.origin, "; spike.destination=", spike.destination);
							this->nearFutureSpikes_[newNearFutureSpikesLength] = spike;
							newNearFutureSpikesLength++;

							if (spike.kerneltime >= std::get<0>(endRefractoryPeriods.getSpikes(spike.destination))) { // do not add incomming spikes that occur during the refactory period
								this->addToPastAndNearFuture(spike.destination, spike);
							} else {
								//std::cout << "spike::v3::IncommingSpikeQueue::advanceCurrentTime: IncommingSpike=" << next.toString() << "; endOfRefractoryPeriod=" << endOfRefractoryPeriod << std::endl;
							}
						} else {
							this->spikesTmp_[newFarFutureSpikesLength] = spike;
							newFarFutureSpikesLength++;
						}
					}
					for (size_t i = 0; i < newFarFutureSpikesLength; ++i) {
						this->farFutureSpikes_[i] = this->spikesTmp_[i];
					}

					this->nearFutureSpikesLength_ = newNearFutureSpikesLength;
					this->farFutureSpikesLength_ = newFarFutureSpikesLength;
				}


				this->currentTime_ = futureTime;
				return std::make_tuple(this->nearFutureSpikes_.data(), 0, this->nearFutureSpikesLength_);
			}

			void addToPastAndNearFuture(const NeuronId neuronId, const IncommingSpike spike)
			{
				if (Options::tranceNeuronOn && (neuronId == Options::tranceNeuron)) {
					printf("spike::v3::IncommingSpikeQueueSlow::addToPastAndNearFuture: neuronId=%u; spike time %u\n", neuronId, spike.kerneltime);
				}
				std::tuple<unsigned int, unsigned int>& tuple = this->pastAndNearFutureSpikesStartEndPos_[neuronId];
				unsigned int& endPos = std::get<1>(tuple);

				if (endPos == this->absoluteEndPos(neuronId)) { // cleanup pastAndNearFutureSpikes_
					const unsigned int beginPos = std::get<0>(tuple);
					const unsigned int newBeginPos = this->absoluteBeginPos(neuronId);

					const unsigned int length = endPos - beginPos;
					if (length < maxNumberOfSpikes) {
						for (unsigned int i = 0; i < length; ++i) {
							this->pastAndNearFutureSpikes_[newBeginPos + i] = this->pastAndNearFutureSpikes_[beginPos + i];
						}
						this->pastAndNearFutureSpikesStartEndPos_[neuronId] = std::make_tuple(newBeginPos, newBeginPos + length);
					} else {
						std::cout << "spike::v3::IncommingSpikeQueue::addToPast: too many spikes" << std::endl;
						this->pastAndNearFutureSpikesStartEndPos_[neuronId] = std::make_tuple(newBeginPos, newBeginPos);
					}
					tuple = this->pastAndNearFutureSpikesStartEndPos_[neuronId];
				}
				this->pastAndNearFutureSpikes_[endPos] = spike;
				endPos++;
			}

			void substractTime(const KernelTime time)
			{
				for (const NeuronId neuronId : Topology::iterator_AllNeurons()) {

					const auto tuple = this->pastAndNearFutureSpikesStartEndPos_[neuronId];
					for (size_t i = std::get<0>(tuple); i < std::get<1>(tuple); ++i) {
						this->pastAndNearFutureSpikes_[i].kerneltime -= time;
					}
				}

				for (size_t i = 0; i < this->farFutureSpikesLength_; ++i) {
					this->farFutureSpikes_[i].kerneltime -= time;
				}
				for (size_t i = 0; i < this->nearFutureSpikesLength_; ++i) {
					this->nearFutureSpikes_[i].kerneltime -= time;
				}

				this->currentTime_ -= time;
			}

		private:

			static const size_t maxNumberOfSpikes = 10000;

			std::vector<IncommingSpike> pastAndNearFutureSpikes_;
			std::array<std::tuple<unsigned int, unsigned int>, Options::nNeurons> pastAndNearFutureSpikesStartEndPos_;

			// solely for temporary purposes
			std::vector<IncommingSpike> spikesTmp_;

			//std::priority_queue<IncommingSpike, std::vector<IncommingSpike>, CompareIncommingSpike> spikesQueue_;
			std::vector<IncommingSpike> farFutureSpikes_;
			size_t farFutureSpikesLength_;

			std::vector<IncommingSpike> nearFutureSpikes_;
			size_t nearFutureSpikesLength_;

			KernelTime currentTime_; //currentTime_ is only used for debug purposes

			size_t index(const NeuronId neuronId, const size_t pos)
			{
				return (neuronId * Options::nNeurons) + pos;
			}

			unsigned int absoluteBeginPos(const NeuronId neuronId) const
			{
				return (neuronId * maxNumberOfSpikes);
			}
			unsigned int absoluteEndPos(const NeuronId neuronId) const
			{
				return (neuronId * maxNumberOfSpikes) + Options::nNeurons;
			}

			void cleanupPast(const KernelTime kerneltime)
			{
				const KernelTime timeHorizon = kerneltime - Options::toKernelTime(Options::kernelRangeEpsilonInMs);
				for (const NeuronId neuronId : Topology::iterator_AllNeurons()) {
					std::tuple<unsigned int, unsigned int>& tuple = this->pastAndNearFutureSpikesStartEndPos_[neuronId];
					size_t startPos = std::get<0>(tuple);
					const size_t endPos = std::get<1>(tuple);

					while (startPos < endPos) {
						if (this->pastAndNearFutureSpikes_[startPos].kerneltime <= timeHorizon) {
							startPos++;
						} else {
							break;
						}
					}
					//if (startPos != std::get<0>(this->pastAndNearFutureSpikesStartEndPos_Exc_[neuronId])) {
					//	std::cout << "spike::v3::IncommingSpikeQueue: cleanupPast: new startPos=" << startPos << "; old startPos=" << std::get<0>(this->pastAndNearFutureSpikesStartEndPos_Exc_[neuronId]) << std::endl;
					//}
					std::get<0>(tuple) = static_cast<unsigned int>(startPos);

					/*
					000000013FBA2578  cmp         r8,r10
					000000013FBA257B  jae         labelEND
					000000013FBA257D  mov         eax,r8d
					000000013FBA2580  shl         rax,4
					000000013FBA2584  add         rax,qword ptr [rbx]
					labelLOOP:
					000000013FBA2587  cmp         dword ptr [rax],r9d
					000000013FBA258A  jg          labelEND
					000000013FBA258C  inc         r8
					000000013FBA258F  add         rax,10h
					000000013FBA2593  cmp         r8,r10
					000000013FBA2596  jb          labelLOOP
					labelEND:
					000000013FBA2598  mov         dword ptr [r11+4],r8d
					*/
				}
			}
		};
	}
}