# -*- python -*-

env = Environment()

env["CPPFLAGS"] = ["-O2", "-Wall", "-Werror", "-Wshadow", "-std=c++11"]
env["CPPPATH"] = [Dir("../libjson-rpc-cpp-lib/include"), Dir("src")]
env["LIBPATH"] = [Dir("../libjson-rpc-cpp-lib/lib")]
env["LINKFLAGS"] = ["-pthread", "-rdynamic"]

env["CXXCOMSTR"] = "Compiling $SOURCE"
env["ARCOMSTR"] = "Creating $TARGET"
env["RANLIBCOMSTR"] = "Indexing $TARGET"
env["LINKCOMSTR"] = "Linking $TARGET"

env["PLUGINS"] = []

########################################################################################################################
# application library

sources = [
    "output/alsa.cpp",
    "codec/basecodec.cpp",
    "codec/mp3.cpp",
    "library/musiclibrary.cpp",
    "library/scanner.cpp",
    "library/metaparser.cpp",
    "library/sqlitestorage.cpp",
    "player/player.cpp",
    "player/decoder.cpp",
    "player/controller.cpp",
    "player/fifo.cpp",
    "player/queue.cpp",
    "thread/thread.cpp",
    "utils/stringutils.cpp",
    "config/parser.cpp",
    "filter/volume.cpp",
    "plugin/pluginmanager.cpp"
]

zep_lib = env.StaticLibrary(
    "zeppelin",
    source = ["src/%s" % s for s in sources]
)

########################################################################################################################
# main application

# TODO: jsonrpc dependency should be removed because the core application is not using the RPC part of it, just the
#       json parser for the configuration
zep = env.Program(
    "zeppelin",
    source = ["src/main.cpp"] + zep_lib,
    LIBS = ["asound", "mpg123", "sqlite3", "jsonrpc", "dl"]
)

########################################################################################################################
# plugins

SConscript(dirs = ["plugins"], exports = ["env"])

# define the defualt target
Default([zep] + env["PLUGINS"])

########################################################################################################################
# testing

tests = [
    "fifo.cpp",
    "queue.cpp"
]

env.Program(
    "unit_test",
    source = ["tst/%s" % t for t in tests] + ["tst/main.cpp"] + zep_lib,
    LIBS = ["boost_unit_test_framework"]
)
