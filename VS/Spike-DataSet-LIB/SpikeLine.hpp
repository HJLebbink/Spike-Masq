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

#include <vector>

namespace spike
{
	namespace dataset
	{

		// spikes of different neurons that occur in the same ms, used by SpikeSetLite
		class SpikeLine
		{
		public:
			// destructor
			~SpikeLine() = default;

			// default constructor
			SpikeLine() = delete;

			// constructor
			SpikeLine(const unsigned int nNeurons)
				: nNeurons_(nNeurons)
				, nChars_((nNeurons >> 3) + 1)
			{
				//std::cout << "SpikeLine::ctor" << std::endl;
				this->data_ = std::vector<char>(this->nChars_, 0);
			}

			// copy constructor
			SpikeLine(const SpikeLine& other) // copy constructor
				: nNeurons_(other.nNeurons_)
				, nChars_(other.nChars_)
				, data_(other.data_)
			{
				std::cout << "SpikeLine::copy ctor" << std::endl;
			}

			// move constructor
			SpikeLine(SpikeLine&& other) // move constructor
				: nNeurons_(other.nNeurons_)
				, nChars_(other.nChars_)
				, data_(std::move(other.data_))
			{
				std::cout << "SpikeLine::move ctor" << std::endl;
			}

			// copy assignment
			SpikeLine& operator=(const SpikeLine& other) // copy assignment operator
			{
				std::cout << "SpikeLine::copy assignment operator" << std::endl;
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

			// move assignment
			SpikeLine& operator=(SpikeLine&& other)
			{		// move assignment operator
				std::cout << "SpikeLine::move assignment operator" << std::endl;
				assert(this != &other);
				this->data_ = std::move(other.data_);
				return *this;	// by convention, always return *this
			}

			void clear()
			{
				std::fill(this->data_.begin(), this->data_.end(), static_cast<char>(0));
			}

			unsigned int getNumberOfNeurons() const
			{
				return this->nNeurons_;
			}

			unsigned int getNumberOfSpikes() const
			{
				// could have used a bitcount for speed...
				unsigned int count = 0;
				for (unsigned int i = 0; i < this->getNumberOfNeurons(); i++)
				{
					if (this->getValue(i))
					{
						count++;
					}
				}
				return count;
			}

			void setValue(unsigned int pos, bool value)
			{
				const unsigned int index1 = pos >> 3;
				const unsigned int index2 = pos & 7;
				const char currentValue = this->data_[index1];
				this->data_[index1] = (value) ? (currentValue | (1 << index2)) : (currentValue & ~(1 << index2));
				//std::cout << "SpikeLine::setValue(): pos="<<pos <<"; oldValue=" << (int)currentValue << "; value=" << value << "; newValue=" << (int)this->data[index1] << std::endl;
			}

			bool getValue(unsigned int pos) const
			{
				const char dataChar = this->data_[pos >> 3];
				if (dataChar == 0)
				{
					return false;
				}
				else
				{
					//	std::cout << "SpikeLine::getValue() char=" << (int)dataChar << std::endl;
					const unsigned int index2 = pos & 7;
					if ((dataChar & (1 << index2)) == 0)
					{
						return false;
					}
					else
					{
						//		std::cout << "SpikeLine::getValue()X char=" << (int)dataChar << std::endl;
						return true;
					}
				}
			}

			// return a vector of neuronIds that have a spike
			const std::vector<NeuronId> getSpikePositions() const
			{
				const unsigned int nNeurons = this->getNumberOfNeurons();
				std::vector<NeuronId> spikePositions;
				for (unsigned int i = 0; i < nNeurons; ++i)
				{
					if (this->getValue(static_cast<NeuronId>(i)))
					{
						spikePositions.push_back(i);
					}
				}
				return spikePositions;
			}

			std::string toString() const
			{
				std::ostringstream oss;
				for (unsigned int i = 0; i < this->getNumberOfNeurons(); ++i)
				{
					if ((i % 8 == 0) && (i > 0)) oss << " ";
					oss << ((this->getValue(i)) ? "o" : "-");
				}
				return oss.str();
			}

		private:

			const unsigned int nNeurons_;
			const unsigned int nChars_;
			std::vector<char> data_;
		};
	}
}