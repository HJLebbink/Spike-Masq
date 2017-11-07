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

#include <ratio>

#include "Types.hpp"

namespace spike
{
	namespace v3
	{
		template <size_t Ne_i, size_t Ni_i, size_t Ns_i, size_t Nm_i>
		class SpikeOptionsStatic
		{
		public:
			// topology options
			static const size_t Ne = Ne_i;
			static const size_t Ni = Ni_i;
			static const size_t Ns = Ns_i;
			static const size_t Nm = Nm_i;

			static const size_t nNeurons = Ne + Ni + Ns + Nm;
			static const size_t nSynapses = 100; // number of synapses
			static const Delay maxDelay = 20; // maxDelay
			static const Delay minDelay = 1; // original Izhikevich experiment minDelay = 0;

			// misc options
			static constexpr float randomSpikeHz = 1.0f;
			static constexpr float trainRate = 0.005f;
			static constexpr float refractoryPeriod = 5.0; // refractory period has to be larger than the smallest delay

			// threshold options
			static constexpr float minimalThreshold = 8; // the threshold is at least this number
			static const int tau_t1 = 1; //tau_t1 is the added threshold (in voltage) to time 0
			static const int tau_t2 = 100; //tau_t2 is de decay of the threshold 

			static constexpr float minVoltage = -100.0f;

			// weight options
			static constexpr float initialWeightExc = 1.0f;
			static constexpr float initialWeightInh = -1.0f;
			static constexpr float maxExcWeight = 2.0f;
			static constexpr float minExcWeight = 0.0f;

			// kernel options
			static const int nSubMs = 100;
			static constexpr TimeInMs kernelRangeEtaInMs = 100.0f;
			static constexpr TimeInMs kernelRangeEpsilonInMs = 100.0f;
			static constexpr TimeInMs kernelRangeThresholdInMs = 400.0f;
			static constexpr TimeInMs kernelRangeStdpInMs = 40.0f;
			static constexpr float tau_m = 10.0f;
			static constexpr float tau_s = 4.0f;
			static constexpr float log_tau_m_div_tau_s = 0.9162907318741551; // = log(tau_m / tau_s) for tau_m=10; tau_s=4; 

			// delay of the top after the receival of the spike
			static constexpr TimeInMs topDelay = (tau_m * tau_s) / (tau_m - tau_s) * log_tau_m_div_tau_s;
			static constexpr float k = 1.0f;// / this->doubleExponential(calcTopTime(0));

			// threading options
			static const bool useOpenMP = false;
			static const int maxNumberOfThreads = 4;


			static const bool tranceNeuronOn = false;
			static const NeuronId tranceNeuron = 116;

			//constructor
			SpikeOptionsStatic()
			{
				{
					const float expected_log_tau_m_div_tau_s = static_cast<float>(log(tau_m / tau_s));
					const float diff = SpikeOptionsStatic::log_tau_m_div_tau_s - expected_log_tau_m_div_tau_s;
					if (diff > 1e-7)
					{
						printf("SpikeOptionsStatic: incorrect constant: log_tau_m_div_tau_s=%.16f, while it should have been log(tau_m=%f/tau_s=%f) = %.16f (diff %.16f)\n", SpikeOptionsStatic::log_tau_m_div_tau_s, tau_m, tau_s, expected_log_tau_m_div_tau_s, diff);
						//DEBUG_BREAK();
					}
				}
				{
					const float expected_k = static_cast<float>(1.0f / this->doubleExponential(calcTopTime(0)));
					const float diff = SpikeOptionsStatic::k - expected_k;
					if (diff > 1e-7)
					{
						printf("SpikeOptionsStatic: incorrect constant: k=%.16f, while it should have been %.16f (diff %.16f)\n", SpikeOptionsStatic::k, this->doubleExponential(calcTopTime(0)), diff);
						//DEBUG_BREAK();
					}
				}
				{
					const TimeInMs topTime = calcTopTime(0);
					const Voltage voltageAtTop = epsilon_f(topTime);
					const float diff = voltageAtTop - 1.0f;
					if (diff > 1e-7)
					{
						printf("SpikeOptionsStatic: voltage at top should have been 1; constructor: k=%.16f; toptime=%.16f; voltage at toptime=%.16f; diff=%.16f\n", SpikeOptionsStatic::k, topTime, voltageAtTop, diff);
						//DEBUG_BREAK();
					}
				}
			}

			static constexpr KernelTime toKernelTime(const TimeInMs t)
			{
				return static_cast<KernelTime>(t * SpikeOptionsStatic::nSubMs);
			}

			static constexpr TimeInMs toTimeInMs(const KernelTime t)
			{
				return static_cast<TimeInMs>(t) / SpikeOptionsStatic::nSubMs;
			}

			TimeInMs constexpr calcTopTime(const TimeInMs incommingTime) const
			{
				return incommingTime + topDelay;
			}

			Voltage epsilon_f(const TimeInMs x) const
			{
				if (x <= 0)
				{
					return 0;
				}
				const Voltage v = SpikeOptionsStatic::k * this->doubleExponential(x);
				/*
				if (std::isnormal(v)) {
				return v;
				} else {
				std::cout << "epsilon: x=" << x << "; v=" << v << std::endl;
				return 0;
				}
				*/
				return v;
			}

			Voltage mu_f(const TimeInMs x) const
			{
				return epsilon_f(x); // todo: use proper mu
			}



			Voltage eta_f(const TimeInMs x) const
			{
				// x=0 means the first time instant after the refractory period; 

				if (x < 0)
				{
					return this->minVoltage;
				}
				Voltage v = static_cast<Voltage>(-500) / x;
				if (v < this->minVoltage)
				{
					v = this->minVoltage;
				}
				else if (v > 0)
				{
					v = 0;
				}

				//const Voltage v = -10 * this->k_ * this->doubleExponential(x);
				//const Voltage v =  50 * (this->threshold * (2 * exp(-x / this->tauM_) - 4 * (exp(-x / this->tauM_) - exp(-x / this->tauS_))));
				/*
				if (std::isnormal(v)) {
				return v;
				} else {
				std::cout << "eta: x=" << x << "; v=" << v << std::endl;
				return 0;
				}
				*/
				//if (v > 0) std::cout << "eta: voltage=" << v << "; x=" << x << std::endl;
				return v;
			}

			Voltage threshold_f(const TimeInMs x) const
			{
				if (x < 0)
				{
					return 0;
				}
				// x=0 means the first time instant after the refractory period; 
				// x<0 means during the refractory period, and yields the highest posible voltage
				return (tau_t1 * tau_t2) / (x + tau_t2);
			}

			float calcWeightDeltaLtp(const TimeInMs contributionTime, const TimeInMs fireTime) const
			{
				const TimeInMs tau_p_local = 30;
				const TimeInMs tau_star_p = this->trainRate * 2.0f / (tau_p_local * tau_p_local);

				::tools::assert::assert_msg(contributionTime <= fireTime, "calcWeightDeltaLtp: contributionTime=", contributionTime, " has to be before fireTime=", fireTime);
				const TimeInMs relativeDeliverTime = fireTime - contributionTime;
				::tools::assert::assert_msg(relativeDeliverTime >= 0, "calcWeightDeltaLtp: relativeDeliverTime=", relativeDeliverTime, " has to be larger than zero");

				if (relativeDeliverTime < tau_p_local)
				{
					const float result = tau_star_p * ((tau_p_local - relativeDeliverTime + 1) * (tau_p_local - relativeDeliverTime + 1));
					//std::cout << "spike::v3::calcWeightDeltaLtp: return " << result << "; relativeDeliverTime=" << relativeDeliverTime << "; tau_star_p=" << tau_star_p << "; contributionTime=" << contributionTime << "; fireTime=" << fireTime << std::endl;
					return result;
				}
				else
				{
					return 0;
				}
			}

			float calcWeightDeltaLtd(const TimeInMs spikeTime, const TimeInMs t) const
			{
				const TimeInMs tau_m_local = 15;
				const TimeInMs tau_star_m = this->trainRate * 3.0f / (tau_m_local * tau_m_local);

				::tools::assert::assert_msg(spikeTime <= static_cast<int>(t), "calcWeightDeltaLtd: spikeTime=", spikeTime, " has to be after (or equal to) current time=", t);
				const TimeInMs relativeDeliverTime = spikeTime - t;
				::tools::assert::assert_msg(relativeDeliverTime <= 0, "calcWeightDeltaLtd: relativeDeliverTime=", relativeDeliverTime, " has to be smaller (or equal to) zero. spikeTime=", spikeTime, "; t=", t);

				//std::cout << "spike::v3::Network::calcWeightDeltaLtd: relativeDeliverTime=" << relativeDeliverTime << "; spikeTime=" << spikeTime << "; t=" << t << std::endl;
				if (relativeDeliverTime > -tau_m_local)
				{
					const float result = tau_star_m * ((relativeDeliverTime + tau_m_local) * (relativeDeliverTime + tau_m_local));
					//std::cout << "calcWeightDeltaLtd: return " << result << "; relativeDeliverTime=" << relativeDeliverTime << "; tau_star_m=" << tau_star_m << "; spikeTime=" << spikeTime << "; t=" << t << std::endl;
					return result;
				}
				else
				{
					return 0;
				}
			}

		private:

			Voltage static doubleExponential(const TimeInMs x)
			{
				return (exp(-x / tau_m) - exp(-x / tau_s));
			}

		};
	}
}