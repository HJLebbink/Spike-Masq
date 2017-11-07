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
#include <limits>		// std::numeric_limits

#include "Types.hpp"
#include "SpikeOptionsStatic.hpp"

namespace spike
{
	namespace v3
	{
		template <typename Topology_i>
		class Synapses
		{
		public:

			using Topology = Topology_i;
			using Options = typename Topology_i::Options;

			static const size_t nNeurons = Options::nNeurons;

			// constructor
			Synapses()
				: w_(std::vector<float>(nNeurons*nNeurons, std::numeric_limits<float>::quiet_NaN()))
				, delay_(std::vector<KernelTime>(nNeurons*nNeurons, -1))
				, wd_plus_(std::vector<unsigned int>(nNeurons*nNeurons, 0))
				, wd_min_(std::vector<unsigned int>(nNeurons*nNeurons, 0))
				, lastDeliverTime_(std::vector<KernelTime>(nNeurons*nNeurons, -1000 * Options::nSubMs))

				, outgoingNeurons_(nNeurons, std::vector<NeuronId>())
				, incommingNeurons_(nNeurons, std::vector<NeuronId>())
			{
				//	for (size_t i = 0; i < this->incommingSpikes_.size(); ++i) {
				//		this->incommingSpikes_[i] = std::make_shared<std::vector<std::tuple<NeuronId, NeuronId>>>(40000);
				//	}
				//	this->nIncommingSpikes_.fill(0);
			}

			void init(const std::shared_ptr<Topology>& topology)
			{
				// copy the pathways from topology to this network
				for (const NeuronId neuronId : Topology::iterator_AllNeurons())
				{

					std::vector<NeuronId>& outgoingNeurons = this->outgoingNeurons_[neuronId];
					outgoingNeurons.clear();
					for (const Pathway& p : topology->getOutgoingPathways(neuronId))
					{
						::tools::assert::assert_msg(p.origin == neuronId, "incorrect origin");
						outgoingNeurons.push_back(p.destination);
						this->setWeight(neuronId, p.destination, p.efficacy);
						this->setDelay(neuronId, p.destination, Options::toKernelTime(static_cast<TimeInMs>(p.delay)));
					}

					std::vector<NeuronId>& incommingNeurons = this->incommingNeurons_[neuronId];
					incommingNeurons.clear();
					for (const Pathway& p : topology->getIncommingPathways(neuronId))
					{
						::tools::assert::assert_msg(p.destination == neuronId, "incorrect destination");
						incommingNeurons.push_back(p.origin);
					}
				}
			}

			KernelTime getLastDeliverTime(const NeuronId origin, const NeuronId destination) const
			{
				return this->lastDeliverTime_[this->index(origin, destination)];
			}

			void setLastDeliverTime(const NeuronId origin, const NeuronId destination, const KernelTime t)
			{
				this->check(origin, destination);
				const size_t i = this->index(origin, destination);
				this->lastDeliverTime_[i] = t;
			}

			const std::vector<NeuronId>& getDestinations(const NeuronId origin) const
			{
				return this->outgoingNeurons_[origin];
			}

			const std::vector<NeuronId>& getOrigins(const NeuronId destination) const
			{
				return this->incommingNeurons_[destination];
			}

			void setWeight(const NeuronId origin, const NeuronId destination, const float value)
			{
				this->w_[this->index(origin, destination)] = value;
			}

			void incWeight(const NeuronId origin, const NeuronId destination, const float value)
			{
				//std::cout << "incWeight: origin = " << origin << "; destination = " << destination << "; w(o,d)=" << this->w_[this->index(origin, destination)] << "; w(d,o)=" << this->w_[this->index(destination, origin)] << std::endl;
				this->check(origin, destination);
				const size_t i = this->index(origin, destination);
				float newWeight = this->w_[i] + value;
				if (newWeight > Options::maxExcWeight)
				{
					newWeight = Options::maxExcWeight;
				}
				else if (newWeight < Options::minExcWeight)
				{
					newWeight = Options::minExcWeight;
				}
				this->w_[i] = newWeight;
			}

			void decWeight(const NeuronId origin, const NeuronId destination, const float value)
			{
				//std::cout << "incWeight: origin = " << origin << "; destination = " << destination << "; w(o,d)=" << this->w_[this->index(origin, destination)] << "; w(d,o)=" << this->w_[this->index(destination, origin)] << std::endl;
				this->check(origin, destination);
				const size_t i = this->index(origin, destination);
				float newWeight = this->w_[i] - value;
				if (newWeight > Options::maxExcWeight)
				{
					newWeight = Options::maxExcWeight;
				}
				else if (newWeight < Options::minExcWeight)
				{
					newWeight = Options::minExcWeight;
				}
				this->w_[i] = newWeight;
			}

			float getWeight(const NeuronId origin, const NeuronId destination) const
			{
				//std::cout << "getWeight: origin = " << origin << "; destination = " << destination << "; w(o,d)=" << this->w_[this->index(origin, destination)] << "; w(d,o)=" << this->w_[this->index(destination, origin)] << std::endl;
				this->check(origin, destination);
				return this->w_[this->index(origin, destination)];
			}

			KernelTime getDelay(const NeuronId origin, const NeuronId destination) const
			{
				this->check(origin, destination);
				return this->delay_[this->index(origin, destination)];
			}

			void setDelay(const NeuronId origin, const NeuronId destination, const KernelTime delay)
			{
				this->check(origin, destination);
				this->delay_[this->index(origin, destination)] = delay;
			}

		private:

			std::vector<float> w_;
			std::vector<unsigned int> wd_plus_;
			std::vector<unsigned int> wd_min_;
			std::vector<KernelTime> lastDeliverTime_;

			std::vector<std::vector<NeuronId>> outgoingNeurons_;
			std::vector<std::vector<NeuronId>> incommingNeurons_;

			std::vector<KernelTime> delay_;

			size_t index(const NeuronId origin, const NeuronId destination) const
			{
				return (origin * Options::nNeurons) + destination;
			}

			void check(const NeuronId origin, const NeuronId destination) const
			{
				::tools::assert::assert_msg(!std::isnan(this->w_[this->index(origin, destination)]), "position not initialized: origin=", origin, "; destination=", destination);
			}
		};
	}
}