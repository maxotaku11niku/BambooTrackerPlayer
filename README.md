# BambooTrackerPlayer

**GDExtension that can play [Bamboo Tracker](https://github.com/BambooTracker/BambooTracker) modules**

## About

**BambooTrackerPlayer** uses native code from Bamboo Tracker so that Godot games can import and play Bamboo Tracker modules. This means you can include .btm modules directly with your game if you wish to use them to make music for whatever reason, saving quite a lot of space. Most modern devices should be powerful enough to emulate a YM2608 in real time, but you can always double check with a profiler to see how demanding it is.

## Requirements

- Godot 4.2 or higher
- Windows, Linux or Android for prebuilt binaries, build this repository yourself to generate a macOS version.

## Basic usage

Download the latest package, and extract its contents. The `.dll` and `.so` files are native libraries that contain both the main `BambooTrackerPlayer` node class and the `BambooTrackerModule` resource class. `importerplugin` contains an editor plugin that imports `.btm` files as `BambooTrackerModule`s, which a `BambooTrackerPlayer` can use. I recommend making a `BambooTrackerPlayer` an autoload, or rather a minimal stub script like the following that inherits from it:

```
extends BambooTrackerPlayer

func _ready() -> void:
    module = load("res://path/to/module.btm")
    PlayNewModule()
```

I recommend putting the native libraries in `res://bin/`, although you can always edit `bambootrackerplayer.gdextension` if you don't like this convention.

Use `PlaySong(songNum: int, forceRestart: bool = false)` or `PlaySongFromName(songName: String, forceRestart: bool = false)` to play songs from the module, and `StopSong()` to stop playing the current song. More advanced functionality will come later.

## Building

Follow [Godot's guide](https://docs.godotengine.org/en/4.2/tutorials/scripting/gdextension/gdextension_cpp_example.html) on how to build GDExtensions, since there's nothing hugely special about building this GDExtension.
