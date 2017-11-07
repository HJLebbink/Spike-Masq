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

#ifdef _MSC_VER
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>	// CreateDirectory
#else
#endif

#include <string>
#include <vector>
#include <sstream>      // std::stringstream

namespace tools
{
	namespace file
	{
		inline void split_private(
			const std::string& s,
			const char delim,
			std::vector<std::string>& elems)
		{
			std::stringstream ss(s);
			std::string item;
			while (std::getline(ss, item, delim))
			{
				elems.push_back(item);
			}
		}

		inline std::vector<std::string> split(
			const std::string& s,
			const char delim)
		{
			std::vector<std::string> elems;
			split_private(s, delim, elems);
			return elems;
		}

		inline float string2float(const std::string& s)
		{
			return (float)atof((char*)s.c_str());
		}

		inline int string2int(const std::string& s)
		{
			return (int)atof((char*)s.c_str());
		}

		inline bool loadNextLine(
			std::ifstream& stream,
			std::string& line)
		{
			const char remarkChar = '#';

			for (;;)
			{
				if (stream.good())
				{
					std::getline(stream, line);
					if (!line.empty())
					{
						const char firstChar = line.at(0);
						//std::cout << "loadNextLine() line = " << line << "; size=" << line.size() << "; firstChar=" << firstChar << "; remarkChar=" << remarkChar << std::endl;
						if (firstChar != remarkChar)
						{
							return true;
						}
					}
				}
				else
				{
					line = "";
					return false;
				}
			}
			#ifdef _MSC_VER
			#else
			return false;
			#endif
		}

		inline std::vector<std::string> loadNextLineAndSplit(
			std::ifstream& stream,
			const char delim)
		{
			std::string line;
			if (loadNextLine(stream, line)) {}
			return split(line, delim);
		}

		inline void stringToWString(
			std::wstring &ws,
			const std::string &s)
		{
			const std::wstring wsTmp(s.begin(), s.end());
			ws = wsTmp;
		}

		// make a nested directory structure: eg: mkdirTree("C:/Temp/Spike/A//B/C/");
		inline bool mkdirTree(
			const std::string& tree)
		{
			#ifdef __MIC__
			return true;
			#else
			#ifdef _MSC_VER

			std::string fullDirectory = "";
			std::wstring ws;
			for (const std::string& dir : split(tree, '/'))
			{
				if (dir.length() > 0)
				{
					fullDirectory += dir + "/";
					stringToWString(ws, fullDirectory);
					//std::cout << "mkdirTree() dir=" << dir << "; full directory=" << fullDirectory << std::endl;
					if (!CreateDirectory(ws.c_str(), NULL))
					{
						switch (GetLastError())
						{
							case ERROR_ALREADY_EXISTS:
								//std::cout << "mkdirTree(): ERROR_ALREADY_EXISTS" << std::endl;
								// no problem: The specified directory already exists.
								break;
							case ERROR_PATH_NOT_FOUND:
								printf("mkdirTree(): ERROR: ERROR_PATH_NOT_FOUND\n");
								// One or more intermediate directories do not exist; this function will only create the final directory in the path.
								return false;
							case ERROR_ACCESS_DENIED:
								//std::cout << "mkdirTree(): ERROR_ACCESS_DENIED" << std::endl;
								// this happends when you are not allowed to create the directory, but try to continue.
								break;
							default:
								printf("mkdirTree() : ERROR:other = %u\n", static_cast<unsigned int>(GetLastError()));
								return false;
						}
					}
				}
			}
			return true;
			#else
			return true;
			#endif
			#endif
		}

		inline std::string getDirectory(
			const std::string& filename)
		{
			int position = static_cast<int>(filename.size()) - 1;
			while (position >= 0)
			{
				const char c = filename.at(static_cast<std::string::size_type>(position));
				if ((c == '/') || (c == '\\'))
				{
					return filename.substr(0, static_cast<std::string::size_type>(position));
				}
				position--;
			}
			return "";
		}

		// better use an intrinsic for endianness conversion byteswap_ulong(i);
		inline unsigned int reverseInt(unsigned int i)
		{
			const unsigned char c1 = static_cast<unsigned char>(i & 255);
			const unsigned char c2 = static_cast<unsigned char>((i >> 8) & 255);
			const unsigned char c3 = static_cast<unsigned char>((i >> 16) & 255);
			const unsigned char c4 = static_cast<unsigned char>((i >> 24) & 255);
			return
				(static_cast<unsigned int>(c1) << 24) +
				(static_cast<unsigned int>(c2) << 16) +
				(static_cast<unsigned int>(c3) << 8) +
				static_cast<unsigned int>(c4);
		}

	}
}