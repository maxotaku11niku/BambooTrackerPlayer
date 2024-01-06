/* BambooTrackerPlayer - Godot GDExtension which plays .BTM files with YM2608 emulation
 * Maxim Hoxha 2024
 * Module resource, should be created from an imported module
 */

#include "BambooTrackerModule.h"
#include <godot_cpp/core/class_db.hpp>
#include <memory>
#include "bt/instrument/instruments_manager.hpp"
#include "bt/io/btm_io.hpp"
#include "bt/module/module.hpp"

using namespace godot;

void BambooTrackerModule::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("getModuleData"), &BambooTrackerModule::getModuleData);
    ClassDB::bind_method(D_METHOD("setModuleData", "modData"), &BambooTrackerModule::setModuleData);
    ClassDB::add_property("BambooTrackerModule", PropertyInfo(Variant::PACKED_BYTE_ARRAY, "moduleData"), "setModuleData", "getModuleData");
    ClassDB::bind_method(D_METHOD("getNumberOfSongs"), &BambooTrackerModule::getNumberOfSongs);
    ClassDB::bind_method(D_METHOD("setNumberOfSongs", "numSongs"), &BambooTrackerModule::setNumberOfSongs);
    ClassDB::add_property("BambooTrackerModule", PropertyInfo(Variant::INT, "numberOfSongs"), "setNumberOfSongs", "getNumberOfSongs");
    ClassDB::bind_method(D_METHOD("getSongNames"), &BambooTrackerModule::getSongNames);
    ClassDB::bind_method(D_METHOD("setSongNames", "songn"), &BambooTrackerModule::setSongNames);
    ClassDB::add_property("BambooTrackerModule", PropertyInfo(Variant::PACKED_STRING_ARRAY, "songNames"), "setSongNames", "getSongNames");
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
    //Temporary to get info from the module
    std::shared_ptr<InstrumentsManager> instManager = std::make_shared<InstrumentsManager>(false);
    std::shared_ptr<Module> currentModule = std::make_shared<Module>();
    io::BinaryContainer container;
    std::vector<uint8_t> byteVector;
    for (int i = 0; i < modData.size(); i++)
    {
        byteVector.push_back(modData[i]);
    }
    std::move(std::begin(byteVector), std::end(byteVector), std::back_inserter(container));
    io::ModuleIO::getInstance().loadModule(container, currentModule, instManager);
    //Get some useful data from the module
    numberOfSongs = currentModule->getSongCount();
    songNames = PackedStringArray();
    for (int i = 0; i < numberOfSongs; i++)
    {
        songNames.append(godot::String(currentModule->getSong(i).getTitle().c_str()));
    }
    //Generate potentially useful hash map of song names to song numbers
    songNameMap = Dictionary();
    for (int i = 0; i < numberOfSongs; i++)
    {
        songNameMap[songNames[i]] = i;
    }
}

PackedByteArray BambooTrackerModule::getModuleData() const
{
    return moduleData;
}

void BambooTrackerModule::setNumberOfSongs(const int64_t numSongs)
{
    return; //Read-only data
}

int64_t BambooTrackerModule::getNumberOfSongs() const
{
    return numberOfSongs;
}

void BambooTrackerModule::setSongNames(const PackedStringArray &songn)
{
    return; //Read-only data
}

PackedStringArray BambooTrackerModule::getSongNames() const
{
    return songNames;
}
