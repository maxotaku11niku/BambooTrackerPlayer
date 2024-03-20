/* BambooTrackerPlayer - Godot GDExtension which plays .BTM files with YM2608 emulation
 * Copyright (c) 2024 Maxim Hoxha
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * The actual player node, make sure to only use one of these at any time, consider making this an autoload
 */

#pragma once

#include <godot_cpp/classes/audio_stream_player.hpp>
#include <godot_cpp/classes/audio_stream_generator.hpp>
#include <godot_cpp/classes/audio_stream_generator_playback.hpp>
#include <memory>
#include "BambooTrackerModule.h"
#include "bt/opna_controller.hpp"
#include "bt/playback.hpp"
#include "bt/tick_counter.hpp"
#include "bt/instrument/instruments_manager.hpp"
#include "bt/io/btm_io.hpp"
#include "bt/module/module.hpp"

/* Conventions used by BambooTrackerPlayer for channel numbers
 * FM1-6: 0-5
 * SSG1-3: 6-8
 * Rhythm channels: 9-14 (bass, snare, cymbal, hihat, tom, rim in that order)
 * ADPCM: 15
 * FM3op1-3: 16-18 (FM3op4 is 2)
 *
 * Internal Bamboo Tracker conventions:
 * Normal: Matches BambooTrackerPlayer convention
 * FM3chExpanded:
 *   FM1-2: 0-1
 *   FM3op1-4: 2-5
 *   FM4-6: 6-8
 *   SSG1-3: 9-11
 *   Rhythm channels: 12-17
 *   ADPCM: 18
 */

/* Conventions used by BambooTrackerPlayer for note numbers
 * -1: No note playing
 * 0-95: C0-B7, note not retriggered
 * 128-223: C0-B7, but note was retriggered
 */

namespace godot
{
    class BambooTrackerPlayer : public AudioStreamPlayer
    {
        GDCLASS(BambooTrackerPlayer, AudioStreamPlayer)

    private:
        Ref<BambooTrackerModule> module;
        Ref<AudioStreamGenerator> outStream;
        Ref<AudioStreamGeneratorPlayback> playback;
        bool isSongPlaying;
        int64_t currentSongNum;
        int orderNum;
        int stepNum;
        int ticksToNextStep;
        double timeToNextTick;
        double numSampRemainder;
        double samplesPerTick;
        double secondsPerTick;
        std::shared_ptr<OPNAController> chipController;
        std::shared_ptr<InstrumentsManager> instManager;
        std::shared_ptr<TickCounter> tickCounter;
        std::shared_ptr<PlaybackManager> pbManager;
        std::shared_ptr<Module> currentModule;
        int16_t* sampleBuffer;
        PackedVector2Array outSampleBuffer;
        Song* currentSong;
        SongType currentSongType;
        int chNum;
        int currentNotes[19];
        bool retriggeredNotes[19];
        Step* currentSteps[19];

    protected:
        static void _bind_methods();

    public:
        BambooTrackerPlayer();
        ~BambooTrackerPlayer();

        void _process(double delta) override;
        void _ready() override;

        void setModule(const Ref<BambooTrackerModule> &mod);
        Ref<BambooTrackerModule> getModule() const;
        void setSongNumber(int64_t num);
        int64_t getSongNumber() const;
        void setOrderNumber(int64_t num);
        int64_t getOrderNumber() const;
        void setStepNumber(int64_t num);
        int64_t getStepNumber() const;

        //Call this after setting the module (something which you probably only need to do once) and before playing any songs
        void PlayNewModule();
        //This is not recommended unless your module only has a few songs. Please at least use macros for song names.
        void PlaySong(int64_t songNum, bool forceRestart = false);
        //More convenient if you like to reorder the songs, you should probably use this (and make the song names in the module symbolic and not their display names)
        void PlaySongFromName(String songName, bool forceRestart = false);
        void StopSong();
        //Get the value of the given OPNA register, returning 0 for any invalid registers
        int64_t GetRegister(int64_t addr);
        //Gets the currently playing note on an OPNA channel
        int64_t GetNote(int64_t channel);

    private:
        int GetRetriggerMask(int curNote, int prevNote, int stepNote, int index);
    };
}
