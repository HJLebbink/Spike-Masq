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

#include <tuple>

#include "Types.hpp"

namespace spike
{
	namespace v3
	{
		template <typename Topology_i>
		class SpikeHistory4
		{
		public:

			using Topology = Topology_i;
			using Options = typename Topology_i::Options;

			//constructor
			SpikeHistory4()
			{
				for (const NeuronId neuronId : Topology::iterator_AllNeurons())
				{
					this->clear(neuronId);
				}
			}

			const std::tuple<KernelTime, KernelTime, KernelTime, KernelTime>& getSpikes(const NeuronId neuronId) const
			{
				return this->data_[neuronId];
			}

			void addSpike(const NeuronId neuronId, const KernelTime kernelTime)
			{
				//TODO the following code could be done with a sse shift left instruction
				auto& tuple = this->data_[neuronId];
				std::get<3>(tuple) = std::get<2>(tuple);
				std::get<2>(tuple) = std::get<1>(tuple);
				std::get<1>(tuple) = std::get<0>(tuple);
				std::get<0>(tuple) = kernelTime;
			}

			void clear(const NeuronId neuronId)
			{
				const KernelTime lastSpikeTime = -1000 * Options::nSubMs;
				this->data_[neuronId] = std::make_tuple(lastSpikeTime, lastSpikeTime, lastSpikeTime, lastSpikeTime);
			}

			void substractTime(const KernelTime time)
			{
				for (const NeuronId neuronId : Topology::iterator_AllNeurons())
				{
					auto& tuple = this->data_[neuronId];
					std::get<3>(tuple) -= time;
					std::get<2>(tuple) -= time;
					std::get<1>(tuple) -= time;
					std::get<0>(tuple) -= time;
				}
			}

		private:

			std::array<std::tuple<KernelTime, KernelTime, KernelTime, KernelTime>, Options::nNeurons> data_;

		};
	}
}