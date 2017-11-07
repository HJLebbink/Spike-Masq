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
#include <mutex>

namespace tools
{
	namespace log
	{
		namespace priv
		{
			std::mutex log_screen;

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
		}
		inline void log_ERROR(const std::string message, const bool pause = true)
		{
			priv::log_screen.lock();
			std::cout << "ERROR:" << message << std::endl;
			priv::log_screen.unlock();

			if (pause)
			{
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
			}
		}
		template<bool PAUSE = true, typename... Args >
		inline void log_ERROR(Args&&... a_args)
		{
			std::ostringstream s;
			priv::addToStream(s, std::forward<Args>(a_args)...);
			log_ERROR(s.str(), PAUSE);
		}
		inline void WARNING(const std::string message, const bool pause = false)
		{
			std::cout << "WARNING:" << message << std::flush;
			if (pause)
			{
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
			}
		}
		template<bool PAUSE = false, typename... Args>
		inline void log_WARNING(Args&&... a_args)
		{
			std::ostringstream s;
			priv::addToStream(s, std::forward<Args>(a_args)...);
			WARNING(s.str(), PAUSE);
		}
		inline void log_INFO(const std::string message, const bool pause = false)
		{
			priv::log_screen.lock();
			std::cout << "INFO:" << message;
			priv::log_screen.unlock();

			if (pause)
			{
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
			}
		}
		template<bool PAUSE = false, typename... Args>
		inline void log_INFO(Args&&... a_args)
		{
			std::ostringstream s;
			priv::addToStream(s, std::forward<Args>(a_args)...);
			log_INFO(s.str(), PAUSE);
		}
		inline void log_INFO_DEBUG(const std::string message, const bool pause = false)
		{
			#if _DEBUG
			priv::log_screen.lock();
			std::cout << "INFO:" << message << std::flush;
			priv::log_screen.unlock();
			if (pause)
			{
				char dummy;
				std::cin.get(dummy);
			}
			#endif
		}
		template<bool PAUSE = false, typename... Args >
		inline void log_INFO_DEBUG(Args&&... a_args)
		{
			#if _DEBUG
			std::ostringstream s;
			priv::addToStream(s, std::forward<Args>(a_args)...);
			log_INFO_DEBUG(s.str(), PAUSE);
			#endif
		}
	}
}