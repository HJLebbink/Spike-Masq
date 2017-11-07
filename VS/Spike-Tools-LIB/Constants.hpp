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

namespace spike {

#if defined(_MSC_VER)
	const std::string tempDir = "C:/Temp/Spike";
	const std::string sourceDir = "C:/Data/spike data/";
#else
	const std::string tempDir = "/home/henk/Temp";
	const std::string sourceDir = "/home/henk/Data/spike data";
#endif

	namespace v1 {

		namespace constant {

			const	float	firingThreshold = 30.0f;
			const	float	maxEfficacy = 10.0f;			// maximal weight value (valence)
			const	int		maxPreCounter = 1000 * 2;		// maximal number of incommingNeurons; need not be larger than the number of neurons.

			const	int		THREAD_POOL_SIZE = 4;

			// polychronous
			const int polylenmax = 1000;		// maximal number of Ms in poly group 
			const int group_width = 3;			// start width of polychronous groups
			const int min_group_path = 7;		// minimal length of a group (in layers)
			const int min_group_time = 30;	//40	// minimal duration of a group (ms)
			const float minEfficacyGroup = 0.85f*maxEfficacy;	// minimal weight value for inclusion in group
			const int latency = 20; // D;				// maximum latency 
		}
	}
}
