# -*- python -*-

env = Environment()

env["CPPFLAGS"] = ["-O2", "-Wall", "-Werror", "-Wshadow", "-std=c++11"]
env["CPPPATH"] = ["../libjson-rpc-cpp-lib/include", "src"]
env["LIBPATH"] = ["../libjson-rpc-cpp-lib/lib"]
env["LINKFLAGS"] = ["-pthread"]

env["CXXCOMSTR"] = "Compiling $SOURCE"
env["ARCOMSTR"] = "Creating $TARGET"
env["RANLIBCOMSTR"] = "Indexing $TARGET"
env["LINKCOMSTR"] = "Linking $TARGET"

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
    "rpc/server.cpp",
    "player/player.cpp",
    "player/decoder.cpp",
    "player/controller.cpp",
    "player/fifo.cpp",
    "thread/thread.cpp",
    "utils/stringutils.cpp",
    "config/parser.cpp",
    "filter/volume.cpp"
]

woms_lib = env.StaticLibrary(
    "woms",
    source = ["src/%s" % s for s in sources]
)

########################################################################################################################
# main application

woms = env.Program(
    "woms",
    source = ["src/main.cpp"] + woms_lib,
    LIBS = ["asound", "mpg123", "sqlite3", "jsonrpc"]
)

# define the defualt target
Default(woms)

########################################################################################################################
# testing

tests = [
    "fifo.cpp"
]

env.Program(
    "unit_test",
    source = ["tst/%s" % t for t in tests] + ["tst/main.cpp"] + woms_lib,
    LIBS = ["boost_unit_test_framework"]
)
