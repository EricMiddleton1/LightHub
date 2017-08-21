#pragma once

#include <memory>
#include <thread>
#include <functional>
#include <cstdint>
#include <vector>

#include <boost/asio.hpp>
#include <boost/signals2.hpp>

#include <fftw3.h>

#include "AudioDevice.hpp"
#include "Spectrum.hpp"

class SpectrumAnalyzer
{
public:
	SpectrumAnalyzer(std::shared_ptr<AudioDevice>& audioDevice,
		double fStart, double fEnd,
		double binsPerOctave, unsigned int maxBlockSize);
	~SpectrumAnalyzer();


	std::shared_ptr<AudioDevice> getAudioDevice();

	Spectrum getLeftSpectrum() const;
	Spectrum getRightSpectrum() const;
	Spectrum getMonoSpectrum() const;
	Spectrum getCenterSpectrum() const;

private:
	void threadRoutine();
	void cbAudio(const int16_t* left, const int16_t* right);
	void fftRoutine(std::vector<int16_t>, std::vector<int16_t>);
	void generateWindow();

	static double sqr(const double x);

	//Thread stuff
	boost::asio::io_service ioService;
	std::unique_ptr<boost::asio::io_service::work> workUnit;
	std::thread asyncThread;

	Spectrum leftSpectrum, rightSpectrum;
	mutable std::mutex spectrumMutex;

	//Audio sample buffers
	std::vector<int16_t> leftBuffer, rightBuffer;
	std::mutex bufferMutex;

	//FFT stuff
	fftw_complex *fftIn, *fftOut;
	fftw_plan fftPlan;
	std::vector<double> fftWindow;

	//Audio device stuff
	std::shared_ptr<AudioDevice> audioDevice;
	unsigned int callbackID;
	unsigned int chunkSize; //Size of buffer from audio device
	unsigned int blockSize;	//Size of buffer sent through fft
};
