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

#include <sstream>	// for std::ostringstream
#include <limits>	// for std::numeric_limits
#include <string>
#include <cstdint>	// for std::uint8_t

#include "../Spike-DataSet-LIB/DataSetTypes.hpp"

namespace spike
{
	using namespace spike::dataset;

	///////////////////////////////////////////
	// spike types
	using MyDataType = int; // warning: MyDataType is use throughout all projects...
	using Ms = short; // millisecond from 0 to 1000

	// time in ms: max time is approx 49.7 days
	using TimeInMsI = unsigned int;
	using NeuronId = unsigned int;
	using Delay = short; // delay cannot be unsigned: see updateState_serial
	enum class FiringReason : std::uint8_t
	{
		NO_FIRE = 0, FIRE_PROPAGATED = 1, FIRE_RANDOM = 2, FIRE_CLAMPED = 3, FIRE_PROPAGATED_INCORRECT = 4, FIRE_PROPAGATED_CORRECT = 5
	};

	///////////////////////////////////////////
	// spike constants
	static const NeuronId NO_NEURON_ID = std::numeric_limits<NeuronId>::max();
	static const Delay NO_DELAY = static_cast<Delay>(std::numeric_limits<Delay>::max());

	///////////////////////////////////////////
	// spike structs

	struct Firing
	{
		Ms time;
		NeuronId neuronId;

		// default constructor
		Firing() = default;

		// constructor
		Firing(Ms t, NeuronId n)
			: time(t)
			, neuronId(n)
		{
		}

		bool operator< (const Firing& firing) const
		{
			if (this->time == firing.time)
			{
				return (this->neuronId < firing.neuronId);
			}
			else
			{
				return (this->time < firing.time);
			}
		}

		bool operator== (const Firing& rhs) const
		{
			return (this->time == rhs.time) && (this->neuronId == rhs.neuronId);
		}

		bool operator!= (const Firing& rhs) const
		{
			return (this->time != rhs.time) || (this->neuronId != rhs.neuronId);
		}

		std::string toString() const
		{
			std::ostringstream oss;
			oss << "Firing(neuronId " << this->neuronId << "; time " << this->time << ")";
			return oss.str();
		}
	};

	template <typename Time = Ms>
	struct CaseOccurance
	{
		CaseIdType caseId_;
		Time startTime_;
		Time endTime_;
		CaseLabelType caseLabel_;

		// default constructor
		CaseOccurance()
			: caseId_(NO_CASE_ID)
			, startTime_(0)
			, endTime_(0)
			, caseLabel_(NO_CASE_LABEL)
		{
		}

		// constructor
		CaseOccurance(CaseId caseId, Time startTime, Time endTime, CaseLabel caseLabel)
			: caseId_(caseId.val)
			, startTime_(startTime)
			, endTime_(endTime)
			, caseLabel_(caseLabel.val)
		{
		}

		std::string toString() const
		{
			std::ostringstream oss;
			oss << "CaseOccurance(caseId " << this->caseId_ << "(" << this->caseLabel_ << "); startTime " << this->startTime_ << "; endTime " << this->endTime_ << ")";
			return oss.str();
		}
	};


	// a neuron in a stream is either clamped to a certain value, or the neuron fires randomly (given a certain Hz)
	enum class NeuronDynamicType : std::int8_t
	{
		RANDOM = 0, CLAMPED = 1
	};
}