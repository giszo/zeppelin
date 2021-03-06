# This file is part of the Zeppelin music player project.
# Copyright (c) 2013-2014 Zoltan Kovacs, Lajos Santa
# See http://zeppelin-player.com for more details.

import os

########################################################################################################################
# command line options

CODECS = ['mp3', 'flac', 'ogg', 'wavpack', 'monkeysaudio']

vars = Variables()
vars.Add(PathVariable('CXX', 'path of the C++ compiler used for building', None))
vars.Add(BoolVariable('DEBUG', 'compile a debuggable executable', 0))
vars.Add(PathVariable('PREFIX', 'prefix used to install files', '/usr'))
vars.Add(PathVariable('JSONCPP', 'path of jsoncpp library', None))
vars.Add(PathVariable('MPG123', 'path of mpg123 library', None))
vars.Add(BoolVariable('COVERAGE', 'set to 1 to measure coverage', 0))
vars.Add(ListVariable('CODECS', 'list of compiled codecs', CODECS, CODECS))
vars.Add(EnumVariable('OUTPUT', 'name of the output driver to use', 'alsa', ('alsa', 'pulseaudio')))

env = Environment(variables = vars)

env["CPPPATH"] = [Dir("include"), Dir("src")]
env["LIBPATH"] = []
env["CPPFLAGS"] = ["-Wall", "-Werror", "-Wshadow", "-std=c++11", "-pthread"]
env["CPPDEFINES"] = []
env["LINKFLAGS"] = ["-pthread", "-rdynamic"]
env["LIBS"] = ["samplerate", "sqlite3", "jsoncpp"]

if env["COVERAGE"] :
    env["CPPFLAGS"] += ["-coverage"]
    env["LINKFLAGS"] += ["-coverage"]

if env["DEBUG"] :
    env["CPPFLAGS"] += ["-g"]
else :
    env["CPPFLAGS"] += ["-O2"]

for lib in ["JSONCPP", "MPG123"] :
    if lib in env :
        env["CPPPATH"] += ["%s/include" % env[lib]]
        env["LIBPATH"] += ["%s/lib" % env[lib]]

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
    "codec/codecmanager.cpp",
    "library/musiclibrary.cpp",
    "library/scanner.cpp",
    "library/metaparser.cpp",
    "library/sqlitestorage.cpp",
    "library/file.cpp",
    "library/directory.cpp",
    "library/artist.cpp",
    "library/album.cpp",
    "library/metadata.cpp",
    "library/vorbismetadata.cpp",
    "library/picture.cpp",
    "player/player.cpp",
    "player/decoder.cpp",
    "player/controller.cpp",
    "player/fifo.cpp",
    "player/queue.cpp",
    "player/format.cpp",
    "player/eventlistenerproxy.cpp",
    "thread/thread.cpp",
    "thread/condition.cpp",
    "utils/signalhandler.cpp",
    "utils/pidfile.cpp",
    "config/parser.cpp",
    "filter/basefilter.cpp",
    "filter/volume.cpp",
    "filter/resample.cpp",
    "plugin/pluginmanager.cpp"
]

# handle codec list
for codec in env["CODECS"] :
    if codec == "mp3" :
        env["LIBS"] += ["mpg123"]
        sources += ["codec/mp3.cpp"]
    elif codec == "flac" :
        env["LIBS"] += ["FLAC"]
        sources += ["codec/flac.cpp"]
    elif codec == "ogg" :
        env["LIBS"] += ["vorbisfile"]
        sources += ["codec/vorbis.cpp"]
    elif codec == "wavpack" :
        env["LIBS"] += ["wavpack"]
        sources += ["codec/wavpack.cpp"]
    elif codec == "monkeysaudio" :
        env["LIBS"] += ["mac"]
        sources += ["codec/mac.cpp"]

    env["CPPDEFINES"] += [{"HAVE_%s" % codec.upper(): 1}]

# handle output driver
if env["OUTPUT"] == "alsa" :
    env["LIBS"] += ["asound"]
    sources += ["output/alsa.cpp"]
elif env["OUTPUT"] == "pulseaudio" :
    env["LIBS"] += ["pulse"]
    sources += ["output/pulseaudio.cpp"]

env["CPPDEFINES"] += [{"HAVE_%s" % env["OUTPUT"].upper(): 1}]

zep_lib = env.StaticLibrary(
    "zeppelin",
    source = ["src/%s" % s for s in sources]
)

########################################################################################################################
# main application

zep = env.Program(
    "zeppelin",
    source = ["src/main.cpp"] + zep_lib,
    LIBS = env["LIBS"] + ["dl", "boost_locale", "boost_program_options"]
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

env.Alias("install", env.Install("$PREFIX/bin", zep))

for root, dirs, files in os.walk(str(Dir("include/zeppelin"))) :
    for fn in files :
        env.Alias("install", env.InstallAs("$PREFIX/%s/%s" % (root, fn), "%s/%s" % (root, fn)))

########################################################################################################################
# release

SConscript(dirs = ["release"], exports = ["env"])
