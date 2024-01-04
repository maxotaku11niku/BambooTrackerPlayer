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

    protected:
        static void _bind_methods();

    public:
        BambooTrackerModule();
        ~BambooTrackerModule();

        void setModuleData(const PackedByteArray &modData);
        PackedByteArray getModuleData() const;
    };
}
