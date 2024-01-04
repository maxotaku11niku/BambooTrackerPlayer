/* BambooTrackerPlayer - Godot GDExtension which plays .BTM files with YM2608 emulation
 * Maxim Hoxha 2024
 * The actual player node, make sure to only use one of these at any time, consider making this an autoload
 */

#pragma once

#include <godot_cpp/classes/audio_stream_player.hpp>
#include <godot_cpp/classes/audio_stream_generator.hpp>
#include <godot_cpp/classes/audio_stream_generator_playback.hpp>
#include "BambooTrackerModule.h"

namespace godot
{
    class BambooTrackerPlayer : public AudioStreamPlayer
    {
        GDCLASS(BambooTrackerPlayer, AudioStreamPlayer)

    private:
        Ref<BambooTrackerModule> module;
        Ref<AudioStreamGenerator> outStream;
        Ref<AudioStreamGeneratorPlayback> playback;

    protected:
        static void _bind_methods();

    public:
        BambooTrackerPlayer();
        ~BambooTrackerPlayer();

        void _process(double delta) override;
        void _ready() override;

        void setModule(const Ref<BambooTrackerModule> &mod);
        Ref<BambooTrackerModule> getModule() const;

        void PlaySong(int64_t songNum);
    };
}
