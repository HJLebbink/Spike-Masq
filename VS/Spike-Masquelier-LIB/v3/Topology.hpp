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
#include <vector>

#include "../../Spike-Tools-LIB/NeuronIdRange.hpp"

#include "Types.hpp"
#include "SpikeOptionsStatic.hpp"

namespace spike
{
	namespace v3
	{
		using namespace ::spike::tools;

		template <typename Options_i>
		class Topology
		{
		public:

			using Options = Options_i;

			static const size_t Ne = Options::Ne;
			static const size_t Ni = Options::Ni;
			static const size_t Ns = Options::Ns;
			static const size_t Nm = Options::Nm;
			static const size_t nNeurons = Ne + Ni + Ns + Nm;

			static const size_t Ne_start = 0;
			static const size_t Ne_end = Ne;
			static const size_t Ni_start = Ne_end;
			static const size_t Ni_end = Ne_end + Ni;
			static const size_t Nm_start = Ni_end;
			static const size_t Nm_end = Ni_end + Nm;
			static const size_t Ns_start = Nm_end;
			static const size_t Ns_end = Nm_end + Ns;

			// constructor
			Topology()
			{
			}

			std::vector<Pathway> getOutgoingPathways(const NeuronId origin) const
			{
				std::vector<Pathway> v;
				for (const Pathway& pathway : this->pathways_)
				{
					if (pathway.origin == origin)
					{
						v.push_back(pathway);
					}
				}
				//std::cout << "calcOutgoingPathways: neuronId " << origin << "; nOutgoing neurons " << v.size() << std::endl;
				return v;
			}

			std::vector<Pathway> getIncommingPathways(const NeuronId destination) const
			{
				std::vector<Pathway> v;
				for (const Pathway& pathway : this->pathways_)
				{
					if (pathway.destination == destination)
					{
						v.push_back(pathway);
					}
				}
				return v;
			}

			void clearPathways()
			{
				this->pathways_.clear();
			}

			void addPathway(
				const NeuronId origin,
				const NeuronId destination,
				const Delay delay,
				const Efficacy efficacy)
			{
				this->pathways_.push_back(Pathway(origin, destination, delay, efficacy));
			}

			void init_Masquelier()
			{
				printf("spike::v3::topology::init_Masquelier: similar to Masquelier setup with fully connected network\n");

				if (Options::Ne != 0) std::cerr << "spike::v3::Topology:init_Masquelier(): warning: original experiment had 0 excitatory neurons. currently " << Options::Ne << std::endl;
				if (Options::Ni != 3) std::cerr << "spike::v3::Topology:init_Masquelier(): warning: original experiment had 3 inhibitory neurons. currently " << Options::Ni << std::endl;
				if (Options::Ns != 2000) std::cerr << "spike::v3::Topology:init_Masquelier(): warning: original experiment had 2000 sensory neurons. currently " << Options::Ns << std::endl;
				if (Options::Nm != 0) std::cerr << "spike::v3::Topology:init_Masquelier(): warning: original experiment had 0 motor neurons. currently " << Options::Nm << std::endl;

				if (Options::nSynapses != 2000) std::cerr << "spike::v3::Topology:init_Masquelier(): warning: original experiment had 2000 synapses per (excitiatory) neurons. currently " << Options::nSynapses << std::endl;


				this->clearPathways(); // clear this topology, make it empty

				// 2] neurons from exc have a pathways to all inh neurons
				// 4] neurons from inh have a pathways to neurons from exc1 (and not inh);

				for (const NeuronId sensorNeuronId : Topology::iterator_SensorNeurons())
				{
					const Delay delay = 1;
					const Efficacy weight = 1; //TODO find correct initial weight
					for (const NeuronId inhNeuronId : Topology::iterator_InhNeurons())
					{
						this->addPathway(sensorNeuronId, inhNeuronId, delay, weight);
					}
				}

				for (const NeuronId inhNeuron1Id : Topology::iterator_InhNeurons())
				{
					const Delay delay = 1; // Original delay in Masquelier was 0ms
					const Efficacy weight = -1; //TODO find correct initial weight

					for (const NeuronId inhNeuron2Id : Topology::iterator_InhNeurons())
					{
						if (inhNeuron1Id != inhNeuron2Id)
						{
							this->addPathway(inhNeuron1Id, inhNeuron2Id, delay, weight);
						}
					}
				}
			}

			void init_Izhikevich()
			{
				if (Options::Ne != 800) std::cerr << "spike::v3::Topology:init_Izhikevich(): warning: original experiment had 800 excitatory neurons. currently " << Options::Ne << std::endl;
				if (Options::Ni != 200) std::cerr << "spike::v3::Topology:init_Izhikevich(): warning: original experiment had 200 inhibitory neurons. currently " << Options::Ni << std::endl;
				if (Options::nSynapses != 100) std::cerr << "spike::v3::Topology:init_Izhikevich(): warning: original experiment had 100 synapses per neurons. currently " << Options::nSynapses << std::endl;

				if (Options::nSynapses > Options::Ne)
				{
					std::cerr << "spike::v3::Topology:init_Izhikevich(): error: not enough neurons (" << Options::Ne << ") for inhibitory pathways (" << Options::nSynapses << ")" << std::endl; throw 1;
				}
				if (Options::nSynapses > (Options::Ne + Options::Ni))
				{
					std::cerr << "spike::v3::Topology:init_Izhikevich(): error: not enough neurons (" << (Options::Ne + Options::Ni) << ")  for excitatory pathways (" << Options::nSynapses << ")" << std::endl; throw 1;
				}


				this->clearPathways(); // clear this topology, make it empty

				// 2] neurons from exc1 have s pathways to neurons from exc1 or inh;
				// 4] neurons from inh have s pathways to neurons from exc1 (and not inh);

				std::vector<NeuronId> exc_neurons;
				std::vector<NeuronId> inh_neurons;
				std::vector<NeuronId> all_neurons;

				for (const NeuronId neuronId : Topology::iterator_ExcNeurons())
				{
					all_neurons.push_back(neuronId);
					exc_neurons.push_back(neuronId);
				}

				for (const NeuronId neuronId : Topology::iterator_InhNeurons())
				{
					all_neurons.push_back(neuronId);
					inh_neurons.push_back(neuronId);
				}

				std::vector<NeuronId> alreadyUseNeuronIds;

				for (const NeuronId origin : exc_neurons)
				{

					unsigned int s = 0;
					alreadyUseNeuronIds.clear();
					alreadyUseNeuronIds.push_back(origin);

					// 2] neurons from exc1 have M pathways to neurons from exc1 or inh;
					while (s < Options::nSynapses)
					{
						const NeuronId destination = this->getRandomNeuronId(all_neurons, alreadyUseNeuronIds);
						alreadyUseNeuronIds.push_back(destination);
						const Delay delay = ((s%Options::maxDelay) < Options::minDelay) ? Options::minDelay : (s%Options::maxDelay);
						const Efficacy weight = Options::initialWeightExc;
						this->addPathway(origin, destination, delay, weight);
						s++;
					}
				}

				for (const NeuronId origin : inh_neurons)
				{

					unsigned int s = 0;
					alreadyUseNeuronIds.clear();
					alreadyUseNeuronIds.push_back(origin);

					// 4] neurons from inh have M pathways to neurons from exc1 (and not inh);
					while (s < Options::nSynapses)
					{
						const NeuronId destination = getRandomNeuronId(exc_neurons, alreadyUseNeuronIds);
						alreadyUseNeuronIds.push_back(destination);
						const Delay delay = Options::minDelay;
						//const Delay delay = ((s%Options::maxDelay) < Options::minDelay) ? Options::minDelay : (s%Options::maxDelay);
						const Efficacy weight = Options::initialWeightInh;
						this->addPathway(origin, destination, delay, weight);
						s++;
					}
				}
			}
			void init_mnist()
			{
				if (Options::nSynapses > Ne)
				{
					std::cerr << "spike::v3::Topology:init_mnist: error: not enough neurons (" << Ne << ") for inhibitory pathways (" << Options::nSynapses << ")" << std::endl; throw 1;
				}
				if (Options::nSynapses > (Ne + Ni))
				{
					std::cerr << "spike::v3::Topology:init_mnist: error: not enough neurons (" << (Ne + Ni) << ")  for excitatory pathways (" << Options::nSynapses << ")" << std::endl; throw 1;
				}
				if (Ns != 28 * 28)
				{
					std::cerr << "spike::v3::Topology:init_mnist: error: incorrect number of sensory neurons (" << Ns << "), expecting " << 28 * 28 << std::endl; throw 1;
				}
				if (Nm != 10)
				{
					std::cerr << "spike::v3::Topology:init_mnist: error: incorrect number of motor neurons (" << Nm << "), expecting 10." << std::endl; throw 1;
				}

				const Delay minDelay = 1; // original Izhikevich experiment minDelay = 0;

				this->clearPathways(); // clear this topology, make it empty

				std::vector<NeuronId> exc_neurons; // excitatory
				std::vector<NeuronId> inh_neurons; // inhibitory
				std::vector<NeuronId> sensor_neurons; // input
				std::vector<NeuronId> motor_neurons; // output
				std::vector<NeuronId> exc_inh_motor_neurons; // output
				std::vector<NeuronId> exc_motor_neurons; // output

				for (const NeuronId& neuronId : Topology::iterator_ExcNeurons())
				{
					exc_inh_motor_neurons.push_back(neuronId);
					exc_neurons.push_back(neuronId);
					exc_motor_neurons.push_back(neuronId);
				}

				for (const NeuronId& neuronId : Topology::iterator_InhNeurons())
				{
					exc_inh_motor_neurons.push_back(neuronId);
					inh_neurons.push_back(neuronId);
				}

				for (const NeuronId& neuronId : Topology::iterator_MotorNeurons())
				{
					motor_neurons.push_back(neuronId);
					exc_inh_motor_neurons.push_back(neuronId);
					exc_motor_neurons.push_back(neuronId);
				}

				for (const NeuronId& neuronId : Topology::iterator_SensorNeurons())
				{
					sensor_neurons.push_back(neuronId);
				}

				std::vector<NeuronId> alreadyUseNeuronIds;

				for (const NeuronId& neuronId1 : exc_neurons)
				{
					alreadyUseNeuronIds.clear();
					alreadyUseNeuronIds.push_back(neuronId1);

					// 2] neurons from exc_neurons have M pathways to neurons from exc, inh or motor;
					for (size_t s = 0; s < Options::nSynapses; ++s)
					{
						const NeuronId neuronId2 = this->getRandomNeuronId(exc_inh_motor_neurons, alreadyUseNeuronIds);
						alreadyUseNeuronIds.push_back(neuronId2);
						const unsigned int maxDelay = static_cast<unsigned int>(Options::maxDelay);
						const Delay delay = static_cast<Delay>(::tools::random::rand_int32(minDelay, maxDelay+1));
						const float weight = Options::initialWeightExc;
						this->addPathway(neuronId1, neuronId2, delay, weight);
					}
				}

				for (const NeuronId& neuronId1 : inh_neurons)
				{
					alreadyUseNeuronIds.clear();

					size_t s = 0;

					// 4] neurons from inh_neurons have M pathways to neurons from exc1 (and not inh);
					while (s < Options::nSynapses)
					{
						const NeuronId neuronId2 = getRandomNeuronId(exc_motor_neurons, alreadyUseNeuronIds);
						alreadyUseNeuronIds.push_back(neuronId2);
						const Delay delay = minDelay;
						const float weight = Options::initialWeightInh;
						this->addPathway(neuronId1, neuronId2, delay, weight);
						s++;
					}
				}

				//for (const NeuronId neuronId1 : motor_neurons) {
				// motor neurons have no outgoing pathways
				//}

				for (const NeuronId& neuronId1 : sensor_neurons)
				{
					alreadyUseNeuronIds.clear();

					size_t s = 0;

					/*
					for (const NeuronId neuronId2 : motor_neurons) {
					alreadyUseNeuronIds.push_back(neuronId2);
					const unsigned int maxDelay = static_cast<unsigned int>(D);
					const Delay delay = static_cast<Delay>(tools::getRandomInt_excl(minDelay, maxDelay));
					const float weight = 5;
					this->addPathway(neuronId1, neuronId2, delay, weight);
					s++;
					}
					*/
					while (s < Options::nSynapses)
					{
						const NeuronId neuronId2 = getRandomNeuronId(exc_neurons, alreadyUseNeuronIds);

						alreadyUseNeuronIds.push_back(neuronId2);
						const unsigned int maxDelay = static_cast<unsigned int>(Options::maxDelay);
						const Delay delay = static_cast<Delay>(::tools::random::rand_int32(minDelay, maxDelay));
						const float weight = Options::initialWeightExc;
						this->addPathway(neuronId1, neuronId2, delay, weight);
						s++;
					}
				}
			}

			void init_mnist2()
			{
				if (Options::nSynapses > Options::Ne)
				{
					std::cerr << "Topology::load_mnist(): error: not enough neurons (" << Ne << ") for inhibitory pathways (" << Options::nSynapses << ")" << std::endl; throw 1;
				}
				if (Options::nSynapses > (Options::Ne + Options::Ni))
				{
					std::cerr << "Topology::load_mnist(): error: not enough neurons (" << (Ne + Ni) << ") for excitatory pathways (" << Options::nSynapses << ")" << std::endl; throw 1;
				}
				if (Options::Ne != 800 + Options::Ns)
				{
					std::cerr << "Topology::load_mnist(): error: incorrect number of exitatory neurons (" << Ne << "), expecting " << 800 + Ns << std::endl; throw 1;
				}
				if (Options::Ns != (28 * 28))
				{
					std::cerr << "Topology::load_mnist(): error: incorrect number of sensory neurons (" << Ns << "), expecting " << (28 * 28) << std::endl; throw 1;
				}
				if (Options::Nm != 10)
				{
					std::cerr << "Topology::load_mnist(): error: incorrect number of motor neurons (" << Nm << "), expecting 10." << std::endl; throw 1;
				}

				const Delay minDelay = 1; // original Izhikevich experiment minDelay = 0;
				const Delay maxDelay = Options::nSynapses;

				this->clearPathways(); // clear this topology, make it empty

				std::vector<NeuronId> exc_neurons; // excitatory
				std::vector<NeuronId> inh_neurons; // inhibitory
				std::vector<NeuronId> sensor_neurons; // input
				std::vector<NeuronId> motor_neurons; // output

				std::vector<NeuronId> exc1_neurons; // excitatory layer1
				std::vector<NeuronId> exc2_neurons; // excitatory layer2

				std::vector<NeuronId> exc_inh_neurons;


				unsigned int count = 0;
				for (const NeuronId& neuronId : Topology::iterator_ExcNeurons())
				{
					exc_neurons.push_back(neuronId);
					exc_inh_neurons.push_back(neuronId);
					if (count < (28 * 28))
					{
						exc1_neurons.push_back(neuronId);
					}
					else
					{
						exc2_neurons.push_back(neuronId);
					}
					count++;
				}

				BOOST_ASSERT_MSG_HJ(exc1_neurons.size() == 28 * 28, "incorrect size");

				for (const NeuronId& neuronId : Topology::iterator_InhNeurons())
				{
					inh_neurons.push_back(neuronId);
					exc_inh_neurons.push_back(neuronId);
				}
				for (const NeuronId& neuronId : Topology::iterator_MotorNeurons())
				{
					motor_neurons.push_back(neuronId);
				}
				for (const NeuronId& neuronId : Topology::iterator_SensorNeurons())
				{
					sensor_neurons.push_back(neuronId);
				}

				std::vector<NeuronId> alreadyUseNeuronIds;

				for (const NeuronId& destination : exc1_neurons)
				{
					const int x1 = (destination - Ne_start) / 28;
					const int y1 = (destination - Ne_start) % 28;
					//std::cout << "load_mnist2: destination " << destination << ":" << x1 << "," << y1 << std::endl;

					if (((x1 * 28) + y1) != (destination - Ne_start)) //DEBUG_BREAK();

					for (int x2 = (x1 - 5); x2 < (x1 + 5); ++x2)
					{
						for (int y2 = (y1 - 5); y2 < (y1 + 5); ++y2)
						{
							if ((x2 < 0) || (x2 >= 28) || (y2 < 0) || (y2 >= 28))
							{
								// do nothing
							}
							else
							{
								const NeuronId origin = static_cast<NeuronId>(Ns_start + (x2 * 28) + y2);
								const unsigned int maxDelay = static_cast<unsigned int>(Options::maxDelay);
								const Delay delay = static_cast<Delay>(tools::getRandomInt_excl(minDelay, maxDelay));
								const float weight = Options::initialWeightExc;
								//std::cout << "load_mnist2: sensor " << x2 << "," << y2 << " (" << origin << ") is maped to " << x1 << "," << y1 << " (" << destination << ")" << std::endl;
								this->addPathway(origin, destination, delay, weight);
							}
						}
					}
				}

				for (const NeuronId& destination : exc2_neurons)
				{
					alreadyUseNeuronIds.clear();
					alreadyUseNeuronIds.push_back(destination); // connot make pathway to itself

					// 2] neurons from exc_neurons have M pathways to neurons from exc2, inh or motor;
					for (size_t s = 0; s < Options::nSynapses; ++s)
					{
						const NeuronId origin = this->getRandomNeuronId(exc_inh_neurons, alreadyUseNeuronIds);
						alreadyUseNeuronIds.push_back(origin);
						const unsigned int maxDelay = static_cast<unsigned int>(Options::maxDelay);
						const Delay delay = static_cast<Delay>(tools::getRandomInt_excl(minDelay, maxDelay));
						const float weight = Options::initialWeightExc;
						this->addPathway(origin, destination, delay, weight);
					}
				}

				for (const NeuronId& destination : inh_neurons)
				{
					alreadyUseNeuronIds.clear();

					// 4] neurons from inh_neurons have M pathways to neurons from exc1 (and not inh);
					for (size_t s = 0; s < Options::nSynapses; ++s)
					{
						const NeuronId origin = this->getRandomNeuronId(exc2_neurons, alreadyUseNeuronIds);
						alreadyUseNeuronIds.push_back(origin);
						const Delay delay = minDelay;
						const float weight = Options::initialWeightInh;
						this->addPathway(origin, destination, delay, weight);
					}
				}

				for (const NeuronId destination : motor_neurons)
				{
					alreadyUseNeuronIds.clear();

					for (size_t s = 0; s < Options::nSynapses; ++s)
					{
						const NeuronId origin = this->getRandomNeuronId(exc2_neurons, alreadyUseNeuronIds);
						alreadyUseNeuronIds.push_back(origin);
						const Delay delay = static_cast<Delay>(tools::getRandomInt_excl(minDelay, maxDelay));
						const float weight = Options::initialWeightExc;
						this->addPathway(origin, destination, delay, weight);
					}
				}
			}


			static const NeuronIdRange<NeuronId, Ne_start, Ne_end> iterator_ExcNeurons()
			{
				return NeuronIdRange<NeuronId, Ne_start, Ne_end>();
			}
			static const NeuronIdRange<NeuronId, Ni_start, Ni_end> iterator_InhNeurons()
			{
				return NeuronIdRange<NeuronId, Ni_start, Ni_end>();
			}
			static const NeuronIdRange<NeuronId, Nm_start, Nm_end> iterator_MotorNeurons()
			{
				return NeuronIdRange<NeuronId, Nm_start, Nm_end>();
			}
			static const NeuronIdRange<NeuronId, Ns_start, Ns_end> iterator_SensorNeurons()
			{
				return NeuronIdRange<NeuronId, Ns_start, Ns_end>();
			}
			// active neurons are excitatory, sensor and motor neurons
			static const NeuronIdRange<NeuronId, Ne_start, Nm_end> iterator_ActiveNeurons()
			{
				return NeuronIdRange<NeuronId, Ne_start, Nm_end>();
			}
			static const NeuronIdRange<NeuronId, Ne_start, Ni_end> iterator_ExcInhNeurons()
			{
				return NeuronIdRange<NeuronId, Ne_start, Ni_end>();
			}
			static const NeuronIdRange<NeuronId, Ne_start, Ns_end> iterator_AllNeurons()
			{
				return NeuronIdRange<NeuronId, Ne_start, Ns_end>();
			}

			static bool isExcNeuron(const NeuronId neuronId)
			{
				BOOST_ASSERT_MSG_HJ(Ne_start == 0, "spike::v3::Topology:isExcNeuron: assumed Ne_start is zero");
				return (neuronId < Ne_end);
				//				return (neuronId >= Ne_start) && (neuronId < Ne_end);
			}
			static bool isInhNeuron(const NeuronId neuronId)
			{
				return (neuronId >= Ni_start) && (neuronId < Ni_end);
			}
			static bool isMotorNeuron(const NeuronId neuronId)
			{
				return (neuronId >= Nm_start) && (neuronId < Nm_end);
			}
			static bool isSensorNeuron(const NeuronId neuronId)
			{
				return (neuronId >= Ns_start) && (neuronId < Ns_end);
			}

			static NeuronId translateToSensorNeuronId(const NeuronId caseNeuronId)
			{
				const NeuronId sensorNeuronId = Ns_start + caseNeuronId;
				if (isSensorNeuron(sensorNeuronId))
				{
					return sensorNeuronId;
				}
				else
				{
					std::cout << "Topology::translateToSensorNeuronId: caseNeuronId " << caseNeuronId << " is larger than number of sensor neurons Ns=" << Ns << std::endl;
					//DEBUG_BREAK();
					return sensorNeuronId;
				}
			}

			static NeuronId translateToMotorNeuronId(const CaseLabel caseLabel)
			{
				const NeuronId motorNeuronId = Nm_start + static_cast<NeuronId>(caseLabel.val);
				if (isMotorNeuron(motorNeuronId))
				{
					return motorNeuronId;
				}
				else
				{
					std::cout << "Topology::translateToMotorNeuronId: caseLabel " << caseLabel.val << " is too large" << std::endl;
					//DEBUG_BREAK();
					return motorNeuronId;
				}
			}

			void loadFromFile(const std::string& filename)
			{
				// mutex to protect file access
				//static std::mutex mutex;

				// lock mutex before accessing file
				//std::lock_guard<std::mutex> lock(mutex);

				std::string line;
				std::ifstream inputFile(filename);
				if (!inputFile.is_open())
				{
					std::cerr << "spike::v3::Topology::loadFromFile(): Unable to open file " << filename << std::endl;
					throw std::runtime_error("Unable to open file");
				}
				else
				{
					//std::cout << "Topology::loadFromFile(): Opening file " << filename << std::endl;
					// the first line contains "# topology <nNeurons> <nPathways>"

					const std::vector<std::string> content = ::tools::loadNextLineAndSplit(inputFile, ' ');
					const unsigned int nNeurons = ::tools::file::string2int(content[0]);
					const unsigned int nPathways = ::tools::file::string2int(content[1]);

					for (unsigned int i = 0; i < nNeurons; ++i)
					{
						const std::vector<std::string> content = ::tools::loadNextLineAndSplit(inputFile, ' ');
						/*
						if (content.size() == 7) {
						const NeuronId neuronId = static_cast<NeuronId>(tools::file::string2int(content[0]));
						if (neuronId >= nNeurons) {
						std::cerr << "spike::v3::Topology::load(): line " << line << " has incorrect content: neuronId= " << neuronId << std::endl;
						throw std::exception();
						}

						// retrieve neuron type
						this->setParameterA(neuronId, ::tools::file::string2float(content[1]));
						this->setParameterB(neuronId, ::tools::file::string2float(content[2]));
						this->setParameterC(neuronId, ::tools::file::string2float(content[3]));
						this->setParameterD(neuronId, ::tools::file::string2float(content[4]));
						this->trainRate_[neuronId] = ::tools::file::string2float(content[5]);
						this->inputScaling_[neuronId] = ::tools::file::string2float(content[6]);
						} else {
						std::cerr << "spike::v3::Topology::loadFromFile(): line " << line << " has incorrect content" << std::endl;
						}
						*/
					}
					this->pathways_.clear();

					for (unsigned int i = 0; i < nPathways; ++i)
					{
						const std::vector<std::string> content = ::tools::loadNextLineAndSplit(inputFile, ' ');
						if (content.size() == 4)
						{
							const NeuronId neuronId1 = static_cast<NeuronId>(tools::file::string2int(content[0]));
							const NeuronId neuronId2 = static_cast<NeuronId>(tools::file::string2int(content[1]));
							const Delay delay = static_cast<Delay>(tools::file::string2int(content[2]));
							const Efficacy weigth = ::tools::file::string2float(content[3]);

							if ((neuronId1 < 0) || (neuronId1 >= nNeurons)) std::cerr << "spike::v3::Topology::load(): line " << line << " has incorrect content: neuronId1= " << neuronId1 << std::endl;
							if ((neuronId2 < 0) || (neuronId2 >= nNeurons)) std::cerr << "spike::v3::Topology::load(): line " << line << " has incorrect content: neuronId2= " << neuronId2 << std::endl;
							if ((delay < 0) || (delay >= Options::maxDelay)) std::cerr << "spike::v3::Topology::load(): ERROR A. line " << line << " has incorrect content: delay= " << delay << std::endl;

							this->addPathway(neuronId1, neuronId2, delay, weigth);
						}
						else
						{
							std::cerr << "spike::v3::Topology::loadFromFile(): ERROR B. line " << line << " has incorrect content" << std::endl;
						}
					}
				}
			}

			void saveToFile(const std::string& filename) const
			{
				// mutex to protect file access
				//static std::mutex mutex;

				// lock mutex before accessing file
				//std::lock_guard<std::mutex> lock(mutex);

				// create the directory
				const std::string tree = ::tools::file::getDirectory(filename);
				if (!::tools::file::mkdirTree(tree))
				{
					std::cerr << "spike::v3::Topology::saveToFile: Unable to create directory " << tree << std::endl;
					throw std::runtime_error("unable to create directory");
				}

				// try to open file
				std::ofstream outputFile(filename); //fstream is a proper RAII object, it does close automatically at the end of the scope
				if (!outputFile.is_open())
				{
					std::cerr << "spike::v3::Topology::saveToFile: Unable to open file " << filename << std::endl;
					throw std::runtime_error("Unable to open file");
				}
				else
				{
					//std::cout << "Topology::saveToFile(): Opening file " << filename << std::endl;
					const size_t nPathways = this->pathways_.size();

					outputFile << "# topology <nNeurons> <nPathways>" << std::endl;
					outputFile << Options::nNeurons << " " << nPathways << std::endl;

					outputFile << "# parameter <neuronId> <a> <b> <c> <d> <trainRate> <inputScaling>" << std::endl;
					for (NeuronId neuronId = 0; neuronId < Options::nNeurons; ++neuronId)
					{
						//const float a = this->getParameterA(neuronId);
						//const float b = this->getParameterB(neuronId);
						//const float c = this->getParameterC(neuronId);
						//const float d = this->getParameterD(neuronId);
						//const float tr = this->trainRate_[neuronId];
						//const float is = this->getInputScaling(neuronId);
						outputFile << neuronId << " " << std::endl;
					}

					outputFile << "# pathway <origin> <destination> <delay> <weight>" << std::endl;
					for (unsigned int i = 0; i < nPathways; ++i)
					{
						outputFile << this->pathways_[i].origin << " " << this->pathways_[i].destination << " " << this->pathways_[i].delay << " " << this->pathways_[i].efficacy << std::endl;
					}
				}
			}

		private:

			std::vector<Pathway> pathways_;

			NeuronId getRandomNeuronId(
				const std::vector<NeuronId>& allowedValues,
				const std::vector<NeuronId>& notAllowedValues)
			{
				unsigned int retryCounter = 0;
				NeuronId randomNeuronId = NO_NEURON;
				bool exists;
				const unsigned int largestIndex = static_cast<unsigned int>(allowedValues.size());
				do
				{
					randomNeuronId = allowedValues.at(::tools::random::rand_int32(0U, largestIndex));
					exists = false;
					for (std::vector<NeuronId>::size_type i = 0; i != notAllowedValues.size(); i++)
					{
						if (notAllowedValues.at(i) == randomNeuronId)
						{
							exists = true;
							retryCounter++;
							if (retryCounter == 100000)
							{
								std::cout << "Topology::getRandomNeuronId: could not find a random neuronId: I've tried " << retryCounter << " times, giving up." << std::endl;
								//DEBUG_BREAK();
							}
						}
					}
				}
				while (exists);
				return randomNeuronId;
			}

		};
	}
}