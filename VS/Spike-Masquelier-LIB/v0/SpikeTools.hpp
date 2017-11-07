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

#include <stdlib.h>     /* srand, rand */
#include <climits>
#include <vector>
#include <string>
#include <time.h>
#include <iostream> // for cerr and cout

namespace spike
{
	namespace v0
	{
		using SpikeEvent = unsigned long long;
		using SpikeTime = unsigned int;
		using SpikeNeuronId = unsigned int;
		using SpikePotential = int;
		using Weight = unsigned short;
		using WeightDelta = short;
		using KernelTime = int;
		using SpikeEpsilon = unsigned short;

		struct TJData2
		{
			SpikeTime t_j_time;
			SpikeNeuronId t_j_epspAfferent;
		};

		const SpikeTime LAST_INF_TIME = UINT_MAX;
		const SpikeTime FIRST_INF_TIME = 0;

		const SpikeEvent LAST_INF_EVENT = ULLONG_MAX;
		const SpikeEvent FIRST_INF_EVENT = 0;

		SpikeEvent makeSpikeEvent(SpikeTime time, SpikeNeuronId originatingNeuronId)
		{
			return (((SpikeEvent)time) << 32) | ((SpikeEvent)originatingNeuronId);
		}
		SpikeTime getTimeFromSpikeEvent(SpikeEvent event)
		{
			return (SpikeTime)(event >> 32);
		}
		SpikeNeuronId getOriginatingNeuronIdFromSpikeEvent(SpikeEvent event)
		{
			return (SpikeNeuronId)event;
		}
		/**
		* @param event1
		* @param event2
		* @return true if event1 is before event2; false otherwise
		*/
		bool beforeTime(SpikeEvent event1, SpikeEvent event2)
		{
			return event1 < event2;
		}
		bool equalNeuronId(SpikeEvent event1, SpikeEvent event2)
		{
			return ((int)event1) == ((int)event2);
		}

		std::string event2String(SpikeEvent event)
		{
			const std::string s = "SpikeEvent(time=" + std::to_string(getTimeFromSpikeEvent(event)) + "; neuron=" + std::to_string(getOriginatingNeuronIdFromSpikeEvent(event)) + ")";
			//	std::cout << s << std::endl;
			return s;
		}
	}
}