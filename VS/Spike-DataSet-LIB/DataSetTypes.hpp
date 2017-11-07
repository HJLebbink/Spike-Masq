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

#include <limits>		// for std::numeric_limits
#include <string>
#include <iostream>
#include <algorithm>    // std::max

namespace spike
{
	namespace dataset
	{
		///////////////////////////////////////////////
		// Bit
		///////////////////////////////////////////////
		using BitType = unsigned char;
		struct Bit
		{
			Bit(const size_t b): val(static_cast<BitType>(b)) {}
			BitType val;
		};

		///////////////////////////////////////////////
		// Case Label
		///////////////////////////////////////////////
		using CaseLabelType = unsigned short;
		struct CaseLabel
		{
			CaseLabel() = default;

			CaseLabel(const CaseLabel& other):
				val(other.val)
			{
			}

			CaseLabel(const CaseLabelType c): val(c)
			{
			}
			bool operator<(const CaseLabel& rhs) const
			{
				return rhs.val < this->val;
			}
			bool operator==(const CaseLabel& rhs) const
			{
				return rhs.val == this->val;
			}
			bool operator!=(const CaseLabel& rhs) const
			{
				return rhs.val != this->val;
			}
			CaseLabelType val;
		};
		inline std::ostream& operator<<(std::ostream& stream, const CaseLabel caseLabel)
		{
			stream << caseLabel.val;
			return stream;
		}
		const CaseLabel NO_CASE_LABEL = CaseLabel(std::numeric_limits<CaseLabelType>::max());

		///////////////////////////////////////////////
		// Case Id
		///////////////////////////////////////////////
		using CaseIdType = unsigned short;
		struct CaseId
		{

			// default constructor
			CaseId(): val(NO_CASE_LABEL.val) {}

			// constructor
			explicit CaseId(const CaseIdType c): val(c) {}

			bool operator<(const CaseId& rhs) const
			{
				return rhs.val < this->val;
			}
			bool operator==(const CaseId& rhs) const
			{
				return rhs.val == this->val;
			}
			bool operator!=(const CaseId& rhs) const
			{
				return rhs.val != this->val;
			}
			CaseIdType val;
		};
		inline std::ostream& operator<<(std::ostream& stream, const CaseId caseId)
		{
			stream << caseId.val;
			return stream;
		}
		const CaseId NO_CASE_ID = CaseId(std::numeric_limits<CaseIdType>::max());

		///////////////////////////////////////////////
		// Variable
		///////////////////////////////////////////////
		using VariableIdType = unsigned int;
		const VariableIdType NO_VARIABLE_ID_VALUE = std::numeric_limits<VariableIdType>::max();

		class VariableId
		{
		public: VariableIdType val;
				VariableId(): val(NO_VARIABLE_ID_VALUE)
				{
				}
				VariableId(const VariableIdType b): val(b)
				{
				}
				bool operator<(const VariableId& rhs) const
				{
					return rhs.val < this->val;
				}
				bool operator==(const VariableId& rhs) const
				{
					return rhs.val == this->val;
				}
		};
		inline std::ostream& operator<<(std::ostream& stream, const VariableId variableId)
		{
			stream << variableId.val;
			return stream;
		}
		const VariableId NO_VARIABLE_ID = VariableId(NO_VARIABLE_ID_VALUE);
	}
}