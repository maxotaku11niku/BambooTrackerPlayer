/* BambooTrackerPlayer - Godot GDExtension which plays .BTM files with YM2608 emulation
 * Maxim Hoxha 2024
 * The actual player node, make sure to only use one of these at any time, consider making this an autoload
 */

#pragma once

#include <godot_cpp/classes/audio_stream_player.hpp>

namespace godot
{
    class BambooTrackerPlayer : public AudioStreamPlayer
    {
        GDCLASS(BambooTrackerPlayer, AudioStreamPlayer)

    protected:
        static void _bind_methods();

    public:
        BambooTrackerPlayer();
        ~BambooTrackerPlayer();

        void _process(double delta) override;
    };
}
