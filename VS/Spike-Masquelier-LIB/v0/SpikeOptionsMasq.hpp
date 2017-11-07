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

#include <stdio.h>
#include "../../Spike-Masquelier-LIB/v0/SpikeTools.hpp"

namespace spike
{
	namespace v0
	{
		class SpikeOptionsMasq
		{
		private:
		public:

			static constexpr float tau_m = 10.0f;
			static constexpr float tau_minus = 33.7f;
			static constexpr float tau_plus = 16.8f;

			static constexpr float T_MAX_OPTIMIZATION = 7 * tau_m;


			static constexpr int POTENTIAL_DENOMINATOR_POW = 15; // dont use 16 bits potential because that may not fit in 16 register
			static constexpr int POTENTIAL_DENOMINATOR = 1 << POTENTIAL_DENOMINATOR_POW;

			static constexpr int TIME_DENOMINATOR_POW = 2;
			static constexpr int TIME_DENOMINATOR = 1 << TIME_DENOMINATOR_POW;
			static constexpr int KERNEL_SCAN_INTERVAL_POW = 2;
			static constexpr int KERNEL_SCAN_INTERVAL = 1 << KERNEL_SCAN_INTERVAL_POW;

			static constexpr int KERNEL_SIZE_TMP = 7 * static_cast<int>(tau_m) * KERNEL_SCAN_INTERVAL;
			static constexpr int KERNEL_SIZE = ((KERNEL_SIZE_TMP >> 4) + 1) << 4;
			static constexpr int LTD_KERNEL_SIZE = 7 * static_cast<int>(tau_minus) * TIME_DENOMINATOR;
			static constexpr int LTP_KERNEL_SIZE = 7 * static_cast<int>(tau_plus) * TIME_DENOMINATOR;

			static constexpr int T_J_LENGTH = 50000;

			float alpha;
			float alpha_minus;
			float alpha_plus;

			float k;
			float k1;
			float k2;

			SpikeTime refractory_period;

			//float t_max_optimization;

			float tau_s;

			float threshold;
			SpikePotential thresholdInt;

			bool beSmart;

			bool quiet;

			//--------------------------------
			static const bool traceNeuronOn = false;
			static const unsigned int traceNeuronId = 2000;
			static const unsigned int traceNeuronId2 = 0; //neuron id used for pathways to traceNeuronId
			//--------------------------------


			SpikeOptionsMasq(
				const float alpha,
				const float alpha_plus,
				const float alpha_minus,

				const float k,
				const float k1,
				const float k2,

				const SpikeTime refractory_period,

				const float tau_s,

				const float threshold,

				const bool beSmart,
				const bool quiet
			):
				alpha(alpha),
				alpha_plus(alpha_plus),
				alpha_minus(alpha_minus),

				k(k),
				k1(k1),
				k2(k2),

				refractory_period(refractory_period),

				tau_s(tau_s),

				//t_max_optimization(T_MAX_OPTIMIZATION),

				threshold(threshold),
				thresholdInt((SpikePotential)(threshold * POTENTIAL_DENOMINATOR)),

				beSmart(beSmart),
				quiet(quiet)
			{
				//this->printOptions(stdout);
			}

			~SpikeOptionsMasq() = default;

			std::string toString() const
			{
				std::stringstream sstm;
				sstm << std::endl;
				sstm << "========================================================" << std::endl;
				sstm << "tau_m = " << tau_m << std::endl;
				sstm << "tau_s = " << this->tau_s << std::endl;
				sstm << "tau_minus = " << tau_minus << std::endl;
				sstm << "tau_plus = " << tau_plus << std::endl;
				sstm << std::endl;

				sstm << "k = " << this->k << std::endl;
				sstm << "t_max_optimization = " << T_MAX_OPTIMIZATION << std::endl;
				sstm << std::endl;

				sstm << "alpha = " << SpikeOptionsMasq::alpha << std::endl;
				sstm << "alpha_minus = " << this->alpha_minus << std::endl;
				sstm << "alpha_plus = " << this->alpha_plus << std::endl;
				sstm << std::endl;

				sstm << "k1 = " << this->k1 << std::endl;
				sstm << "k2 = " << this->k2 << std::endl;
				sstm << "refractory_period = " << this->refractory_period << std::endl;
				sstm << std::endl;

				sstm << "threshold = " << this->threshold << std::endl;
				sstm << "thresholdInt = " << this->thresholdInt << std::endl;
				sstm << "TIME_DENOMINATOR_POW = " << TIME_DENOMINATOR_POW << std::endl;
				sstm << "TIME_DENOMINATOR = " << TIME_DENOMINATOR << std::endl;
				sstm << "KERNEL_SIZE = " << KERNEL_SIZE << std::endl;
				sstm << "LTD_KERNEL_SIZE = " << LTD_KERNEL_SIZE << std::endl;
				sstm << "LTP_KERNEL_SIZE = " << LTP_KERNEL_SIZE << std::endl;
				sstm << std::endl;

				sstm << "T_J_LENGTH = " << T_J_LENGTH << std::endl;
				sstm << "beSmart = " << this->beSmart << std::endl;

				sstm << std::endl;
				sstm << "quiet = " << this->quiet << std::endl;

				sstm << "========================================================" << std::endl;

				return sstm.str();
			}
		};
	}
}