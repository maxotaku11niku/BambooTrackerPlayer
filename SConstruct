#!/usr/bin/env python
import os
import sys

env = SConscript("godot-cpp/SConstruct")

# For reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

# tweak this if you want to use different folders, or more folders, to store your source code in.
env.Append(CPPPATH=["src/"])
env.Append(CPPPATH=["src/bt/"])
env.Append(CPPPATH=["src/bt/module"])
env.Append(CPPPATH=["src/bt/io"])
env.Append(CPPPATH=["src/bt/instrument"])
sources = Glob("src/*.cpp") + Glob("src/bt/*.cpp") + Glob("src/bt/*/*.cpp") + Glob("src/bt/chip/*/*.cpp") + Glob("src/bt/chip/*/*.c")

if env["platform"] == "macos":
    library = env.SharedLibrary(
        "demo/bin/libbambootrackerplayer.{}.{}.framework/libbambootrackerplayer.{}.{}".format(
            env["platform"], env["target"], env["platform"], env["target"]
        ),
        source=sources,
    )
else:
    library = env.SharedLibrary(
        "bin/libbambootrackerplayer{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
    )

Default(library)
