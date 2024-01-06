/* BambooTrackerPlayer - Godot GDExtension which plays .BTM files with YM2608 emulation
 * Maxim Hoxha 2024
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

        //Call this after setting the module (something which you probably only need to do once) and before playing any songs
        void PlayNewModule();
        //This is not recommended unless your module only has a few songs. Please at least use macros for song names.
        void PlaySong(int64_t songNum, bool forceRestart = false);
        //More convenient if you like to reorder the songs, you should probably use this (and make the song names in the module symbolic and not their display names)
        void PlaySongFromName(String songName, bool forceRestart = false);
        void StopSong();
    };
}
