/* BambooTrackerPlayer - Godot GDExtension which plays .BTM files with YM2608 emulation
 * Maxim Hoxha 2024
 * Module resource, should be created from an imported module
 */

#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>

namespace godot
{
    class BambooTrackerModule : public Resource
    {
        GDCLASS(BambooTrackerModule, Resource)

    private:
        PackedByteArray moduleData;
        int64_t numberOfSongs;
        PackedStringArray songNames;

    protected:
        static void _bind_methods();

    public:
        BambooTrackerModule();
        ~BambooTrackerModule();

        Dictionary songNameMap;

        void setModuleData(const PackedByteArray &modData);
        PackedByteArray getModuleData() const;
        void setNumberOfSongs(const int64_t numSongs);
        int64_t getNumberOfSongs() const;
        void setSongNames(const PackedStringArray &songn);
        PackedStringArray getSongNames() const;
    };
}
