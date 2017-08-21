#include "SpectrumAnalyzer.hpp"

#include <iostream>

using namespace std;

SpectrumAnalyzer::SpectrumAnalyzer(std::shared_ptr<AudioDevice>& _audioDevice,
	double fStart, double fEnd,
	double binsPerOctave, unsigned int maxBlockSize)
	:	workUnit(std::make_unique<boost::asio::io_service::work>(ioService))
	,	leftSpectrum(fStart, fEnd, binsPerOctave)
	,	rightSpectrum(fStart, fEnd, binsPerOctave)
	,	audioDevice(_audioDevice)
	,	chunkSize{audioDevice->getBlockSize()} {

	//Determine optimum block size
	double minResolution = leftSpectrum.begin()->getFreqEnd() -
		leftSpectrum.begin()->getFreqStart();
	
	unsigned int chunksPerBlockPower =
		std::ceil(std::log2(audioDevice->getSampleRate() /
		(minResolution * chunkSize)));
	
	blockSize = chunkSize * (1 << chunksPerBlockPower);

	if(blockSize > maxBlockSize) {
		cout << "[Warning] Optimal block size of " << blockSize << " too large, using "
			<< maxBlockSize << " instead" << endl;

		blockSize = maxBlockSize;
	}
	else {
		std::cout << "[Info] For minimum resolution of " << (int)(minResolution+0.5)
			<< "Hz, using block size of " << blockSize << std::endl;
	}
	
	//resolution = Fs / blockSize
	//blockSize = Fs/resolution
	//chunksPerBlock = blockSize / chunkSize = Fs/(resolution * chunkSize)

	//Initialize FFTW stuff
	//fftIn, fftOut, fftPlan
	fftIn = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * blockSize);
	fftOut = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * blockSize);

	//Compute FFT plans
	fftPlan = fftw_plan_dft_1d(blockSize, fftIn, fftOut,
		FFTW_FORWARD, FFTW_MEASURE);

	//Generate FFT window function
	generateWindow();

	//Initialize audio buffers
	leftBuffer.resize(blockSize);
	rightBuffer.resize(blockSize);

	//Launch thread
	asyncThread = std::thread(std::bind(&SpectrumAnalyzer::threadRoutine,
		this));

	//Register audio callback
	callbackID = audioDevice->addCallback(std::bind(&SpectrumAnalyzer::cbAudio, this,
		std::placeholders::_1, std::placeholders::_2));
}

SpectrumAnalyzer::~SpectrumAnalyzer() {
	//Remove audio callback
	audioDevice->removeCallback(callbackID);

	//Shutdown threads
	workUnit.reset();

	asyncThread.join();

	//Cleanup fftw stuff
	fftw_destroy_plan(fftPlan);
	fftw_free(fftIn);
	fftw_free(fftOut);
}

std::shared_ptr<AudioDevice> SpectrumAnalyzer::getAudioDevice() {
	return audioDevice;
}

Spectrum SpectrumAnalyzer::getLeftSpectrum() const {
	std::unique_lock<std::mutex> spectrumLock(spectrumMutex);

	return leftSpectrum;
}

Spectrum SpectrumAnalyzer::getRightSpectrum() const {
	std::unique_lock<std::mutex> spectrumLock(spectrumMutex);

	return rightSpectrum;
}

Spectrum SpectrumAnalyzer::getMonoSpectrum() const {
	std::unique_lock<std::mutex> spectrumLock(spectrumMutex);

	auto mono = leftSpectrum + rightSpectrum;

	//Average
	for(unsigned int i = 0; i < mono.getBinCount(); ++i) {
		auto& bin = mono.getByIndex(i);
		bin.setEnergy(bin.getEnergy() / 2.);
	}

	return mono;
}

Spectrum SpectrumAnalyzer::getCenterSpectrum() const {
	std::unique_lock<std::mutex> spectrumLock(spectrumMutex);

	auto center = leftSpectrum;
/*
	auto& leftBin = center.getByIndex(2);
	auto& rightBin = right.getByIndex(2);

	std::cout << leftBin.getEnergy() << " " << rightBin.getEnergy() << " "
		<< std::max(0., leftBin.getEnergy() - rightBin.getEnergy()) << std::endl;
*/

	for(unsigned int i = 0; i < center.getBinCount(); ++i) {
		auto& leftBin = center.getByIndex(i);
		auto rightBin = rightSpectrum.getByIndexConst(i);
		
		double leftOnly = std::max(0., leftBin.getEnergy() - rightBin.getEnergy());
		double rightOnly = std::max(0., rightBin.getEnergy() - leftBin.getEnergy());
		leftBin.setEnergy(leftBin.getEnergy() - leftOnly);
	}

	return center;
}

void SpectrumAnalyzer::cbAudio(const int16_t* left, const int16_t* right) {
	std::unique_lock<std::mutex> bufferLock(bufferMutex);

	//Shift the samples forward by 1 chunk size
	std::memmove(leftBuffer.data(), &leftBuffer[chunkSize],
		sizeof(int16_t) * (blockSize - chunkSize));
	std::memmove(rightBuffer.data(), &rightBuffer[chunkSize],
		sizeof(int16_t) * (blockSize - chunkSize));

	//Copy the new chunk samples into the end of the block
	std::memcpy(&leftBuffer[blockSize - chunkSize], left,
		sizeof(int16_t) * chunkSize);
	std::memcpy(&rightBuffer[blockSize - chunkSize], right,
		sizeof(int16_t) * chunkSize);

	//Post the fft routine to the async thread
	ioService.post(std::bind(&SpectrumAnalyzer::fftRoutine, this,
		leftBuffer, rightBuffer));
}

void SpectrumAnalyzer::threadRoutine() {
	//Run work from ioService
	ioService.run();
}

void SpectrumAnalyzer::fftRoutine(std::vector<int16_t> left,
	std::vector<int16_t> right) {

	double sampleRate = audioDevice->getSampleRate();
	
	{
		std::unique_lock<std::mutex> bufferLock(bufferMutex);
		
		//Fill FFT input buffer
		for(unsigned int i = 0; i < blockSize; i++) {
			fftIn[i][0] = fftWindow[i] *
				((double)left[i] / INT16_MAX / blockSize);
			fftIn[i][1] = 0.; //Imaginary
		}
	}

	//Do FFT on left samples
	fftw_execute(fftPlan);

	//Fill left spectrum with new FFT data
	leftSpectrum.clear();

	for(unsigned int i = 0; i < blockSize/2; ++i) {
		double f = sampleRate * i / blockSize; //Frequency of fft bin

		try {
			//Put the energy from this bin into the appropriate location
			leftSpectrum.get(f).addEnergy(std::sqrt(sqr(fftOut[i][0]) +
				sqr(fftOut[i][1])));
		}
		catch(const Exception& e) {
			if(e.getErrorCode() != Spectrum::ERROR_BIN_NOT_FOUND) {
				std::cout << "[Error] SpectrumAnalyzer::fftRoutine Exception caught: "
					<< e.what() << std::endl;
			}
			else {
				//This is not an error
				//The frequency is not in the range of interest for the spectrum
			}
		}
	}
	
	//Now do right FFT

	{
		std::unique_lock<std::mutex> bufferLock(bufferMutex);

		//Copy real audio data into complex fft input array and scale to [-1., 1.]
		for(unsigned int i = 0; i < blockSize; i++) {
			fftIn[i][0] = fftWindow[i] * ((double)right[i] / INT16_MAX / blockSize); //Real
			fftIn[i][1] = 0.; //Imaginary
		}
	}

	//Do FFT on right samples
	fftw_execute(fftPlan);

	//Fill right spectrum with new FFT data
	rightSpectrum.clear();

	for(unsigned int i = 0; i < blockSize/2; ++i) {
		double f = sampleRate * i / blockSize; //Frequency of fft bin

		try {
			//Put the energy from this bin into the appropriate location
			rightSpectrum.get(f).addEnergy(std::sqrt(sqr(fftOut[i][0]) +
				sqr(fftOut[i][1])));
		}
		catch(const Exception& e) {
			if(e.getErrorCode() != Spectrum::ERROR_BIN_NOT_FOUND) {
				std::cout << "[Error] SpectrumAnalyzer::cbAudio Exception caught: "
					<< e.what() << std::endl;
			}
			else {
				//This is not an error
				//The frequency is not in the range of interest for the spectrum
			}
		}
	}

	//Update stats for both spectrums
	leftSpectrum.updateStats();
	rightSpectrum.updateStats();
}

void SpectrumAnalyzer::generateWindow() {
	//Hanning window
	fftWindow.resize(blockSize);

	for(unsigned int i = 0; i < blockSize; i++) {
		fftWindow[i] = 0.5 * (1. - std::cos((2*3.141592654*i)/(blockSize - 1)));
	}
}

//Helper function
double SpectrumAnalyzer::sqr(const double x) {
	return x*x;
}
