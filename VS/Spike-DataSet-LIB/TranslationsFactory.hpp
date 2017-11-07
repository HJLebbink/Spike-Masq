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

#include "../../Spike-Tools-LIB/SpikeTypes.hpp"
#include "../../Spike-Tools-LIB/SpikeRuntimeOptions.hpp"
#include "../../Spike-Tools-LIB/random.ipp"

#include "DataSetState.hpp"
#include "SpikeDataSetMnist.hpp"
#include "SpikeSetLarge.hpp"

namespace spike
{
	namespace dataset
	{
		class TranslationsFactory
		{
		public:

			using TimeType = TimeInMsI;

			TranslationsFactory() = delete;

			static inline std::shared_ptr<Translations<MnistPixel>> createMnistTranslations(
				const std::shared_ptr<const DataSetMnist>& dataSetMnist,
				const spike::tools::SpikeRuntimeOptions& spikeRuntimeOptions)
			{
				const TimeType caseDurationInMs = spikeRuntimeOptions.getCaseDurationInMs();
				const auto translations = std::make_shared<Translations<MnistPixel>>();

				for (const VariableId& variableId : dataSetMnist->getInputVariableIds())
				{
					const TimeType randTime = static_cast<TimeType>(::tools::random::rand_int32(caseDurationInMs));

					const NeuronId neuronId = static_cast<NeuronId>(variableId.val);
					std::vector<NeuronId> neuronIds;
					neuronIds.reserve(1);
					neuronIds.push_back(neuronId);

					// set pixel 0-170 with NO spikes per caseDurationInMs
					const auto spikeSet0 = TranslationsFactory::createMnistSpikeSet(neuronIds, neuronId, 0, caseDurationInMs, randTime);
					for (MnistPixel pixelValue = 0; pixelValue < 170; pixelValue++)
					{
						translations->setTranslation(variableId, pixelValue, spikeSet0);
					}
					const auto spikeSet4 = TranslationsFactory::createMnistSpikeSet(neuronIds, neuronId, 1, caseDurationInMs, randTime);
					for (MnistPixel pixelValue = 170; pixelValue < 256; pixelValue++)
					{
						translations->setTranslation(variableId, pixelValue, spikeSet4);
					}
				}
				return translations;
			}

			template <typename D>
			static inline std::shared_ptr<Translations<D>> createRandomStrategy1(
				const std::shared_ptr<const DataSetState<D>>& dataSetState,
				const spike::tools::SpikeRuntimeOptions& spikeRuntimeOptions)
			{
				const auto translations = std::make_shared<Translations<D>>();
				const Ms caseDurationInMs = spikeRuntimeOptions.getCaseDurationInMs();

				NeuronId nextNeuronId = 0;
				for (const VariableId& variableId : dataSetState->getVariableIds())
				{

					const std::set<D> uniqueValues = dataSetState->getUniqueValues(variableId);

					std::vector<NeuronId> neuronIds;
					for (unsigned int i = 0; i < uniqueValues.size(); ++i)
					{
						neuronIds.push_back(nextNeuronId);
						nextNeuronId++;
					}

					for (const D& value : uniqueValues)
					{
						const auto spikeSet = std::make_shared<SpikeSetLarge>(neuronIds, caseDurationInMs);
						spikeSet->setRandomContent(spikeRuntimeOptions);
						translations->setTranslation(variableId, value, std::move(spikeSet));
					}
				}
				std::cout << "TranslationsFactory::createRandomStrategy1: yields " << nextNeuronId << " neurons" << std::endl;
				//std::cout << translations->toString() << std::endl;
				return translations;
			}

			template <typename D>
			static inline std::shared_ptr<Translations<D>> createRandomStrategy2(
				const std::shared_ptr<const DataSetState<D>>& dataSetState,
				const spike::tools::SpikeRuntimeOptions& spikeRuntimeOptions)
			{
				const auto translations = std::make_shared<Translations<D>>();
				const TimeType caseDurationInMs = spikeRuntimeOptions.getCaseDurationInMs();

				NeuronId nextNeuronId = 0;
				for (const VariableId& variableId : dataSetState->getVariableIds())
				{

					const std::set<D> uniqueValues = dataSetState->getUniqueValues(variableId);
					const unsigned int nUniqueValues = static_cast<unsigned int>(uniqueValues.size());

					std::vector<NeuronId> neuronIds;
					for (unsigned int i = 0; i < nUniqueValues * 40; ++i)
					{
						neuronIds.push_back(nextNeuronId);
						nextNeuronId++;
					}

					for (const D& value : uniqueValues)
					{
						const auto spikeSet = std::make_shared<SpikeSetLarge>(neuronIds, caseDurationInMs);
						spikeSet->setRandomContent(spikeRuntimeOptions);
						translations->setTranslation(variableId, value, std::move(spikeSet));
					}
				}
				std::cout << "TranslationsFactory::createRandomStrategy2: yields " << nextNeuronId << " neurons" << std::endl;
				//std::cout << translations->toString() << std::endl;
				return translations;
			}

		private:

			static inline std::shared_ptr<SpikeSetLarge> createMnistSpikeSet(
				const std::vector<NeuronId>& neuronIds,
				const NeuronId neuronId,
				const unsigned int nSpikes,
				const TimeType durationInMs,
				const TimeType randTime)
			{
				auto spikeSet = std::make_shared<SpikeSetLarge>(neuronIds, durationInMs);
				if (nSpikes == 0)
				{
					return spikeSet;
				}
				else
				{
					const float timeBetweenSpikes = static_cast<float>(durationInMs) / nSpikes;
					const float shiftBetweenTranslations = timeBetweenSpikes / nSpikes;

					for (unsigned int i = 0; i < nSpikes; ++i)
					{
						const TimeType timeInMs = (randTime + lroundf((i * timeBetweenSpikes) + shiftBetweenTranslations)) % durationInMs;
						//if (nSpikes == 4) std::cout << "TranslationFactory::createMnistSpikeSet; timeInMs=" << timeInMs << "; randTime=" << randTime <<"; timeBetweenSpikes=" << timeBetweenSpikes << "; shiftBetweenTranslations=" << shiftBetweenTranslations << std::endl;
						spikeSet->setSpike(neuronId, timeInMs);
					}
					return spikeSet;
				}
			}
		};
	}
}