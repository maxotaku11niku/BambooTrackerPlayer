/* BambooTrackerPlayer - Godot GDExtension which plays .BTM files with YM2608 emulation
 * Maxim Hoxha 2024
 * This is Godot-related initialisation stuff
 */

#pragma once

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void InitialiseModule(ModuleInitializationLevel p_level);
void UninitialiseModule(ModuleInitializationLevel p_level);
