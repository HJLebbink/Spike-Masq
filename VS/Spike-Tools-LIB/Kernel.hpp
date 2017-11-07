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

//#include <string>
//#include <vector>
#include <cmath> //for expf


namespace spike
{
	namespace tools
	{
		// if t_j <= t_i then LTP; t_j > t_i then LTD
		inline float stdp(const float t_j_inMs, const float t_i_inMs, const float alpha_plus, const float alpha_minus, const float tau_plus, const float tau_minus)
		{
			if (t_j_inMs == t_i_inMs)
			{
				return 0;
			}
			else if (t_j_inMs <= t_i_inMs)
			{
				const float value = alpha_plus * expf((t_j_inMs - t_i_inMs) / tau_plus);
				//log.info(String.format("n %d: stdp(t_j=%f, t_i=%f): LTP: will return %.6f.", this.id, t_j_inMs, t_i_inMs, value));
				return value;
			}
			else
			{
				const float value = -alpha_minus * expf(-(t_j_inMs - t_i_inMs) / tau_minus);
				//log.info(String.format("n %d: stdp(t_j=%f, t_i=%f): LTD: will return %.6f", this.id, t_j_inMs, t_i_inMs, value));
				return value;
			}
		}
	}
}