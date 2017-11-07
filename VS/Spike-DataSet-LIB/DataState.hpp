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
#include <vector>
#include <set>
#include <map>
#include <memory>
#include <iomanip> // for setfill and setw
#include <set>
#include <iostream> // for cerr and cout
#include <sstream>	// std::ostringstream

#include "DataSetTypes.hpp"
#include "OptionsState.hpp"

namespace spike
{
	namespace dataset
	{
		template <class D>
		class DataState
			: public OptionsState<D>
		{
		public:

			DataState()
				: nCases_(0)
			{
			}

			unsigned int getNumberOfCases() const
			{
				return this->nCases_;
			}

			void setData(const CaseId caseId, const VariableId variableId, const D value)
			{
				if (this->nCases_ < (static_cast<unsigned int>(caseId.val) + 1))
				{
					this->nCases_ = (caseId.val + 1);
				}
				if (!this->hasVariableId(variableId))
				{
					this->createVariableId(variableId);
				}
				//	std::cout << "DataState::setData(): caseId=" << caseId << "; variableId=" << variableId << "; value=" << value << std::endl;
				const std::shared_ptr<std::vector<D>> values = this->data_.at(variableId);
				if (values->size() <= caseId.val)
				{
					//std::cout << "Resizing!" << std::endl;
					const D missingValue = this->getOptions().getMissingValue();
					values->resize(caseId.val + 100, missingValue);
				}
				(*values)[caseId.val] = value;
			}

			D getData(const CaseId caseId, const VariableId variableId) const
			{
				if (this->hasVariableId(variableId))
				{
					//std::cout << "DataState::getData(): nCases=" << this->_nCases << "; caseId=" << caseId << std::endl;
					return (*this->data_.at(variableId))[caseId.val];
				}
				else
				{
					std::cerr << "DataState::getData(): ERROR unknown variableId " << variableId << std::endl;
					throw std::runtime_error("unknown variableId");
				}
			}

			const std::shared_ptr<const std::vector<D>> getDataVector(const VariableId variableId) const
			{
				//if (this->hasVariableId(variableId)) {
				const auto vector = this->data_.at(variableId.val);
				//std::cout << "DataState::getDataVector() nCases=" << nCases << "; " << vector->size() << std::endl;
				return vector;
				//}
				//else {
				//	std::cerr << "DataState::getData(): ERROR unknown variableId " << variableId << std::endl;
				//	throw std::runtime_error("unknown variableId");
				//}
			}

			const std::set<D> getUniqueValues(const VariableId variableId) const
			{
				const D missingValue = this->getOptions().getMissingValue();

				std::set<D> uniqueValues;
				for (const D &value : (*this->data_.at(variableId).get()))
				{
					if (value != missingValue)
					{
						uniqueValues.insert(value);
					}
				}
				return uniqueValues;
			}

			std::string toString() const
			{
				std::ostringstream oss;

				const std::set<VariableId> variableIds = this->getVariableIds();

				oss << "----------------";
				for (size_t i = 0; i < variableIds.size(); ++i)
				{
					oss << "--------";
				}

				oss << std::endl << "variableId:\t";
				for (const VariableId& variableId : variableIds)
				{
					oss << variableId << "\t";
				}
				oss << std::endl;

				const unsigned int nCasesToPrint = std::min(static_cast<unsigned int>(10), this->getNumberOfCases());
				unsigned int counter = 0;
				const std::set<CaseId> caseIds = this->getCaseIds();
				for (const CaseId& caseId : caseIds)
				{
					oss << "case " << std::setfill(' ') << std::setw(5) << caseId << ":\t";
					for (const VariableId& variableId : variableIds)
					{
						const auto dataVariable = this->data_.at(variableId);
						oss << dataVariable->at(caseId) << "\t";
					}
					oss << std::endl;

					counter++;
					if (counter >= nCasesToPrint)
					{
						oss << "Printed " << nCasesToPrint << " of " << caseIds.size() << " total cases " << std::endl;
						break;
					}
				}
				return oss.str();
			}

			bool hasVariableId(const VariableId variableId) const
			{
				return (this->data_.find(variableId) != this->data_.cend());
			}

			const std::set<VariableId> getVariableIds() const
			{
				// consider caching the returned set
				std::set<VariableId> s;
				for (typename std::map<VariableId, std::shared_ptr<std::vector<D>>>::const_iterator it = this->data_.begin(); it != this->data_.end(); ++it)
				{
					s.insert(it->first);
				}
				return s;
			}

			const std::vector<CaseId> getCaseIds() const
			{
				// consider caching the returned set
				std::vector<CaseId> s(this->nCases_);
				for (CaseIdType caseId = 0; caseId < this->nCases_; caseId++)
				{
					s.push_back(CaseId(caseId));
				}
				return s;
			}

			// one in noiceChance is the chance of a snip being something other than the original value
			// does not change the missing values
			void addNoise(unsigned int noiseChance)
			{

				if (noiseChance > RAND_MAX)
				{
					oss << "DataState:addNoise: noise chance " << noiseChance << " is too big." << std::endl;
					//DEBUG_BREAK();
				}

				std::vector<CaseId> caseIds = this->getCaseIds();

				for (const VariableId variableId : this->getVariableIds())
				{
					const std::set<D> possibleValues = this->getUniqueValues<D>(variableId);
					const std::vector<D> valuesVector = new std::vector<D>(possibleValues);
					const unsigned int nValues = valuesVector.size();


					for (const CaseId caseId : caseIds)
					{
						if (tools::getRandomInt_excl(noiseChance) == 0)
						{

							D originalValue = (*this->data_.at(variableId))[caseId.val];

							bool updated = false;
							while (!updated)
							{
								const unsigned int randomIndex = tools::getRandomInt_excl(nValues);
								const D randValue = valuesVector[randomIndex];
								if (randValue != originalValue)
								{
									oss << "DataState:addNoise: variable=" << variableId.val << "; case=" << caseId.val << "; old value=" << originalValue << "; rand value=" << randValue << std::endl;
									(*this->data_.at(variableId))[caseId.val] = randValue;
									updated = true;
								}
							}
						}
					}
				}
			}

		protected:

			virtual ~DataState() = default;

		private:

			unsigned int nCases_;
			std::map<VariableId, std::shared_ptr<std::vector<D>>> data_;

			void createVariableId(const VariableId variableId)
			{
				//	std::cout << "DataState::createVariableId() variableId=" << variableId << std::endl;
				const D missingValue = this->getOptions().getMissingValue();
				auto emptyDataLineTmp = std::make_shared<std::vector<D>>(10, missingValue); // init with 10 positions filled with missing values
				this->data_.insert(std::make_pair(variableId, std::move(emptyDataLineTmp)));
			}
		};
	}
}