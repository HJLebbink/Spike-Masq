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
#include <iostream> // for cerr and cout
#include <array>
#include <ios>

#include "DataSet.hpp"
#include "DataSetBackendTxt.hpp"

namespace spike
{
	namespace dataset
	{
		using MnistPixel = unsigned short;

		class DataSetMnist
			: public DataSet<DataSetBackendTxt<MnistPixel>, MnistPixel>
		{
		public:

			virtual ~DataSetMnist() = default;
			DataSetMnist() = default;

			void loadMnistSource(
				const std::string& imagesFilename,
				const std::string& labelsFilename)
			{
				// mutex to protect file access
				//static std::mutex mutex;

				// lock mutex before accessing file
				//std::lock_guard<std::mutex> lock(mutex);

				///////////////////////////////////////
				// load imagesFilename
				//
				std::ifstream inputFileStream(imagesFilename, std::ios::binary);
				if (!inputFileStream.is_open())
				{
					std::cerr << "DataSetMnist::loadMnistSource(): Unable to open file " << imagesFilename << std::endl;
				}

				unsigned int magic_number = 0;
				inputFileStream.read((char*)&magic_number, sizeof(magic_number));
				magic_number = ::tools::file::reverseInt(magic_number);

				unsigned int number_of_images = 0;
				inputFileStream.read((char*)&number_of_images, sizeof(number_of_images));
				number_of_images = ::tools::file::reverseInt(number_of_images);

				unsigned int n_rows = 0;
				inputFileStream.read((char*)&n_rows, sizeof(n_rows));
				n_rows = ::tools::file::reverseInt(n_rows);

				unsigned int n_cols = 0;
				inputFileStream.read((char*)&n_cols, sizeof(n_cols));
				n_cols = ::tools::file::reverseInt(n_cols);

				std::cout << "DataSetMnist::loadMnistSource: magic_number = " << magic_number << "; number_of_images = " << number_of_images << "; n_rows = " << n_rows << "; n_cols = " << n_cols << std::endl;

				///////////////////////////////////////
				// load labelsFilename
				//
				std::ifstream labelFileStream(labelsFilename, std::ios::binary);
				if (!labelFileStream.is_open())
				{
					std::cerr << "DataSetMnist::loadMnistSource(): Unable to open file " << labelsFilename << std::endl;
				}

				unsigned int magic_number2 = 0;
				labelFileStream.read((char*)&magic_number2, sizeof(magic_number2));
				magic_number2 = ::tools::file::reverseInt(magic_number2);

				unsigned int number_of_images2 = 0;
				labelFileStream.read((char*)&number_of_images2, sizeof(number_of_images2));
				number_of_images2 = ::tools::file::reverseInt(number_of_images2);

				std::cout << "DataSetMnist::loadMnistSource: magic_number = " << magic_number2 << "; number_of_images = " << number_of_images2 << std::endl;

				if (number_of_images != number_of_images2)
				{
					throw std::runtime_error("unequal number of images");
				}

				///////////////////////////////////////
				// process imagesFilename
				//
				for (unsigned int r = 0; r < n_rows; ++r)
				{
					for (unsigned int c = 0; c < n_cols; ++c)
					{
						const VariableId variableId = this->createVariableId(r, c, n_cols);
						this->setVariableValue(variableId, Variable::PropertyName::Used, "1");
						this->setVariableValue(variableId, Variable::PropertyName::VariableType, "INPUT");
					}
				}

				for (CaseIdType i = 0; i < number_of_images; ++i)
				{
					const CaseId caseId = CaseId(i);
					for (unsigned int r = 0; r < n_rows; ++r)
					{
						for (unsigned int c = 0; c < n_cols; ++c)
						{
							const VariableId variableId = this->createVariableId(r, c, n_cols);
							unsigned char temp = 0;
							inputFileStream.read((char*)&temp, sizeof(temp));
							const MnistPixel pixelValue = static_cast<MnistPixel>(temp);
							this->setData(caseId, variableId, pixelValue);
						}
					}
				}

				///////////////////////////////////////
				// process labelsFilename
				//
				const VariableId outputVariableId = static_cast<VariableId>(n_rows * n_cols);

				this->setVariableValue(outputVariableId, Variable::PropertyName::Used, "1");
				this->setVariableValue(outputVariableId, Variable::PropertyName::VariableType, "OUTPUT");

				for (CaseIdType i = 0; i < number_of_images; ++i)
				{
					const CaseId caseId = CaseId(i);
					unsigned char temp = 0;
					labelFileStream.read((char*)&temp, sizeof(temp));
					const MnistPixel pixelValue = static_cast<MnistPixel>(temp);
					this->setData(caseId, outputVariableId, pixelValue);
				}
			}

			void setOutputVariablesToMissing()
			{
				const VariableId outputVariableId = this->getOutputVariableId();
				const MnistPixel missingValue = this->getOptions().getMissingValue();
				//std::cout << "setOutputVariablesToMissing: outputVariable = " << outputVariableId << "; missing value = " << missingValue << std::endl;
				for (CaseIdType i = 0; i < this->getNumberOfCases(); ++i)
				{
					this->setData(CaseId(i), outputVariableId, missingValue);
				}
			}

			DataSetMnist rescale28to14() const
			{
				DataSetMnist dataSetMnist;
				dataSetMnist.setOptions(this->getOptions());

				const unsigned int nPixels = this->getNumberOfVariables() - 1; // the last variable is the classification of the image.
				if (nPixels != (28 * 28))
				{
					std::cout << "rescale28to14: nPixels " << nPixels << " is not equal to 784" << std::endl;
					return dataSetMnist;
				}

				for (int x = 0; x < 14; x++)
				{
					for (int y = 0; y < 14; y++)
					{
						const VariableId newVariableId = this->createVariableId(x, y, 14);
						dataSetMnist.setVariableValue(newVariableId, Variable::PropertyName::Used, "1");
						dataSetMnist.setVariableValue(newVariableId, Variable::PropertyName::VariableType, "INPUT");
					}
				}

				const VariableId outputVariableIdNew = static_cast<VariableId>(14 * 14);
				const VariableId outputVariableIdOld = static_cast<VariableId>(28 * 28);
				dataSetMnist.setVariableValue(outputVariableIdNew, Variable::PropertyName::Used, "1");
				dataSetMnist.setVariableValue(outputVariableIdNew, Variable::PropertyName::VariableType, "OUTPUT");

				for (CaseIdType i = 0; i < this->getNumberOfCases(); i++)
				{
					const CaseId caseId = CaseId(i);

					//1] set the input variables
					for (int x = 0; x < 14; x++)
					{
						for (int y = 0; y < 14; y++)
						{

							const VariableId variableId1 = this->createVariableId((2 * x) + 0, (2 * y) + 0, 28);
							const VariableId variableId2 = this->createVariableId((2 * x) + 1, (2 * y) + 0, 28);
							const VariableId variableId3 = this->createVariableId((2 * x) + 0, (2 * y) + 1, 28);
							const VariableId variableId4 = this->createVariableId((2 * x) + 1, (2 * y) + 1, 28);

							const MnistPixel mnistPixel1 = this->getData(caseId, variableId1);
							const MnistPixel mnistPixel2 = this->getData(caseId, variableId2);
							const MnistPixel mnistPixel3 = this->getData(caseId, variableId3);
							const MnistPixel mnistPixel4 = this->getData(caseId, variableId4);

							const MnistPixel newPixelValue = (mnistPixel1 + mnistPixel2 + mnistPixel3 + mnistPixel4) / 4;
							const VariableId newVariableId = this->createVariableId(x, y, 14);

							dataSetMnist.setData(caseId, newVariableId, newPixelValue);
						}
					}
					//2] set the output variable
					dataSetMnist.setData(caseId, outputVariableIdNew, this->getData(caseId, outputVariableIdOld));
				}
				return dataSetMnist;
			}

			VariableId getOutputVariableId() const
			{
				const unsigned int nVariables = this->getNumberOfVariables();
				if (nVariables == ((28 * 28) + 1))
				{
					return static_cast<VariableId>(28 * 28);
				}
				else if (nVariables == (14 * 14) + 1)
				{
					return static_cast<VariableId>(14 * 14);
				}
				else
				{
					std::cerr << "getOutputVariableId: unsupported number of variables " << nVariables << std::endl;
					throw std::runtime_error("unsupported number of variables");
				}
			}

			unsigned int getNumberOfPixels() const
			{
				return this->getNumberOfVariables() - 1;
			}

			std::vector<VariableId> getInputVariableIds() const
			{
				std::vector<VariableId> inputVariableIds;

				const unsigned int nVariables = this->getNumberOfVariables();
				if (nVariables == ((28 * 28) + 1))
				{
					for (VariableIdType i = 0; i < (28 * 28); ++i)
					{
						inputVariableIds.push_back(VariableId(i));
					}
				}
				else if (nVariables == (14 * 14) + 1)
				{
					for (VariableIdType i = 0; i < (14 * 14); ++i)
					{
						inputVariableIds.push_back(VariableId(i));
					}
				}
				else
				{
					std::cerr << "getInputVariableIds: unsupported number of variables " << nVariables << std::endl;
					throw std::runtime_error("unsupported number of variables");
				}
				return inputVariableIds;
			}

		private:

			inline VariableId createVariableId(const unsigned int r, const unsigned int c, const unsigned int n_cols) const
			{
				return static_cast<VariableId>((r * n_cols) + c);
			}
		};
	}
}