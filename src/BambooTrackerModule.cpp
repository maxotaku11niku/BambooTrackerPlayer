/* BambooTrackerPlayer - Godot GDExtension which plays .BTM files with YM2608 emulation
 * Maxim Hoxha 2024
 * Module resource, should be created from an imported module
 */

#include "BambooTrackerModule.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void BambooTrackerModule::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("getModuleData"), &BambooTrackerModule::getModuleData);
    ClassDB::bind_method(D_METHOD("setModuleData", "modData"), &BambooTrackerModule::setModuleData);
    ClassDB::add_property("BambooTrackerModule", PropertyInfo(Variant::PACKED_BYTE_ARRAY, "moduleData"), "setModuleData", "getModuleData");
}

BambooTrackerModule::BambooTrackerModule()
{

}

BambooTrackerModule::~BambooTrackerModule()
{

}

void BambooTrackerModule::setModuleData(const PackedByteArray &modData)
{
    moduleData = modData;
}

PackedByteArray BambooTrackerModule::getModuleData() const
{
    return moduleData;
}

