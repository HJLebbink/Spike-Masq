#pragma once

#include <vector>
#include <typeinfo> //typeid(a)
#include <sstream>	// for std::ostringstream


#include "../../Tools/ToolsLib2/FixedPoint.hpp"

template <typename T>
struct WeightClass
{};

///////////////////////////////////////////////////
// partial specialization for float
///////////////////////////////////////////////////
template <>
struct WeightClass < float >
{
	using type = float;
	using ThisClass = WeightClass < type > ;
	type val;

	// constructor
	WeightClass()
	{}

	// constructor
	WeightClass(const type v)
	{
		this->val = v;
	}

	//compound assignment addition
	ThisClass& operator+=(const ThisClass& rhs)
	{
		this->val += rhs.val;
		return *this; // return the result by reference
	}

	std::string toString() const
	{
		std::ostringstream oss;
		oss << this->val;
		return oss.str();
	}

};

///////////////////////////////////////////////////
// partial specialization for FixedPoint<DIM, T>
///////////////////////////////////////////////////
template <unsigned int DIM, typename T>
struct WeightClass < FixedPoint<DIM, T> >
{
	using type = FixedPoint < DIM, T > ;
	using ThisClass = WeightClass < type >;
	type val;

	// constructor
	WeightClass()
	{}

	// constructor
	WeightClass(const float v)
	{
		this->val = v;
	}

	ThisClass& operator+=(const type& rhs)
	{
		this->val += rhs;
		return *this; // return the result by reference
	}

	ThisClass& operator+=(const ThisClass& rhs)
	{
		this->val += rhs.val;
		return *this; // return the result by reference
	}

	std::string toString() const
	{
		return this->val.toString();
	}

};