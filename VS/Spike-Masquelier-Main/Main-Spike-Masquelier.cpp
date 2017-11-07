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

#ifdef _MSC_VER
#pragma warning (disable: 4350) //warning C4350: behavior change: 'std::_Wrap_alloc<_Alloc>::_Wrap_alloc(const std::_Wrap_alloc<_Alloc> &) throw()' called instead of 'std::_Wrap_alloc<_Alloc>::_Wrap_alloc<std::_Wrap_alloc<_Alloc>>(_Other &) throw()'	C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\include\vector

#	define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#	if !defined(NOMINMAX)
#		define NOMINMAX 1 
#	endif
#	if !defined(_CRT_SECURE_NO_WARNINGS)
#		define _CRT_SECURE_NO_WARNINGS 1
#	endif

#	if _DEBUG
		// see http://msdn.microsoft.com/en-us/library/x98tx3cf.aspx
#		define _CRTDBG_MAP_ALLOC
#		include <stdlib.h>
#		include <crtdbg.h>
#	endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <random>
#include <tuple>
#include <type_traits>
#include <memory>


#define _USE_SSE2

#include "../Spike-Tools-LIB/Constants.hpp"
#include "../Spike-Tools-LIB/timing.ipp"

#include "../Spike-Masquelier-LIB/v0/SpikeTools.hpp"
#include "../Spike-Masquelier-LIB/v0/Network0.hpp"
#include "../Spike-Masquelier-LIB/v0/Neuron0.hpp"
#include "../Spike-Masquelier-LIB/v0/SpikeOptionsMasq.hpp"

#include "../Spike-Masquelier-LIB/v0/Dumper0.hpp"
#include "../Spike-Masquelier-LIB/v0/DumperSpikesMasquelier.hpp"

#include "../Spike-Masquelier-LIB/v0/SpikeInputContainerStatic.hpp"
#include "../Spike-Masquelier-LIB/v0/SpikeInputContainerDynamic.hpp"
#include "../Spike-Masquelier-LIB/v0/SpikeInputContainerMatlab.hpp"
#include "../Spike-Masquelier-LIB/v0/SpikeEpspContainer.hpp"

#include "../Spike-Masquelier-LIB/v3/Network3.hpp"
#include "../Spike-Masquelier-LIB/v3/SpikeOptionsStatic.hpp"
#include "../Spike-Masquelier-LIB/v3/SpikeCase.hpp"
#include "../Spike-Masquelier-LIB/v3/SpikeStreamDataSet.hpp"
#include "../Spike-Masquelier-LIB/v3/SpikeStreamMatlab.hpp"
#include "../Spike-Masquelier-LIB/v3/Experiments.hpp"


namespace spike {

	std::tuple<std::vector<double>, std::vector<NeuronId>> loadMatlabSpikes(
		const std::string& filename,
		const double maxTimeInMs)
	{
		const std::string variableName1 = "spikeList";
		const std::string variableName2 = "afferentListDouble";

		std::vector<double> spikeTimesInMs;
		std::vector<NeuronId> afferent;

		mat_t *mat;
		matvar_t *matvar1;
		matvar_t *matvar2;

		mat = Mat_Open(filename.c_str(), MAT_ACC_RDONLY);
		if (mat) {
			matvar1 = Mat_VarRead(mat, (char*)variableName1.c_str());
			matvar2 = Mat_VarRead(mat, (char*)variableName2.c_str());

			if ((matvar1 == NULL) || (matvar2 == NULL)) {
				std::cerr << "spike::masquelier::SpikeInputContainerMatlab::load():error" << std::endl;
				throw 1;
			} else {
				Mat_VarReadDataAll(mat, matvar1);
				Mat_VarReadDataAll(mat, matvar2);

				const size_t length1 = matvar1->dims[1];
				const size_t length2 = matvar2->dims[1];

				if (length1 != length2) {
					std::cerr << "spike::masquelier::SpikeInputContainerMatlab::load(): sizes do not match " << std::endl;
					throw std::runtime_error("spike::masquelier::SpikeInputContainerMatlab::load(): sizes do not match");
				}

				//this->length_ = (numberOfInputEvents > length1) ? static_cast<unsigned int>(length1) : numberOfInputEvents;
				//this->spikeEvent_.resize(this->length_);

				char * data1 = (char *)matvar1->data;
				char * data2 = (char *)matvar2->data;
				const size_t stride1 = Mat_SizeOf(matvar1->data_type);
				const size_t stride2 = Mat_SizeOf(matvar2->data_type);


				unsigned int i = 0;
				bool continueLoading = true;
				while (continueLoading && (i < length1)) {
					if ((i & 0xFFFFF) == 0) std::cout << "spike::masquelier::SpikeInputContainerMatlab::load: loaded " << i << " spikes" << std::endl;

					const double timeInSec = *(double *)(data1 + (i*stride1));
					const double afferentDouble = *(double *)(data2 + (i*stride2));

					const double timeInMs = timeInSec * 1000;

					if (timeInMs > maxTimeInMs) {
						continueLoading = false;
						break;
					} else {
						spikeTimesInMs.push_back(timeInMs); // plus one to simulate a one ms delay
						afferent.push_back(static_cast<NeuronId>(std::lround(afferentDouble)));
						i++;
					}
				}
			}
			Mat_VarFree(matvar1);
			Mat_VarFree(matvar2);
			Mat_Close(mat);
		} else {
			std::cerr << "spike::masquelier::SpikeInputContainerMatlab::load:: could not load file " << filename << std::endl;
			throw std::runtime_error("spike::masquelier::SpikeInputContainerMatlab::load:: could not load file");
		}
		std::cout << "spike::masquelier::SpikeInputContainerMatlab::load: done loading " << spikeTimesInMs.size() << " spikes." << std::endl;
		return std::make_tuple(spikeTimesInMs, afferent);
	}

	template <typename Topology>
	std::shared_ptr<v3::SpikeStreamDataSet<Topology>> loadMnistSpikeStream(
		const v0::SpikeRuntimeOptions& SpikeRuntimeOptions)
	{
		using Options = Topology::Options;

		const auto spikeStream = std::make_shared<v3::SpikeStreamDataSet<Topology>>(SpikeRuntimeOptions);

		std::vector<NeuronId> neuronIds;
		for (const NeuronId& neuronId : Topology::iterator_AllNeurons()) {
			neuronIds.push_back(neuronId);
		}

		//1] create one random case
		const v3::TimeInMs caseTailSilence = 0;
		const v3::TimeInMs randomCaseDuration = SpikeRuntimeOptions.getRandomCaseDurationInMs();
		const CaseId nextCaseId = CaseId(0);
		const CaseLabel caseLabel = NO_CASE_LABEL;
		const auto spikeCase = std::make_shared<v3::SpikeCase<Options>>(v3::SpikeCase<Options>(nextCaseId, caseLabel, neuronIds, randomCaseDuration, caseTailSilence));
		spikeCase->setAllNeuronsRandomSpikeHz(SpikeRuntimeOptions.getRandomSpikeHz());
		spikeStream->add(std::move(spikeCase));


		//2.1 get the file name
		const std::string spikeDataSetFilename = sourceDir + "/../mnist data/28x28/SpikeDataSetMnist-test-only10-28x28.txt";
		const std::string translationsFilename = sourceDir + "/../mnist data/28x28/TranslationsMnist-test-only10-28x28.txt";

		const bool createNewSpikeDataSet = false;
		if (createNewSpikeDataSet) {
			std::cout << "spike::v3::Network::loadMnistSpikeStream: creating SpikeDataSet file " << spikeDataSetFilename << "; creating translations from file " << translationsFilename << std::endl;

			//TODO
			//const Translations<> translations = Translations<>();
			//translations.loadFromFile(translationsFilename);

		} else {
			std::cout << "spike::v3::Network::loadMnistSpikeStream: loading SpikeDataSet from file " << spikeDataSetFilename << std::endl;

			v3::SpikeDataSet<Options> spikeDataSet;
			spikeDataSet.loadFromFile(spikeDataSetFilename);

			//2.3 add the spike data to the stream
			spikeStream->addSpikeDataSet(spikeDataSet);
		}
		return spikeStream;
	}

	template <typename Topology>
	std::shared_ptr<spike::v3::SpikeStreamMatlab<Topology>> loadMasquelierMatlabSpikeStream(
		const v0::SpikeRuntimeOptions& SpikeRuntimeOptions)
	{
		using Options = Topology::Options;

		const auto spikeStream = std::make_shared<spike::v3::SpikeStreamMatlab<Topology>>(SpikeRuntimeOptions);

		std::vector<NeuronId> neuronIds;
		for (const NeuronId& neuronId : Topology::iterator_AllNeurons()) {
			neuronIds.push_back(neuronId);
		}

		//2.1 set the file name
		const std::string spikeInputFilename = "../../misc/afferent.rand000.mat";
		const std::string classificationFilename = "../../misc/classification.rand000.txt";

		std::cout << "spike::v3::Network::loadMasquelierMatlabSpikeStream: loading spikes from file " << spikeInputFilename << std::endl;

		const double timeDurationToLoadInMs = 10 * 1000;
		const auto dataTuple = loadMatlabSpikes(spikeInputFilename, timeDurationToLoadInMs);

		spike::v3::SpikeDataSet<Options> spikeDataSet;
		//spikeDataSet.loadFromFile(spikeDataSetFilename);

		//2.3 add the spike data to the stream
		spikeStream->addSpikeDataSet(spikeDataSet);

		return spikeStream;
	}


	void testNetworkV0_masquelier(const std::string& timeString)
	{
		printf("Running spike network testNetworkV0_masquelier: original Masquelier code\n");

		const int iMax = 2000;
		const int hMax = 3;

		const std::string epspSpikeFilename       = tempDir + "/v0/epsp-output-" + timeString + ".txt";
		const std::string potentialOutputFilename = tempDir + "/v0/potential-output-" + timeString + ".txt";
		const std::string weightOutputFilename    = tempDir + "/v0/weight-output-" + timeString + ".txt";

		const bool syncEpspToFile	= true;
		const bool dumperOn			= true;
		const bool beSmart			= true;
		const bool quiet			= false;

		////////////////////////////////////////////////////////////////////////
		const float alpha			= 0.25;
		const float alpha_plus		= 0.03125;
		const float alpha_minus		= 0.85f * alpha_plus;

		const float k				= 2.1165;
		//const float k				= 1.652; for tau_s = 1.5 and tau_m = 2
		const float k1				= 2;
		const float k2				= 4;

		const int refractory_period = 5;
		const float tau_s			= 2.5;

		const float threshold		= 550.0f;
		////////////////////////////////////////////////////////////////////////


		/////////////////////////////////////
		// Create Options
		const v0::SpikeOptionsMasq options = v0::SpikeOptionsMasq(alpha,
			alpha_plus, alpha_minus, k, k1, k2, refractory_period,
			tau_s, threshold, beSmart, quiet);

		std::cout << "Options = " << options.toString() << std::endl;

		/////////////////////////////////////
		// Create Options
		v0::SpikeRuntimeOptions spikeRuntimeOptions = v0::SpikeRuntimeOptions();

		// case generation options
		spikeRuntimeOptions.setCaseDurationInMs(50);
		spikeRuntimeOptions.setRefractoryPeriodInMs(5);
		spikeRuntimeOptions.setRandomCaseDurationInMs(500);
		spikeRuntimeOptions.setRandomSpikeHz(1);

		spikeRuntimeOptions.setFilenamePath_Spikes(tempDir   + "/v0/Spikes/Train");
		spikeRuntimeOptions.setFilenamePath_Topology(tempDir + "/v0/Topology/Topology");
		spikeRuntimeOptions.setFilenamePath_State(tempDir    + "/v0/State/State");

		spikeRuntimeOptions.setFilenamePrefix_Spikes("spikes");
		spikeRuntimeOptions.setFilenamePrefix_Topology("topology");
		spikeRuntimeOptions.setFilenamePrefix_State("state");

		spikeRuntimeOptions.setDumpIntervalInSec_Spikes(0 * 1 * 1);
		spikeRuntimeOptions.setDumpIntervalInSec_Topology(0 * 1 * 60 * 60);
		spikeRuntimeOptions.setDumpIntervalInSec_State(1 * 1 * 1);
		spikeRuntimeOptions.setDumpIntervalInSec_Group(0 * 1 * 60);

		//	std::cout << "spikeOptions = " << spikeOptions.toString() << std::endl;


		/////////////////////////////////////
		// Create SpikeDumper
		std::shared_ptr<v0::Dumper> dumper;
		if (dumperOn) {
			dumper = std::make_shared<v0::Dumper>(options, spikeRuntimeOptions);
			dumper->setWeightOutputFilename(weightOutputFilename);
			dumper->setPotentialOutputFilename(potentialOutputFilename);
		}
		std::shared_ptr<v0::DumperSpikesMasquelier> dumperSpikes;
		if (dumperOn) {
			dumperSpikes = std::make_shared<v0::DumperSpikesMasquelier>(options, spikeRuntimeOptions);
		}

		/////////////////////////////////////
		// Create SpikeInputContainer

		//const unsigned int nNeurons = iMax+hMax;
		const bool useMatlab = true;
		const bool useDynamic = false;

		using InputContainer = v0::SpikeInputContainerMatlab;
		//using InputContainer = v0::SpikeInputContainerStatic;
		//using InputContainer = v0::SpikeInputContainerDynamic<Topology>;

		const auto inputContainer = std::make_shared<InputContainer>(options, spikeRuntimeOptions);

		if (useMatlab) {
			inputContainer->setInputFilename("../../misc/afferent.rand000.mat");
			inputContainer->setClassificationFilename("../../misc/classification.rand000.txt");
			inputContainer->load(225 * 1000);
		} else if (useDynamic) {
			inputContainer->setInputFilename("../../misc/SpikeDataSetMnist-test-only10-train-14x14.txt");
			inputContainer->setClassificationFilename("../../misc/classification.rand000.txt");
			inputContainer->load(2 * 60 * 60 * 1000);
			//std::cout << "InputContainer=" << inputContainer->toString() << std::endl;
		} else {
			inputContainer->setInputFilename("../../misc/epsp-output-2013-08-06-13-05-55.txt");
			inputContainer->setClassificationFilename("../../misc/classification.rand000.txt");
			inputContainer->load(225 * 1000);
		}

		/////////////////////////////////////
		// Create SpikeEpspContainer
		const auto epspContainer = std::make_shared<v0::SpikeEpspContainer>(syncEpspToFile, options);
		epspContainer->setOutputFilename(epspSpikeFilename);

		/////////////////////////////////////
		// Create SpikeNetwork
		const auto network = std::make_shared<v0::Network0<InputContainer>>(iMax, hMax, options, spikeRuntimeOptions, inputContainer, epspContainer, dumper, dumperSpikes);
		network->t_max_inMs = inputContainer->getMaxTimeInMs();
		network->t_max = static_cast<v0::SpikeTime>(std::ceil((int)inputContainer->getMaxTimeInMs()* v0::SpikeOptionsMasq::TIME_DENOMINATOR));
		//network->setSpikeClassification(spikeClassification);
		network->initFull();
		//network->printCachedKernel();
		network->executeInputSpikesSerial();
	}

	void testNetworkV3_mnist()
	{
		printf("Running spike network testNetworkV3_izhikevich\n");

		auto spikeRuntimeOptions = v0::SpikeRuntimeOptions();

		spikeRuntimeOptions.setCaseDurationInMs(0);
		spikeRuntimeOptions.setCaseTailSilenceInMs(100);

		spikeRuntimeOptions.setRefractoryPeriodInMs(5);
		spikeRuntimeOptions.setRandomCaseDurationInMs(500);
		spikeRuntimeOptions.setRandomSpikeHz(1);
		spikeRuntimeOptions.setCorrectNeuronSpikeHz(2);


		spikeRuntimeOptions.setFilenamePath_Spikes(tempDir + "/v3-mnist/Spikes/Train");
		spikeRuntimeOptions.setFilenamePath_Topology(tempDir + "/v3-mnist/Topology");
		spikeRuntimeOptions.setFilenamePath_State(tempDir + "/v3-mnist/State");

		spikeRuntimeOptions.setFilenamePrefix_Spikes("spikes");
		spikeRuntimeOptions.setFilenamePrefix_Topology("topology");
		spikeRuntimeOptions.setFilenamePrefix_State("state");

		spikeRuntimeOptions.setDumpIntervalInSec_Spikes(1 * 1 * 1);
		spikeRuntimeOptions.setDumpIntervalInSec_State(0 * 1 * 60);
		spikeRuntimeOptions.setDumpIntervalInSec_Topology(1 * 60 * 60);
		spikeRuntimeOptions.setDumpIntervalInSec_Group(0 * 1 * 60);

		const size_t Ne = 800;
		const size_t Ni = 200;
		const size_t Ns = 28 * 28;
		const size_t Nm = 10;

		using Options = spike::v3::SpikeOptionsStatic<Ne, Ni, Ns, Nm>;
		using Top = spike::v3::Topology<Options>;
		using SpikeStream = spike::v3::SpikeStreamDataSet<Top>;


		Options staticOptions = Options();
		auto topology = std::make_shared<Top>();

		//topology->init_Izhikevich();
		topology->init_mnist();

		spike::v3::Network3<Top, SpikeStream> net(staticOptions, spikeRuntimeOptions);

		net.setTopology(topology);
		const auto spikeStream = loadMnistSpikeStream<Top>(spikeRuntimeOptions);
		net.setSpikeStream(spikeStream);

		const bool printKernelsBool = false;
		if (printKernelsBool) {
			net.printKernels(tempDir + "/v3-mnist/kernels.txt");
		}

		const unsigned int nSeconds = 1 * 1 * 60;
		const bool useConfusionMatrix = true;
		net.mainLoop(nSeconds, useConfusionMatrix);
	}

	void testNetworkV3_masquelier()
	{
		printf("Running spike network testNetworkV3_masquelier\n");

		v0::SpikeRuntimeOptions spikeRuntimeOptions = v0::SpikeRuntimeOptions();

		spikeRuntimeOptions.setCaseDurationInMs(0);
		spikeRuntimeOptions.setCaseTailSilenceInMs(100);

		spikeRuntimeOptions.setRefractoryPeriodInMs(5);
		spikeRuntimeOptions.setRandomCaseDurationInMs(500);
		spikeRuntimeOptions.setRandomSpikeHz(1);
		spikeRuntimeOptions.setCorrectNeuronSpikeHz(2);

		spikeRuntimeOptions.setFilenamePath_Spikes(tempDir + "/v3-masq/Spikes");
		spikeRuntimeOptions.setFilenamePath_Topology(tempDir + "/v3-masq/Topology");
		spikeRuntimeOptions.setFilenamePath_State(tempDir + "/v3-masq/State");

		spikeRuntimeOptions.setFilenamePrefix_Spikes("spikes");
		spikeRuntimeOptions.setFilenamePrefix_Topology("topology");
		spikeRuntimeOptions.setFilenamePrefix_State("state");

		spikeRuntimeOptions.setDumpIntervalInSec_Spikes(1 * 1 * 1);
		spikeRuntimeOptions.setDumpIntervalInSec_State(0 * 1 * 60);
		spikeRuntimeOptions.setDumpIntervalInSec_Topology(1 * 60 * 60);
		spikeRuntimeOptions.setDumpIntervalInSec_Group(0 * 1 * 60);

		const size_t Ne = 0;
		const size_t Ni = 3;
		const size_t Ns = 2000;
		const size_t Nm = 0;

		using Options = v3::SpikeOptionsStatic<Ne, Ni, Ns, Nm>;
		using Top = v3::Topology<Options>;
		using SpikeStream = v3::SpikeStreamMatlab<Top>;

		Options optionsV3 = Options();

		v3::Network3<Top, SpikeStream> net(optionsV3, spikeRuntimeOptions);

		auto topology = std::make_shared<Top>();
		topology->init_Masquelier();
		net.setTopology(topology);

		const auto spikeStream = loadMasquelierMatlabSpikeStream<Top>(spikeRuntimeOptions);
		net.setSpikeStream(spikeStream);

		const bool printKernelsBool = true;
		if (printKernelsBool) {
			net.printKernels(tempDir + "/v3-masq/kernels.txt");
		}

		const unsigned int nSeconds = 1 * 1 * 60;
		const bool useConfusionMatrix = true;
		net.mainLoop(nSeconds, useConfusionMatrix);
	}

	void runExperiments()
	{
		spike::tools::SpikeRuntimeOptions spikeRuntimeOptions = spike::tools::SpikeRuntimeOptions();

		spikeRuntimeOptions.setRefractoryPeriodInMs(5);
		spikeRuntimeOptions.setRandomSpikeHz(1);

		spikeRuntimeOptions.setFilenamePath_Spikes(tempDir + "/v3/Spikes/Train");
		spikeRuntimeOptions.setFilenamePath_State(tempDir + "/v3/State");

		spikeRuntimeOptions.setFilenamePrefix_Spikes("spikes");
		spikeRuntimeOptions.setFilenamePrefix_State("state");

		const size_t Ne = 3;
		const size_t Ni = 0;
		const size_t Ns = 0;
		const size_t Nm = 0;

		using Options = spike::v3::SpikeOptionsStatic<Ne, Ni, Ns, Nm>;

		//spike::v3::experiment::experiment1<Options>(spikeRuntimeOptions);
		spike::v3::experiment::experiment5<Options>(spikeRuntimeOptions);
	}
}

int main(int /*argc*/, char** /*argv[]*/)
{
	const auto start = std::chrono::system_clock::now();

	//srand((unsigned int)time(NULL));
	srand(123456789);

	spike::testNetworkV0_masquelier("now"); // works

	//spike::testNetworkV3_mnist();
	//spike::testNetworkV3_masquelier();
	//spike::runExperiments();

	std::cout << "DONE: passed time = " << tools::timing::elapsed_time_str(start, std::chrono::system_clock::now());
	printf("\n-------------------\n");
	printf("\nPress RETURN to finish:");
	getchar();
	return 0;
}