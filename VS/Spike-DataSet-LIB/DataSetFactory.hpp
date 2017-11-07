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

#include "Options.hpp"
#include "DataSet.hpp"

namespace spike
{
	namespace dataset
	{
		class DataSetFactory
		{
		public:

			DataSetFactory() = delete;
			~DataSetFactory() = delete;

			template <class B, class D>
			static inline DataSet<B, D> createRandomDataSet(
				const unsigned int nVariables,
				const unsigned int nCases,
				const unsigned int maxValue,
				const float percentageOfMissingValues,
				const Options<D>& dataSetOptions)
			{
				const bool useMissingValues = (percentageOfMissingValues > 0);
				const float missingValuesThreshold = percentageOfMissingValues / 100;

				DataSet<B, D> dataSet;
				dataSet.setOptions(dataSetOptions);

				//1] init the dataSet1 with random content
				for (VariableIdType i = 0; i < nVariables; ++i)
				{
					const VariableId variableId(i);

					// 1.1] set the input/output and the group
					dataSet.setVariableValue(variableId, Variable::PropertyName::Used, "1");

					if (i == (nVariables - 1))
					{
						dataSet.setVariableValue(variableId, Variable::PropertyName::VariableType, Variable::variableTypeEnumToString(Variable::VariableTypeEnum::output));
						dataSet.setVariableValue(variableId, Variable::PropertyName::VariableGroup, "none");
					}
					else if (i == (nVariables - 2))
					{
						dataSet.setVariableValue(variableId, Variable::PropertyName::VariableType, Variable::variableTypeEnumToString(Variable::VariableTypeEnum::nuisance));
						dataSet.setVariableValue(variableId, Variable::PropertyName::VariableGroup, "none");
					}
					else
					{
						dataSet.setVariableValue(variableId, Variable::PropertyName::VariableType, Variable::variableTypeEnumToString(Variable::VariableTypeEnum::input));
						const std::string group = (::tools::random::rand_int32(2) == 1) ? "a" : "f";
						dataSet.setVariableValue(variableId, Variable::PropertyName::VariableGroup, group);
					}

					//1.2] set the description
					std::ostringstream oss;
					oss << "variable " << variableId;
					const std::string propertyString = oss.str();
					dataSet.setVariableValue(variableId, Variable::PropertyName::Description, propertyString);

					//1.3] fill the data 
					for (CaseIdType j = 0; j < nCases; ++j)
					{
						const CaseId caseId(j);
						if (useMissingValues && (::tools::random::rand_float() < missingValuesThreshold))
						{
							// do nothing: we have a missing value
						}
						else
						{
							const D randValue = static_cast<D>(::tools::random::rand_int32(maxValue));
							//const D randValue = randFloat();
							//const D randValue = "a";
							dataSet.setData(caseId, variableId, randValue);
						}
					}
				}
				return dataSet;
			}
		};
	}
}