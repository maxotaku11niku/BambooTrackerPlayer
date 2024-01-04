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
	var data := PackedByteArray()
	mod.setModuleData(FileAccess.get_file_as_bytes(source_file))
	return ResourceSaver.save(mod, "%s.%s" % [save_path, _get_save_extension()], ResourceSaver.FLAG_COMPRESS)
