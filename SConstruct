# -*- python -*-

import os

vars = Variables()
vars.Add(PathVariable('PREFIX', 'prefix used to install files', '/'))
vars.Add(BoolVariable('COVERAGE', 'set to 1 to measure coverage', 0))

env = Environment(variables = vars)

env["CPPFLAGS"] = ["-Wall", "-Werror", "-Wshadow", "-std=c++11", "-pthread"]
env["CPPPATH"] = [Dir("include"), Dir("src")]
env["LINKFLAGS"] = ["-pthread", "-rdynamic"]

if env["COVERAGE"] :
    env["CPPFLAGS"] += ["-coverage"]
    env["LINKFLAGS"] += ["-coverage"]
else :
    env["CPPFLAGS"] += ["-O2"]

env["CXXCOMSTR"] = "Compiling $SOURCE"
env["SHCXXCOMSTR"] = "Compiling $SOURCE"
env["ARCOMSTR"] = "Creating $TARGET"
env["RANLIBCOMSTR"] = "Indexing $TARGET"
env["LINKCOMSTR"] = "Linking $TARGET"
env["SHLINKCOMSTR"] = "Linking $TARGET"

########################################################################################################################
# application library

sources = [
    "logger.cpp",
    "output/baseoutput.cpp",
    "output/alsa.cpp",
    "codec/codecmanager.cpp",
    "codec/mp3.cpp",
    "codec/flac.cpp",
    "codec/vorbis.cpp",
    "codec/wavpack.cpp",
    "codec/mac.cpp",
    "codec/metadata.cpp",
    "library/musiclibrary.cpp",
    "library/scanner.cpp",
    "library/metaparser.cpp",
    "library/sqlitestorage.cpp",
    "library/file.cpp",
    "library/directory.cpp",
    "library/artist.cpp",
    "library/album.cpp",
    "player/player.cpp",
    "player/decoder.cpp",
    "player/controller.cpp",
    "player/fifo.cpp",
    "player/queue.cpp",
    "player/format.cpp",
    "thread/thread.cpp",
    "thread/condition.cpp",
    "utils/stringutils.cpp",
    "utils/signalhandler.cpp",
    "utils/pidfile.cpp",
    "config/parser.cpp",
    "filter/basefilter.cpp",
    "filter/volume.cpp",
    "filter/resample.cpp",
    "plugin/pluginmanager.cpp"
]

zep_lib = env.StaticLibrary(
    "zeppelin",
    source = ["src/%s" % s for s in sources]
)

########################################################################################################################
# main application

zep = env.Program(
    "zeppelin",
    source = ["src/main.cpp"] + zep_lib,
    LIBS = ["asound", "mpg123", "FLAC", "vorbisfile", "wavpack", "mac", "samplerate", "sqlite3", "jsoncpp", "dl", "boost_locale", "boost_program_options"]
)

# define the defualt target
Default(zep)

########################################################################################################################
# testing

tests = [
    "fifo.cpp",
    "queue.cpp",
    "format.cpp",
    "controller.cpp"
]

env.Program(
    "unit_test",
    source = ["tst/%s" % t for t in tests] + ["tst/main.cpp"] + zep_lib,
    LIBS = ["jsoncpp", "samplerate", "boost_unit_test_framework"]
)

########################################################################################################################
# install

env.Alias("install", env.Install("$PREFIX/usr/bin", zep))

for root, dirs, files in os.walk(str(Dir("include/zeppelin"))) :
    for fn in files :
        env.Alias("install", env.InstallAs("$PREFIX/usr/%s/%s" % (root, fn), "%s/%s" % (root, fn)))

########################################################################################################################
# release

SConscript(dirs = ["release"], exports = ["env"])
