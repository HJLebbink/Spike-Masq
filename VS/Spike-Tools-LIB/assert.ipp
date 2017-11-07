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
#include <iostream>     // std::cout
#include <sstream>      // std::stringstream

namespace tools
{
	namespace assert
	{
		namespace priv
		{
			void addToStream(std::ostringstream&)
			{
				//intentionally empty
			}

			template<typename T, typename... Args>
			void addToStream(std::ostringstream& a_stream, T&& a_value, Args&&... a_args)
			{
				a_stream << std::forward<T>(a_value);
				addToStream(a_stream, std::forward<Args>(a_args)...);
			}
			inline void assert_msg(const std::string& message)
			{
				#if _DEBUG
				std::cerr << message;
				//std::cerr << std::flush; // no need to flush cerr
				if (true)
				{
					__debugbreak(); // generate int 3
				}
				else
				{
					char dummy;
					std::cin.get(dummy);
					//std::abort();
				}
				#endif
			}
		}

		inline void assert_msg(const bool cond)
		{
			#if _DEBUG
			if (!cond)
			{
				priv::assert_msg("ASSERT:");
			}
			#endif
		}

		template<typename... Args>
		inline void assert_msg(const bool cond, Args&&... a_args)
		{
			#if _DEBUG
			if (!cond)
			{
				std::ostringstream s;
				s << "----------------------------------" << std::endl << "ASSERT:";
				priv::addToStream(s, std::forward<Args>(a_args)...);
				priv::assert_msg(s.str());
			}
			#endif
		}
	}
}