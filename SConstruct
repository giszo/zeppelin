# -*- python -*-

env = Environment()

env["CPPFLAGS"] = ["-O2", "-Wall", "-Werror", "-Wshadow", "-std=c++11"]
env["CPPPATH"] = ["../libjson-rpc-cpp-lib/include", "src"]
env["LIBPATH"] = ["../libjson-rpc-cpp-lib/lib"]
env["LINKFLAGS"] = ["-pthread"]

########################################################################################################################
# application library

sources = [
    "output/alsa.cpp",
    "codec/mp3.cpp",
    "library/musiclibrary.cpp",
    "library/worker.cpp",
    "library/scandirectory.cpp",
    "rpc/server.cpp",
    "player/player.cpp",
    "player/decoder.cpp",
    "player/controller.cpp",
    "buffer/ringbuffer.cpp",
    "thread/thread.cpp"
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
    "ringbuffer.cpp"
]

env.Program(
    "unit_test",
    source = ["tst/%s" % t for t in tests] + ["tst/main.cpp"] + woms_lib,
    LIBS = ["boost_unit_test_framework"]
)
