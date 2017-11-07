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
#include <limits>		// std::numeric_limits
#include <sstream>		// std::ostringstream
#include <iostream>		// std::cout, std::fixed
#include <iomanip>		// std::setprecision

#include "../../Spike-Tools-LIB/SpikeTypes.hpp"

namespace spike
{
	namespace v3
	{
		using NeuronId = unsigned int;
		using TimeInMs = float; // precision 
		using KernelTime = int;
		using TimeInSec = unsigned int; // max time is 136.19 year for unsigned int;

		using Delay = unsigned int;
		using Efficacy = float;
		using Voltage = float;

		const KernelTime LAST_KERNEL_TIME = std::numeric_limits<KernelTime>::max();
		const KernelTime NO_KERNEL_TIME = std::numeric_limits<KernelTime>::max();
		const NeuronId NO_NEURON = std::numeric_limits<NeuronId>::max();

		struct IncommingSpike
		{
			KernelTime kerneltime;
			NeuronId origin;
			NeuronId destination;
			Efficacy efficacy;

			// default constructor
			IncommingSpike()
				: kerneltime(NO_KERNEL_TIME)
				, origin(NO_NEURON)
				, destination(NO_NEURON)
				, efficacy(0)
			{
			}

			// constructor
			IncommingSpike(
				const KernelTime kerneltime,
				const NeuronId origin,
				const NeuronId destination,
				const Efficacy efficacy)
				: kerneltime(kerneltime)
				, origin(origin)
				, destination(destination)
				, efficacy(efficacy)
			{
			}

			bool operator< (const IncommingSpike& spike) const
			{
				if (this->kerneltime < spike.kerneltime)
				{
					return true;
				}
				else if (this->kerneltime > spike.kerneltime)
				{
					return false;
				}
				else
				{
					if (this->origin > spike.origin)
					{
						return true;
					}
					else if (this->origin < spike.origin)
					{
						return false;
					}
					else
					{
						return (this->destination > spike.origin);
					}
				}
			}
			bool operator== (const IncommingSpike& spike) const
			{
				return ((this->kerneltime == spike.kerneltime) && (this->origin == spike.origin) && (this->destination == spike.destination));
			}
			bool operator!= (const IncommingSpike& spike) const
			{
				return ((this->kerneltime != spike.kerneltime) || (this->origin != spike.origin) || (this->destination != spike.destination));
			}

			std::string toString() const
			{
				std::ostringstream oss;
				oss << "IncommingSpike(kerneltime " << std::setprecision(15) << this->kerneltime << "; origin " << this->origin << "; destination " << destination << "; efficacy " << efficacy << ")";
				return oss.str();
			}
		};

		class CompareIncommingSpike
		{
		public:
			bool operator()(IncommingSpike& t1, IncommingSpike& t2)
			{
				return t2 < t1;
			}
		};

		struct PostSynapticSpike
		{
			KernelTime kerneltime;
			NeuronId neuronId;
			FiringReason firingReason;

			// constructor
			PostSynapticSpike()
				: kerneltime(NO_KERNEL_TIME)
				, neuronId(NO_NEURON)
				, firingReason(FiringReason::NO_FIRE)
			{
			}

			// constructor
			PostSynapticSpike(
				const KernelTime kerneltime,
				const NeuronId neuronId,
				const FiringReason firingReason
			): kerneltime(kerneltime)
				, neuronId(neuronId)
				, firingReason(firingReason)
			{
			}

			std::string toString() const
			{
				std::ostringstream oss;
				oss << "PostSynapticSpike(kerneltime " << std::setprecision(15) << this->kerneltime << "; neuronId " << this->neuronId << "; firingReason " << static_cast<int>(firingReason) << ")";
				return oss.str();
			}
		};

		struct Pathway
		{
			NeuronId origin;			// presynaptic neuronId
			NeuronId destination;	// neuronId
			Delay delay;				// delay from presynaptic neuron to neuron
			Efficacy efficacy;		// weight 

			// constructor
			Pathway()
				: origin(NO_NEURON)
				, destination(NO_NEURON)
				, delay(0)
				, efficacy(0)
			{
			}

			// constructor
			Pathway(
				const NeuronId origin,
				const NeuronId destination,
				const Delay delay,
				const Efficacy efficacy
			): origin(origin)
				, destination(destination)
				, delay(delay)
				, efficacy(efficacy)
			{
			}

			std::string toString() const
			{
				std::ostringstream oss;
				oss << "Pathway(origin " << this->origin << "; destination " << this->destination << "; delay " << std::setprecision(15) << delay << "; efficacy " << efficacy << ")";
				return oss.str();
			}
		};
	}
}