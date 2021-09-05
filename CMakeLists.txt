cmake_minimum_required(VERSION 2.8)
project(simpleIC3 C CXX)
include (GNUInstallDirs)
include_directories("src/")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Werror=return-type")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3  ")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG -O3") #it is _critically_ important that release builds specify -DNDEBUG!
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}  -DNDEBUG -O2") #it is _critically_ important that release builds specify -DNDEBUG!
set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -DNDEBUG") #it is _critically_ important that release builds specify -DNDEBUG!


set (CMAKE_CXX_FLAGS_SANITIZE "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -fPIC -fno-omit-frame-pointer -fsanitize=address -fuse-ld=gold")
set (CMAKE_LINKER_FLAGS_SANITIZE "${CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO} -fno-omit-frame-pointer -fsanitize=address  -fuse-ld=gold")

set (CMAKE_CXX_FLAGS_SANITIZEUB "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -fPIC -fno-omit-frame-pointer -fsanitize=undefined -fuse-ld=gold")
set (CMAKE_LINKER_FLAGS_SANITIZEUB "${CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO} -fno-omit-frame-pointer -fsanitize=undefined  -fuse-ld=gold")


set( CMAKE_VERBOSE_MAKEFILE on )
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified.\nUse cmake -DCMAKE_BUILD_TYPE=Debug to select Debug release.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"  "MinSizeRel" "RelWithDebInfo" "Sanitize")
endif()



set(SOURCE_FILES

        src/simpleIC3/util/VHeap.h
        src/simpleIC3/util/Bitset.h
        src/aiger/aiger.c
        src/aiger/aiger.h
        src/simpleIC3/ic3/Cone.cpp
        src/simpleIC3/ic3/Cone.h
        src/simpleIC3/ic3/ic3.h
        src/simpleIC3/ic3/ic3config.cpp
        src/simpleIC3/ic3/ic3config.h
        src/simpleIC3/ic3/ic3Types.cpp
        src/simpleIC3/ic3/ic3Types.h
        src/simpleIC3/ic3/Ternary.cpp
        src/simpleIC3/ic3/Ternary.h
        src/simpleIC3/aiglib/aiglib.h
        src/simpleIC3/aiglib/aigtocnf.h
        src/simpleIC3/aiglib/Word.h
        src/simpleIC3/main.cpp

        src/minisat/utils/Options.cc
        src/minisat/utils/System.cc
        src/minisat/utils/ParseUtils.h
        src/minisat/core/Solver.cc
        src/minisat/core/Dimacs.h
        src/minisat/core/Solver.h
        src/minisat/core/SolverTypes.h
        src/minisat/simp/SimpSolver.cc
        src/minisat/mtl/Alg.h
        src/minisat/mtl/Alloc.h
        src/minisat/mtl/Heap.h
        src/minisat/mtl/IntMap.h
        src/minisat/mtl/IntTypes.h
        src/minisat/mtl/Map.h
        src/minisat/mtl/Queue.h
        src/minisat/mtl/Rnd.h
        src/minisat/mtl/Sort.h
        src/minisat/mtl/Vec.h
        src/minisat/mtl/XAlloc.h
        )

add_executable(simpleIC3 ${SOURCE_FILES}         src/simpleIC3/main.cpp)


