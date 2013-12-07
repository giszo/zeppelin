# -*- python -*-

env = Environment()

env["CPPFLAGS"] = ["-O2", "-Wall", "-std=c++11"]
env["CPPPATH"] = ["../libjson-rpc-cpp-lib/include"]
env["LIBPATH"] = ["../libjson-rpc-cpp-lib/lib"]
env["LINKFLAGS"] = ["-pthread"]

sources = [
    "main.cpp",
    "output/alsa.cpp",
    "codec/mp3.cpp",
    "library/musiclibrary.cpp",
    "library/worker.cpp",
    "library/scandirectory.cpp",
    "rpc/server.cpp",
    "player/player.cpp",
    "player/queue.cpp"
]

env.Program(
    "woms",
    source = ["src/%s" % s for s in sources],
    LIBS = ["asound", "mpg123", "sqlite3", "jsonrpc"]
)
