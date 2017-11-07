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

#include <memory>

#include "../../Spike-Tools-LIB/SpikeTypes.hpp"
#include "../../Spike-Tools-LIB/SpikeRuntimeOptions.hpp"

#include "SpikeStream.hpp"

namespace spike
{
	namespace dataset
	{
		template <typename Topology_i>
		class SpikeStreamFactory
		{
		public:

			using Topology = Topology_i;

			SpikeStreamFactory() = delete;

			static inline const std::shared_ptr<SpikeStream<Topology>> createRandomSpikeStream(
				const unsigned int nCases,
				const bool createRandomCase,
				const std::vector<NeuronId>& neuronIds,
				const SpikeRuntimeOptions& spikeRuntimeOptions)
			{
				const auto spikeStream = std::make_shared<SpikeStream<Topology>>(spikeRuntimeOptions);
				CaseIdType caseId = 0;

				//1] fill the case data
				if (nCases > 0)
				{
					const Ms caseDurationInMs = spikeRuntimeOptions.getCaseDurationInMs();
					if (caseDurationInMs == 0)
					{
						std::cerr << "SpikeStreamFactory::createRandomSpikeStream: cannot create case with duration of 0ms";
						throw std::runtime_error("cannot create case with duration of 0ms");
					}
					else
					{

						const Ms caseTailSilenceInMs = spikeRuntimeOptions.getCaseTailSilenceInMs();
						const double targetHz = spikeRuntimeOptions.getRandomSpikeHz();
						const unsigned int randThreshold = static_cast<unsigned int>(std::lround(1000.0 / targetHz));

						unsigned int spikeCount = 0;
						unsigned int noSpikeCount = 0;

						for (unsigned int i = 0; i < nCases; ++i)
						{

							// use the case is as a case label
							const CaseLabel caseLabel = CaseLabel(static_cast<CaseLabelType>(caseId));
							auto spikeCase = std::make_shared<SpikeCaseFast>(CaseId(caseId), caseLabel, neuronIds, caseDurationInMs, caseTailSilenceInMs);
							caseId++;

							for (const NeuronId& neuronId : neuronIds)
							{
								const bool spikes = (::tools::random::rand_int32(randThreshold) == 0);

								if (spikes)
								{
									spikeCount++;
									const Ms ms = static_cast<Ms>(::tools::random::rand_int32(caseDurationInMs));
									spikeCase->setData(neuronId, ms, true);
								}
								else
								{
									noSpikeCount++;
								}
							}
							spikeStream->add(std::move(spikeCase));
						}

						const double observedHz = (static_cast<float>(spikeCount) / (noSpikeCount + spikeCount)) / 1000;
						std::cout << "SpikeStreamFactory::createRandomSpikeStream: targetHz = " << targetHz << "; observedHz = " << observedHz << std::endl;
					}
				}

				//2] create one random case
				if (createRandomCase)
				{
					const Ms randomCaseDurationInMs = spikeRuntimeOptions.getRandomCaseDurationInMs();
					if (randomCaseDurationInMs == 0)
					{
						std::cerr << "SpikeStreamFactory::createRandomSpikeStream: cannot create random case with duration of 0ms";
						throw std::runtime_error("cannot create random case with duration of 0ms");
					}
					else
					{
						const Ms caseTailSilenceDurationInMs = 0;
						const CaseLabel caseLabel = NO_CASE_LABEL;
						const auto spikeCase = std::make_shared<SpikeCaseFast>(CaseId(caseId), caseLabel, neuronIds, randomCaseDurationInMs, caseTailSilenceDurationInMs);
						const double randomHz = spikeRuntimeOptions.getRandomSpikeHz();
						spikeCase->setAllNeuronsRandomSpikeHz(randomHz);
						spikeStream->add(std::move(spikeCase));
					}
				}

				return spikeStream;
			}
		};
	}
}
