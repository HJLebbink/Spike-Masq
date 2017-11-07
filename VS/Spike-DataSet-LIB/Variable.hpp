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

#include <map>
#include <string>
#include <sstream>	// for std::ostringstream
#include <cassert>

namespace spike
{
	namespace dataset
	{
		class Variable
		{
		public:

			enum class PropertyName { Description, Used, VariableType, VariableGroup, UNKNOWN };
			enum class VariableTypeEnum { input, output, nuisance, UNKNOWN };

			static const std::string variableTypeEnumToString(const Variable::VariableTypeEnum variableTypeEnum)
			{
				switch (variableTypeEnum)
				{
				case Variable::VariableTypeEnum::input: return "INPUT";
				case Variable::VariableTypeEnum::output: return "OUTPUT";
				case Variable::VariableTypeEnum::nuisance: return "NUISANCE";
				case Variable::VariableTypeEnum::UNKNOWN: return "UNKNOWN";
				default: return "UNKNOWN";
				}
			}

			static Variable::VariableTypeEnum stringToVariableTypeEnum(std::string str)
			{
				for (auto & c : str) c = toupper(c);
				if (str == "INPUT") return Variable::VariableTypeEnum::input;
				if (str == "OUTPUT") return Variable::VariableTypeEnum::output;
				if (str == "NUISANCE") return Variable::VariableTypeEnum::nuisance;
				return Variable::VariableTypeEnum::UNKNOWN;
			}

			~Variable() = default;				// destructor
			Variable() = default;				// simple constructor

			// copy constructor
			Variable(const Variable& other)
				: data_(other.data_)
			{
				// this code is equal to "Variable(const Variable& other) = default"
				//std::cout << "Variable::copy ctor" << std::endl;
			}

			// move constructor
			Variable(Variable&& other)  // Variable&& is an rvalue reference to a Variable
				: data_(std::move(other.data_))
			{
				//std::cout << "Variable::move ctor" << std::endl;
			}

			// copy assignment operator 
			Variable& operator=(const Variable& other)
			{
				//std::cout << "Variable::copy assignment" << std::endl;
				if (this != &other) // protect against invalid self-assignment
				{
					this->data_ = other.data_;
				}
				else
				{
					std::cout << "Variable::operator=: prevented self-assignment" << std::endl;
				}
				return *this;	// by convention, always return *this
			}

			// move assignment operator 
			Variable& operator=(Variable&& other)
			{
				//std::cout << "Variable::move assignment operator" << std::endl;
				assert(this != &other);

				this->data_ = std::move(other.data_);
				return *this;	// by convention, always return *this
			}

			void setVariableValue(const Variable::PropertyName propertyName, const std::string& value)
			{
				this->data_[propertyName] = value;
				//	this->data.insert(std::make_pair(propertyName, value)); insert does not replace old content...

#if _DEBUG
				const std::string storedValue = this->getVariableValue(propertyName);
				if (storedValue != value)
				{
					std::cout << "Variable::setVariableValue() I tried to store value " << value << "; but retrieved value " << storedValue << std::endl;
					throw std::runtime_error("tried to store a value but could not retrieved its value");
				}
#endif
			}

			std::string getVariableValue(const Variable::PropertyName propertyName) const
			{
				const std::map<Variable::PropertyName, std::string>::const_iterator it = this->data_.find(propertyName);
				return (it == this->data_.end()) ? "" : it->second;
			}

			bool hasVariableValue(const Variable::PropertyName propertyName) const
			{
				const std::map<Variable::PropertyName, std::string>::const_iterator it = this->data_.find(propertyName);
				return (it != this->data_.end());
			}

			std::string toString() const
			{
				std::ostringstream oss;
				for (std::map<Variable::PropertyName, std::string>::const_iterator it = this->data_.begin(); it != this->data_.end(); ++it)
				{
					const int propertyNameInt = static_cast<int>(it->first);
					//TODO chance propertyNameInt to readable string
					oss << "PropertyName " << propertyNameInt << " -> " << it->second << std::endl;
				}
				return oss.str();
			}

		private:

			std::map<Variable::PropertyName, std::string> data_;

		};
	}
}