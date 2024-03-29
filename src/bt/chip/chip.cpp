/*
 * Copyright (C) 2018-2022 Rerrah
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

#include "chip.hpp"
#include <utility>
#include "resampler.hpp"
//#include "register_write_logger.hpp" MH - We don't need to log register writes

namespace chip
{
Chip::Chip(int id, int clock, int rate, int autoRate, size_t maxDuration,
		   std::unique_ptr<AbstractResampler> resampler1, std::unique_ptr<AbstractResampler> resampler2,
		   std::shared_ptr<AbstractRegisterWriteLogger> logger)
	: id_(id),
	  rate_(rate),	// Dummy set
	  clock_(clock),
	  autoRate_(autoRate),
	  maxDuration_(maxDuration),
	  masterVolumeRatio_(100),
	  logger_(logger)
{
	resampler_[0] = std::move(resampler1);
	resampler_[1] = std::move(resampler2);

	for (int pan = STEREO_LEFT; pan <= STEREO_RIGHT; ++pan) {
		for (auto& buf : buffer_) {
			buf[pan] = new sample[CHIP_SMPL_BUF_SIZE_];
		}
	}
}

Chip::~Chip()
{
	for (int pan = STEREO_LEFT; pan <= STEREO_RIGHT; ++pan) {
		for (auto& buf : buffer_) {
			delete[] buf[pan];
		}
	}
}

void Chip::initResampler()
{
	for (int snd = 0; snd < 2; ++snd) {
		resampler_[snd]->init(internalRate_[snd], rate_, maxDuration_);
	}
}

void Chip::reset()
{
	std::lock_guard<std::mutex> lg(mutex_);

	for (int snd = 0; snd < 2; ++snd) {
		resampler_[snd]->reset();
	}

	resetSpecific();
}

void Chip::setRate(int rate)
{
	std::lock_guard<std::mutex> lg(mutex_);

	funcSetRate(rate);

	for (auto& rsmp : resampler_) {
		rsmp->setDestributionRate(rate);
	}
}

void Chip::funcSetRate(int rate) noexcept
{
	rate_ = rate ? rate : autoRate_;
}

void Chip::updateVolumeRatio()
{
	for (int i = 0; i < 2; ++i) {
		updateVolumeRatio(i);
	}
}

void Chip::updateVolumeRatio(int i)
{
	volumeRatio_[i] = busVolumeRatio_[i] * masterVolumeRatio_;
}

void Chip::setMaxDuration(size_t maxDuration)
{
	maxDuration_ = maxDuration;
	for (int snd = 0; snd < 2; ++snd) {
		resampler_[snd]->setMaxDuration(maxDuration);
	}
}

void Chip::setRegisterWriteLogger(std::shared_ptr<AbstractRegisterWriteLogger> logger)
{
	logger_ = logger;
}

void Chip::setMasterVolume(int percentage)
{
	masterVolumeRatio_ = percentage / 100.0;
	updateVolumeRatio();
}
}
