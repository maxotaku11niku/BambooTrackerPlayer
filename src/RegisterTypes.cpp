/* BambooTrackerPlayer - Godot GDExtension which plays .BTM files with YM2608 emulation
 * Maxim Hoxha 2024
 * This is Godot-related initialisation stuff
 */

#include "RegisterTypes.h"
#include "BambooTrackerPlayer.h"
#include "BambooTrackerModule.h"
#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

void InitialiseModule(ModuleInitializationLevel p_level)
{
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
    {
        return;
    }

    ClassDB::register_class<BambooTrackerModule>();
    ClassDB::register_class<BambooTrackerPlayer>();
}

void UninitialiseModule(ModuleInitializationLevel p_level)
{
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
    {
        return;
    }
}

extern "C" {
GDExtensionBool GDE_EXPORT libraryinit(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization)
{
    godot::GDExtensionBinding::InitObject initObj(p_get_proc_address, p_library, r_initialization);

    initObj.register_initializer(InitialiseModule);
    initObj.register_terminator(UninitialiseModule);
    initObj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

    return initObj.init();
}
}
