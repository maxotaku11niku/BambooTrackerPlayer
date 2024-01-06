# BambooTrackerPlayer - Godot GDExtension which plays .BTM files with YM2608 emulation
# Copyright (c) 2024 Maxim Hoxha

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# Actual importer plugin code

@tool
extends EditorImportPlugin

func _get_importer_name():
	return "mhoxha.bambootrackerimporter"

func _get_visible_name():
	return "Bamboo Tracker Module"

func _get_recognized_extensions():
	return ["btm"]

func _get_save_extension():
	return "res"

func _get_resource_type():
	return "BambooTrackerModule"

func _get_preset_count():
	return 1

func _get_preset_name(preset_index):
	return "Default"

func _get_import_options(path, preset_index):
	return []

func _get_option_visibility(path, option_name, options):
	return true

func _get_import_order():
	return 0

func _get_priority():
	return 1.0

func _import(source_file, save_path, options, r_platform_variants, r_gen_files):
	var mod: BambooTrackerModule = BambooTrackerModule.new()
	mod.setModuleData(FileAccess.get_file_as_bytes(source_file))
	return ResourceSaver.save(mod, "%s.%s" % [save_path, _get_save_extension()], ResourceSaver.FLAG_COMPRESS)
