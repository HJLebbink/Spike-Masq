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
#include <algorithm> // for std::replace

#include "DataSetState.hpp"

namespace spike
{
	namespace dataset
	{
		template <class D>
		class DataSetBackendTxt
		{
		public:

			DataSetBackendTxt() = default;

			void saveToFileBackend(
				const std::string& filename,
				const DataSetState<D>& dataSetState) const
			{
				// mutex to protect file access
				//static std::mutex mutex;

				// lock mutex before accessing file
				//std::lock_guard<std::mutex> lock(mutex);

				// create the directory
				const std::string tree = ::tools::file::getDirectory(filename);
				std::cout << "making directory " << tree << " for filename " << filename << std::endl;
				if (!::tools::file::mkdirTree(tree))
				{
					std::cerr << "DataSetBackendTxt::saveToFileBackend(): Unable to create directory " << tree << std::endl;
					throw std::runtime_error("unable to create directory");
				}

				// try to open file
				std::ofstream outputFile(filename); //fstream is a proper RAII object, it does close automatically at the end of the scope
				if (!outputFile.is_open())
				{
					std::cerr << "DataSetBackendTxt::saveToFileBackend(): Unable to open file " << filename << std::endl;
					throw std::runtime_error("unable to open file");
				}
				else
				{
					std::cout << "DataSetBackendTxt::saveToFileBackend(): Opening file " << filename << std::endl;

					const std::vector<CaseId> caseIds = dataSetState.getCaseIds();
					const unsigned int nCases = dataSetState.getNumberOfCases();
					const std::set<VariableId> variableIds = dataSetState.getVariableIds();
					const unsigned int nVariables = dataSetState.getNumberOfVariables();
					const D missingValue = dataSetState.getOptions().getMissingValue();

					//1] print the number of cases and the number of variables
					outputFile << "#<nCases> <nVariables> <missingValue>" << std::endl;
					outputFile << nCases << " " << nVariables << " " << missingValue << std::endl;

					//2] print the property descriptions
					outputFile << "#<variableId> <used> <description> <type> <group>" << std::endl;
					for (const VariableId& variableId : variableIds)
					{

						std::string used = dataSetState.getVariableValue(variableId, Variable::PropertyName::Used);
						std::replace(used.begin(), used.end(), ' ', '_');
						if (used.empty()) used = "0";

						std::string descr = dataSetState.getVariableValue(variableId, Variable::PropertyName::Description);
						std::replace(descr.begin(), descr.end(), ' ', '_');
						if (descr.empty()) descr = "no_description";

						std::string type = dataSetState.getVariableValue(variableId, Variable::PropertyName::VariableType);
						std::replace(type.begin(), type.end(), ' ', '_');
						if (type.empty()) type = "no_variable_type";

						std::string group = dataSetState.getVariableValue(variableId, Variable::PropertyName::VariableGroup);
						std::replace(group.begin(), group.end(), ' ', '_');
						if (group.empty()) group = "no_group";

						outputFile << variableId << " " << used << " " << descr << " " << type << " " << group << std::endl;
					}

					//3] print the values
					outputFile << "#data" << std::endl;

					for (const CaseId& caseId : caseIds)
					{
						for (const VariableId& variableId : variableIds)
						{
							outputFile << dataSetState.getData(caseId, variableId) << " ";
						}
						outputFile << std::endl;
					}
				}
				// file will be closed 1st when leaving scope (regardless of exception)
				// mutex will be unlocked 2nd (from lock destructor) when leaving
				// scope (regardless of exception)
			}

			void loadFromFileBackend(
				const std::string& filename,
				DataSetState<D>& dataSetState) const
			{
				// mutex to protect file access
				//static std::mutex mutex;

				// lock mutex before accessing file
				//std::lock_guard<std::mutex> lock(mutex);

				std::string line;
				std::ifstream inputFileStream(filename);

				if (!inputFileStream.is_open())
				{
					std::cerr << "DataSetBackendTxt::loadFromFileBackend(): Unable to open file " << filename << std::endl;
					//throw tools::FileNotFoundException("no such file", filename);
					throw std::runtime_error("no such file");
				}
				else
				{
					std::cout << "DataSetBackendTxt::loadFromFileBackend(): Opening file " << filename << std::endl;

					//1] load the number of cases in this file
					if (!::tools::file::loadNextLine(inputFileStream, line))
					{
						std::cerr << "DataSetBackendTxt::loadFromFileBackend(): first line " << line << " has incorrect content" << std::endl;
					}
					//std::cout << "loadFromFile() first line = " << line << std::endl;
					const std::vector<std::string> content1 = ::tools::file::split(line, ' ');
					const unsigned int nCases = static_cast<unsigned int>(::tools::file::string2int(content1[0]));
					const unsigned int nVariables = static_cast<unsigned int>(::tools::file::string2int(content1[1]));
					const D missingValue = static_cast<D>(::tools::file::string2int(content1[2])); //TODO this only works for D==int

					//2] init this dataset
					Options<D> options;
					options.setMissingValue(missingValue);
					dataSetState.setOptions(options);

					//3] load the property descriptions
					for (unsigned int i = 0; i < nVariables; i++)
					{
						::tools::file::loadNextLine(inputFileStream, line);
						const std::vector<std::string> content2 = ::tools::file::split(line, ' ');
						const VariableId variableId = static_cast<VariableId>(::tools::file::string2int(content2[0]));
						dataSetState.setVariableValue(variableId, Variable::PropertyName::Used, content2[1]);
						dataSetState.setVariableValue(variableId, Variable::PropertyName::Description, content2[2]);
						dataSetState.setVariableValue(variableId, Variable::PropertyName::VariableType, content2[3]);
						dataSetState.setVariableValue(variableId, Variable::PropertyName::VariableGroup, content2[4]);
					}

					//4] handle the case data
					for (CaseIdType caseId = 0; caseId < nCases; caseId++)
					{
						::tools::file::loadNextLine(inputFileStream, line);
						const std::vector<std::string> content3 = ::tools::file::split(line, ' ');
						if (content3.size() == nVariables)
						{
							for (VariableIdType variableId = 0; variableId < nVariables; variableId++)
							{
								const D data = static_cast<D>(::tools::file::string2int(content3[variableId]));
								//const D data = content[propertyId];
								dataSetState.setData(CaseId(caseId), VariableId(variableId), data);
							}
						}
						else
						{
							std::cerr << "DataSetBackendTxt::loadFromFileBackend(): ERROR: " << nVariables << " variables while line for case " << caseId << " has " << content3.size() << " elements." << std::endl;
						}
					}
				}
				// file will be closed 1st when leaving scope (regardless of exception)
				// mutex will be unlocked 2nd (from lock destructor) when leaving
				// scope (regardless of exception)
			}

		protected:

			virtual ~DataSetBackendTxt() = default;

		private:

		};
	}
}