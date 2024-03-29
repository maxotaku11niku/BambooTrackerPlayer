/*
 * Copyright (C) 2018-2023 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "opna.hpp"
#include <cstdint>
#include <cmath>
#include <algorithm>
//#include "register_write_logger.hpp" MH - We don't need to log register writes
//#include "mame/mame_2608.hpp" MH - Not included due to license incompatibility
//#include "nuked/nuked_2608.hpp" MH - Not included due to license incompatibility
#include "ymfm/ymfm_2608.hpp"

#ifdef USE_REAL_CHIP
#include "scci/scci_wrapper.hpp"
#include "c86ctl/c86ctl_wrapper.hpp"
#endif

namespace chip
{
namespace
{
constexpr double VOL_REDUC_ = 7.5;

// 55466Hz: FM internal rate
constexpr int DEFAULT_AUTO_RATE = 55466;

enum SoundSourceIndex : int { FM = 0, SSG = 1 };

enum WriteMode : int { WAIT_MODE = 0, IMMEDIATE_MODE = 1 };

inline double clamp(double value, double low, double high)
{
	return std::min<double>(std::max<double>(value, low), high);
}

void gainSamples(sample** samples, size_t nSamples, double gain)
{
	for (int pan = STEREO_LEFT; pan <= STEREO_RIGHT; ++pan) {
		std::transform(samples[pan], samples[pan] + nSamples, samples[pan],
					   [gain](sample s) { return static_cast<sample>(s * gain); });
	}
}
}

size_t OPNA::count_ = 0;

OPNA::OPNA(OpnaEmulator emu, int clock, int rate, size_t maxDuration, size_t dramSize,
		   std::unique_ptr<AbstractResampler> fmResampler, std::unique_ptr<AbstractResampler> ssgResampler,
		   std::shared_ptr<AbstractRegisterWriteLogger> logger)
	: Chip(count_++, clock, rate, DEFAULT_AUTO_RATE, maxDuration,
		   std::move(fmResampler), std::move(ssgResampler),
		   logger),
	  volumeFm_(0),
	  volumeSsg_(0),
	  dramSize_(dramSize),
	  //rcIntf_(std::make_unique<SimpleRealChipInterface>()), MH - Not included because I have chosen not to support real chips
	  isForcedRegWrite_(false),
	  waitRestFm_(0),
	  waitRestSsg2_(0),
	  writeFuncs {
		  // Wait mode
{ &OPNA::enqueueData, &OPNA::storeBufferForWait },
		  // Immediate mode
{ &OPNA::writeDataImmediately, &OPNA::storeBufferForImmediate }
},
	  writeFunc(&writeFuncs[WAIT_MODE])
{
	for (int pan = STEREO_LEFT; pan <= STEREO_RIGHT; ++pan) {
		tmpBuf_[pan] = new sample[CHIP_SMPL_BUF_SIZE_];
	}

	switch (emu) {
	default:
		fprintf(stderr, "Unknown emulator choice. Using the default.\n");
		/* fall through */
	/* MH - Not included due to license incompatibility
	case OpnaEmulator::Mame:
		fprintf(stderr, "Using emulator: MAME YM2608\n");
		intf_ = std::make_unique<Mame2608>();
		break;
	case OpnaEmulator::Nuked:
		fprintf(stderr, "Using emulator: Nuked OPN-Mod\n");
		intf_ = std::make_unique<Nuked2608>();
		break;
	*/
	case OpnaEmulator::Ymfm:
		fprintf(stderr, "Using emulator: ymfm\n");
		intf_ = std::make_unique<Ymfm2608>();
		break;
	}

	funcSetRate(rate);

	internalRate_[FM] = intf_->startDevice(clock, internalRate_[SSG], dramSize);
	rate2_ = static_cast<size_t>((internalRate_[SSG] << 1) / internalRate_[FM]);	// Should be "9"

	initResampler();

	setVolumeFM(0);
	setVolumeSSG(0);

	reset();
}

OPNA::~OPNA()
{
	intf_->stopDevice();
	--count_;

	for (int pan = STEREO_LEFT; pan <= STEREO_RIGHT; ++pan) {
		delete[] tmpBuf_[pan];
	}
}

void OPNA::resetSpecific()
{
	isForcedRegWrite_ = false;
	regWrites_.clear();
	forcedRegWrites_.clear();
	waitRestFm_ = 0;
	waitRestSsg2_ = 0;

	intf_->resetDevice();
	//rcIntf_->reset(); MH - Not included because I have chosen not to support real chips
}

void OPNA::setImmediateWriteMode(bool enabled) noexcept
{
	writeFunc = &writeFuncs[enabled ? IMMEDIATE_MODE : WAIT_MODE];
}

bool OPNA::isImmediateWriteMode() const noexcept
{
	return writeFunc == &writeFuncs[IMMEDIATE_MODE];
}

void OPNA::setRegister(uint32_t offset, uint8_t value)
{
	std::lock_guard<std::mutex> lg(mutex_);

	/*  MH - We don't need to log register writes
	if (logger_) {
		logger_->recordRegisterChange(offset, value);
	}
	else {
	*/
	(this->*writeFunc->setRegister)(offset, value);
	//}
	//rcIntf_->setRegister(offset, value); MH - Not included because I have chosen not to support real chips
}

void OPNA::enqueueData(uint32_t offset, uint8_t value)
{
	if (isForcedRegWrite_) {
		forcedRegWrites_.push_back({ offset & 0x0ff, value, !(offset & 0x100) });
	}
	else {
		regWrites_.push_back({ offset & 0x0ff, value, !(offset & 0x100) });
	}
}

void OPNA::writeDataImmediately(uint32_t offset, uint8_t value)
{
	if (offset & 0x100) {
		intf_->writeAddressToPortB(offset & 0xff);
		intf_->writeDataToPortB(value & 0xff);
	}
	else {
		intf_->writeAddressToPortA(offset & 0xff);
		intf_->writeDataToPortA(value & 0xff);
	}
}

uint8_t OPNA::getRegister(uint32_t offset) const
{
	if (offset & 0x100) intf_->writeAddressToPortB(offset & 0xff);
	else intf_->writeAddressToPortA(offset & 0xff);
	return intf_->readData();
}

//MH - Included this function to allow BambooTrackerPlayer to read OPNA registers for fun
uint8_t OPNA::getRegisterDebug(uint32_t offset) const
{
	return intf_->readDataDebug(offset);
}

void OPNA::setVolumeFM(double dB)
{
	std::lock_guard<std::mutex> lg(mutex_);
	volumeFm_ = dB;
	busVolumeRatio_[FM] = std::pow(10.0, (dB - VOL_REDUC_) / 20.0) / VOLUME_RATIO_MOD_;
	updateVolumeRatio(FM);
}

void OPNA::setVolumeSSG(double dB)
{
	std::lock_guard<std::mutex> lg(mutex_);
	volumeSsg_ = dB;
	busVolumeRatio_[SSG] = std::pow(10.0, (dB - VOL_REDUC_) / 20.0) / VOLUME_RATIO_MOD_;
	updateVolumeRatio(SSG);

	//rcIntf_->setSSGVolume(dB); MH - Not included because I have chosen not to support real chips
}

size_t OPNA::getDRAMSize() const noexcept
{
	return dramSize_;
}

bool OPNA::mix(int16_t* stream, size_t nSamples)
{
	std::lock_guard<std::mutex> lg(mutex_);

	size_t pointFm = 0;
	size_t pointSsg = 0;

	// Store samples to internal buffer
	bool result = (this->*writeFunc->storeBuffer)(nSamples, pointFm, pointSsg);
	if (!result) return false;

	// Gain volume
	gainSamples(buffer_[FM], pointFm, volumeRatio_[FM]);
	gainSamples(buffer_[SSG], pointSsg, volumeRatio_[SSG]);

	// Resampling
	sample** bufFM = resampler_[FM]->interpolate(buffer_[FM], nSamples, pointFm);
	sample** bufSSG = resampler_[SSG]->interpolate(buffer_[SSG], nSamples, pointSsg);

	// Mix
	int16_t* p = stream;
	for (size_t i = 0; i < nSamples; ++i) {
		for (int pan = STEREO_LEFT; pan <= STEREO_RIGHT; ++pan) {
			*p++ = static_cast<int16_t>(clamp((bufFM[pan][i] + bufSSG[pan][i]) * VOLUME_RATIO_MOD_, -32768, 32767));
		}
	}

	return true;
}

/**
 * @brief OPNA::dequeueData
 * @return wait count as FM internal sample rate
 */
size_t OPNA::dequeueData()
{
	size_t waitCount = 0;

	if (!forcedRegWrites_.empty()) {
		auto& unit = forcedRegWrites_.front();
		if (unit.isPortA_) {
			intf_->writeAddressToPortA(unit.address & 0xff);
			intf_->writeDataToPortA(unit.data & 0xff);
		}
		else {
			intf_->writeAddressToPortB(unit.address & 0xff);
			intf_->writeDataToPortB(unit.data & 0xff);
		}
		waitCount = unit.address == 0x10 ? 4 : 1;
		forcedRegWrites_.pop_front();
	}
	else if (!regWrites_.empty()) {
		auto& unit = regWrites_.front();
		if (unit.isPortA_) {
			intf_->writeAddressToPortA(unit.address & 0xff);
			intf_->writeDataToPortA(unit.data & 0xff);
		}
		else {
			intf_->writeAddressToPortB(unit.address & 0xff);
			intf_->writeDataToPortB(unit.data & 0xff);
		}
		waitCount = unit.address == 0x10 ? 4 : 1;
		regWrites_.pop_front();
	}

	return waitCount;
}

bool OPNA::storeBufferForImmediate(size_t nSamples, size_t& pointFm, size_t& pointSsg)
{
	bool ok = false;
	size_t intrSizeFm = resampler_[FM]->calculateInternalSampleSize(nSamples, ok);
	if (!ok) return false;
	intf_->updateStream(tmpBuf_, intrSizeFm);
	for (int pan = STEREO_LEFT; pan <= STEREO_RIGHT; ++pan) {
		std::copy_n(tmpBuf_[pan], intrSizeFm, buffer_[FM][pan] + pointFm);
	}
	pointFm += intrSizeFm;

	size_t intrSizeSsg = resampler_[SSG]->calculateInternalSampleSize(nSamples, ok);
	if (!ok) return false;
	intf_->updateSsgStream(tmpBuf_, intrSizeSsg);
	for (int pan = STEREO_LEFT; pan <= STEREO_RIGHT; ++pan) {
		std::copy_n(tmpBuf_[pan], intrSizeSsg, buffer_[SSG][pan] + pointSsg);
	}
	pointSsg += intrSizeSsg;

	return true;
}

void OPNA::flushWait(size_t& pointFm, size_t maxFm, size_t& pointSsg2, size_t maxSsg2)
{
	size_t sizeFm = std::min(waitRestFm_, maxFm - pointFm);
	intf_->updateStream(tmpBuf_, sizeFm);
	for (int pan = STEREO_LEFT; pan <= STEREO_RIGHT; ++pan) {
		std::copy_n(tmpBuf_[pan], sizeFm, buffer_[FM][pan] + pointFm);
	}
	waitRestFm_ -= sizeFm;
	pointFm += sizeFm;

	size_t pointSsg = pointSsg2 >> 1;
	size_t endPointSsg2 = std::min(pointSsg2 + waitRestSsg2_, maxSsg2);
	size_t sizeSsg = (endPointSsg2 >> 1) - pointSsg;
	intf_->updateSsgStream(tmpBuf_, sizeSsg);
	for (int pan = STEREO_LEFT; pan <= STEREO_RIGHT; ++pan) {
		std::copy_n(tmpBuf_[pan], sizeSsg, buffer_[SSG][pan] + pointSsg);
	}
	size_t sizeSsg2 = endPointSsg2 - pointSsg2;
	waitRestSsg2_ -= sizeSsg2;
	pointSsg2 += sizeSsg2;
}

bool OPNA::storeBufferForWait(size_t nSamples, size_t& pointFm, size_t& pointSsg)
{	
	bool ok = false;
	size_t intrSizeFm = resampler_[FM]->calculateInternalSampleSize(nSamples, ok);
	if (!ok) return false;
	size_t intrSizeSsg2 = resampler_[SSG]->calculateInternalSampleSize(nSamples, ok) << 1;
	if (!ok) return false;

	size_t pointSsg2 = pointSsg << 1;

	// Flush previous wait
	flushWait(pointFm, intrSizeFm, pointSsg2, intrSizeSsg2);

	// Wait and generate
	while (pointFm < intrSizeFm || pointSsg2 < intrSizeSsg2) {
		size_t waitCount = dequeueData();
		if (!waitCount) break;

		// Add wait count
		waitRestFm_ += waitCount;
		waitRestSsg2_ += rate2_ * waitCount;
		flushWait(pointFm, intrSizeFm, pointSsg2, intrSizeSsg2);
	}

	// Generate rest samples
	size_t sizeFm = intrSizeFm - pointFm;
	intf_->updateStream(tmpBuf_, sizeFm);
	for (int pan = STEREO_LEFT; pan <= STEREO_RIGHT; ++pan) {
		std::copy_n(tmpBuf_[pan], sizeFm, buffer_[FM][pan] + pointFm);
	}
	if (sizeFm <= waitRestFm_) waitRestFm_ -= sizeFm;
	pointFm += sizeFm;

	pointSsg = pointSsg2 >> 1;
	size_t sizeSsg = (intrSizeSsg2 >> 1) - pointSsg;
	intf_->updateSsgStream(tmpBuf_, sizeSsg);
	for (int pan = STEREO_LEFT; pan <= STEREO_RIGHT; ++pan) {
		std::copy_n(tmpBuf_[pan], sizeSsg, buffer_[SSG][pan] + pointSsg);
	}
	size_t sizeSsg2 = intrSizeSsg2 - pointSsg2;
	if (sizeSsg2 <= waitRestSsg2_) waitRestSsg2_ -= sizeSsg2;
	pointSsg = (pointSsg2 + sizeSsg2) >> 1;

	return true;
}

void OPNA::setFmResampler(std::unique_ptr<AbstractResampler> resampler)
{
	std::lock_guard<std::mutex> lg(mutex_);
	resampler_[FM] = std::move(resampler);
	initResampler();
}

void OPNA::setSsgResampler(std::unique_ptr<AbstractResampler> resampler)
{
	std::lock_guard<std::mutex> lg(mutex_);
	resampler_[SSG] = std::move(resampler);
	initResampler();
}

/* MH - Not included because I have chosen not to support real chips
void OPNA::connectToRealChip(RealChipInterfaceType type, RealChipInterfaceGeneratorFunc* f)
{
	switch (type) {
	default:	// Fall through
	case RealChipInterfaceType::NONE:
		if (rcIntf_->getType() != RealChipInterfaceType::NONE)
			rcIntf_ = std::make_unique<SimpleRealChipInterface>();
		rcIntf_->createInstance(f);
		break;
#ifdef USE_REAL_CHIP
	case RealChipInterfaceType::SCCI:
		if (rcIntf_->getType() != RealChipInterfaceType::SCCI)
			rcIntf_ = std::make_unique<Scci>();
		rcIntf_->createInstance(f);
		break;
	case RealChipInterfaceType::C86CTL:
		if (rcIntf_->getType() != RealChipInterfaceType::C86CTL)
			rcIntf_ = std::make_unique<C86ctl>();
		rcIntf_->createInstance(f);
		break;
#endif
	}
}

RealChipInterfaceType OPNA::getRealChipInterfaceType() const
{
	return rcIntf_->getType();
}

bool OPNA::hasConnectedToRealChip() const
{
	return rcIntf_->hasConnected();
}
*/
}
