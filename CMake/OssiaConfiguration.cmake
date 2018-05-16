# Build settings :
option(OSSIA_SANITIZE "Sanitize build" OFF)
option(OSSIA_TIDY "Use clang-tidy" OFF)
option(OSSIA_STATIC "Make a static build" OFF)
option(OSSIA_COVERAGE "Run code coverage" OFF)
option(OSSIA_EXAMPLES "Build examples" OFF)
option(OSSIA_TESTING "Build tests" OFF)
option(OSSIA_CI "Continuous integration run" OFF)
option(OSSIA_FRAMEWORK "Build an OS X framework" OFF)
option(OSSIA_DISABLE_COTIRE "Disable cotire" OFF)
option(OSSIA_NO_SONAME "Set NO_SONAME property" ON)
option(OSSIA_LTO "Link-time optimizations. Fails on Windows." OFF)
option(OSSIA_OSX_FAT_LIBRARIES "Build 32 and 64 bit fat libraries on OS X" OFF)
option(OSSIA_OSX_RETROCOMPATIBILITY "Build for older OS X versions" OFF)
option(OSSIA_MOST_STATIC "Try to make binaries that are mostly static" OFF)
option(OSSIA_DATAFLOW "Dataflow features" ON)
option(OSSIA_EDITOR "Editor features" ON)
option(OSSIA_SPLIT_DEBUG "Split debug info" ON)
option(OSSIA_NO_DEBUG_INFO "No debug info" OFF)
#option(OSSIA_SUBMODULE_AUTOUPDATE "Auto update submodule" ON)
set(OSSIA_SUBMODULE_AUTOUPDATE ON CACHE BOOL "Auto update submodule")

# Bindings :
option(OSSIA_JAVA "Build JNI bindings" OFF)
option(OSSIA_PD "Build PureData externals" ON)
option(OSSIA_MAX "Build Max/MSP externals" OFF)
option(OSSIA_PYTHON "Build Python bindings" OFF)
option(OSSIA_QT "Build Qt bindings. Implies websocket, serial, http." OFF)
option(OSSIA_C "Build C bindings" OFF)
option(OSSIA_CPP "Build CPP bindings" OFF)
option(OSSIA_UNITY3D "Build Unity3D bindings" OFF)
option(OSSIA_QML "Build QML bindings" OFF)
option(OSSIA_QML_SCORE "Build QML bindings to scoring parts" OFF)

# Protocols :
option(OSSIA_PROTOCOL_AUDIO "Enable Audio protocol" ON)
option(OSSIA_PROTOCOL_MIDI "Enable MIDI protocol" ON)
option(OSSIA_PROTOCOL_OSCQUERY "Enable OSCQuery protocol" ON)
option(OSSIA_PROTOCOL_HTTP "Enable HTTP protocol" ON) # Requires Qt
option(OSSIA_PROTOCOL_WEBSOCKETS "Enable WebSockets protocol" OFF) # Requires Qt
option(OSSIA_PROTOCOL_SERIAL "Enable Serial port protocol" OFF) # Requires Qt
option(OSSIA_PROTOCOL_PHIDGETS "Enable Phidgets protocol" OFF) # Requires Phidgets library
option(OSSIA_PROTOCOL_LEAPMOTION "Enable Leapmotion protocol" OFF) # Requires LeapMotion Orion library
option(OSSIA_DISABLE_QT_PLUGIN "Disable building of a Qt plugin" OFF)
option(OSSIA_DNSSD "Enable DNSSD support" ON)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${PROJECT_SOURCE_DIR}/CMake;${PROJECT_SOURCE_DIR}/CMake/cmake-modules;")

if(OSSIA_SUBMODULE_AUTOUPDATE)
message(STATUS "Update general OSSIA dependencies :")
set(OSSIA_SUBMODULES
    GSL chobo-shl hopscotch-map
    nano-signal-slot brigand whereami
    rapidjson readerwriterqueue websocketpp
    asio variant spdlog fmt
    SmallFunction
    Servus
    bitset2
    concurrentqueue
    exprtk
    flat_hash_map
    frozen
    weakjack
    verdigris
    )

foreach(submodule ${OSSIA_SUBMODULES})
    message(" -> ${OSSIA_3RDPARTY_FOLDER}/${submodule}")
    execute_process(COMMAND git submodule update --init -- ${OSSIA_3RDPARTY_FOLDER}/${submodule}
                    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
endforeach()

execute_process(COMMAND git submodule update --init -- ${PROJECT_SOURCE_DIR}/CMake/cmake-modules
                WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
message(STATUS "...done")
set(OSSIA_SUBMODULE_AUTOUPDATE OFF CACHE BOOL "Auto update submodule" FORCE)
endif()

include(Sanitize)
include(DebugMode)
include(UseGold)
include(LinkerWarnings)

if(OSSIA_QML)
    set(OSSIA_OSX_FAT_LIBRARIES 0)
    set(OSSIA_QT 1)
    set(OSSIA_NO_SONAME 1)
endif()
if(OSSIA_MAX)
    set(OSSIA_OSX_FAT_LIBRARIES 1)
    set(OSSIA_QT 0)
endif()
if(OSSIA_UNITY3D)
    set(OSSIA_OSX_FAT_LIBRARIES 1)
    set(OSSIA_QT 0)
    set(OSSIA_NO_SONAME 1)
endif()
if(OSSIA_OSX_FAT_LIBRARIES)
    set(CMAKE_OSX_ARCHITECTURES "x86_64")
    set(OSSIA_DISABLE_COTIRE 1)
endif()
if(OSSIA_NO_QT)
  set(OSSIA_QT 0)
endif()
if(OSSIA_OSX_RETROCOMPATIBILITY)
  set(CMAKE_OSX_DEPLOYMENT_TARGET 10.9)
endif()

if(OSSIA_MOST_STATIC)
    set(OSSIA_STATIC ON)
    set(CMAKE_LINK_SEARCH_END_STATIC ON)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
endif()

# System detection
include(ProcessorCount)
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-Wmisleading-indentation" SUPPORTS_MISLEADING_INDENT_FLAG)
check_cxx_compiler_flag("-Wl,-z,defs" WL_ZDEFS_SUPPORTED)

if(${CMAKE_SYSTEM_NAME} MATCHES "Android")
  set(LINKER_IS_LLD 0)
  set(LINKER_IS_GOLD 0)
  set(OSSIA_PD 0)
  set(OSSIA_PYTHON 0)
  set(OSSIA_DATAFLOW 0)
  set(OSSIA_PROTOCOL_MIDI 0)
  set(OSSIA_DISABLE_COTIRE 1)
  set(ANDROID 1)
else()
    if(UNIX AND NOT APPLE)
        find_program(LSB_RELEASE lsb_release)
        if(LSB_RELEASE)
          execute_process(COMMAND ${LSB_RELEASE} -i
              OUTPUT_VARIABLE RELEASE_CODENAME
              OUTPUT_STRIP_TRAILING_WHITESPACE
          )
        endif()
    endif()

  # broken in ubuntu 17.10
  if(NOT "${RELEASE_CODENAME}" MATCHES "Ubuntu")
    check_cxx_compiler_flag("-fuse-ld=lld" LLD_LINKER_SUPPORTED)
  endif()
  check_cxx_compiler_flag("-fuse-ld=gold" GOLD_LINKER_SUPPORTED)

  if(OSSIA_SANITIZE AND NOT APPLE)
    set(LLD_LINKER_SUPPORTED 0)
  endif()
  if(LLD_LINKER_SUPPORTED)
    set(LINKER_IS_LLD 1)
  elseif(GOLD_LINKER_SUPPORTED)
    set(LINKER_IS_GOLD 1)
  endif()
endif()

if(OSSIA_SPLIT_DEBUG)
  set(DEBUG_SPLIT_FLAG "-gsplit-dwarf")
  if(NOT APPLE AND NOT MINGW)
  set(GOLD_FLAGS
    -gdwarf-4
#    -Wa,--compress-debug-sections
#    -Wl,--compress-debug-sections=zlib
    -Wl,--dynamic-list-cpp-new
    -Wl,--dynamic-list-cpp-typeinfo
  )
  endif()
endif()
if(${CMAKE_SYSTEM_PROCESSOR} MATCHES ".*arm.*")
    set(OSSIA_ARCHITECTURE arm)
elseif(${CMAKE_SYSTEM_PROCESSOR} MATCHES ".*aarch64.*")
    set(OSSIA_ARCHITECTURE arm)
elseif(${CMAKE_SYSTEM_PROCESSOR} MATCHES ".*64.*")
    set(OSSIA_ARCHITECTURE amd64)
elseif(${CMAKE_SYSTEM_PROCESSOR} MATCHES ".*86.*")
    set(OSSIA_ARCHITECTURE x86)
else()
    message("Could not determine target architecture")
    return()
endif()

# Common setup
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
if(MSVC)
  set(CMAKE_CXX_FLAGS "/std:c++latest ${CMAKE_CXX_FLAGS}")
else()
  set(CMAKE_CXX_FLAGS "-std=c++1z ${CMAKE_CXX_FLAGS}")
endif()

# So that make install after make all_unity does not rebuild everything :
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY True)

# We disable debug infos on OS X on travis because it takes up too much space
if(OSSIA_CI AND APPLE OR OSSIA_NO_DEBUG_INFO)
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g0 ${CMAKE_CXX_FLAGS_DEBUG}")
endif()

if(OSSIA_STATIC)
  set(BUILD_SHARED_LIBS OFF)
  set(OSSIA_FRAMEWORK OFF)
else()
  set(BUILD_SHARED_LIBS ON)
endif()

if(OSSIA_COVERAGE)
  include(CodeCoverage)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_COVERAGE}")
endif()

# Compiler & linker flags
if(MSVC)
    set(CMAKE_CXX_FLAGS "-DNOGDI -DLF_FACESIZE=32 ${CMAKE_CXX_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /PDBCompress /OPT:REF /OPT:ICF")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /PDBCompress /OPT:REF /OPT:ICF")
    set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS_RELEASE} /PDBCompress /OPT:REF /OPT:ICF")

    set(OSSIA_COMPILE_OPTIONS
        "/wd4065" # switch statement contains default but no case labels
        "/wd4068" # pragma mark -
        "/wd4250" # inherits via dominance
        "/wd4251" # DLL stuff
        "/wd4275" # DLL stuff
        "/wd4244" # return : conversion from foo to bar, possible loss of data
        "/wd4800" # conversion from int to bool, performance warning
        "/wd4804" # unsafe mix of const bool <= const int
        "/wd4805" # unsafe mix of const bool == const int
        "/wd4996" # SCL_SECURE_NO_WARNINGS
        "/wd4503" # decorated name length exceeded
        "/wd4305" # argument : truncation from double to float
        "/bigobj"
        ${OSSIA_LINK_OPTIONS}
    )
else()
  if(CMAKE_BUILD_TYPE MATCHES Release)
    set(OSSIA_LINK_OPTIONS
      -ffunction-sections
      -fdata-sections
    )
  endif()

    if(CMAKE_COMPILER_IS_GNUCXX)
      set(OSSIA_LINK_OPTIONS ${OSSIA_LINK_OPTIONS}
        -fvar-tracking-assignments
      )
      if(NOT OSSIA_SPLIT_DEBUG)
        set(OSSIA_LINK_OPTIONS ${OSSIA_LINK_OPTIONS}
          -Wl,--gc-sections
          -Wl,-Bsymbolic-functions
        )


        if(LINKER_IS_GOLD OR LINKER_IS_LLD)
          if(NOT OSSIA_SANITIZE)
              set(OSSIA_LINK_OPTIONS ${OSSIA_LINK_OPTIONS}
                ${DEBUG_SPLIT_FLAG}
                -Wl,--gdb-index
              )
          endif()
        endif()
      endif()
    endif()

    if(LINKER_IS_GOLD)
      set(OSSIA_LINK_OPTIONS ${OSSIA_LINK_OPTIONS} ${GOLD_FLAGS})
    endif()

    if(OSSIA_MOST_STATIC)
      set(OSSIA_LINK_OPTIONS ${OSSIA_LINK_OPTIONS} -static -static-libgcc -static-libstdc++)
    endif()

    if(OSSIA_CI)
        if(CMAKE_COMPILER_IS_GNUCXX)
            set(OSSIA_LINK_OPTIONS ${OSSIA_LINK_OPTIONS} -s)
        endif()
    endif()

    if(UNIX AND NOT CMAKE_COMPILER_IS_GNUCXX)
      set(OSSIA_COMPILE_OPTIONS ${OSSIA_COMPILE_OPTIONS}
        -Wno-gnu-statement-expression
        #-Wweak-vtables
      )
    endif()
    set(OSSIA_COMPILE_OPTIONS
        ${OSSIA_COMPILE_OPTIONS}
        -Wall
        -Wextra
        -Wno-unused-parameter
        -Wno-unknown-pragmas
        -Wno-missing-braces
        -Wno-four-char-constants
        -Wnon-virtual-dtor
        -pedantic
        -Wunused
        -Woverloaded-virtual
        -pipe
        -Werror=return-type
        -Werror=trigraphs
        -Wmissing-field-initializers
        ${OSSIA_LINK_OPTIONS}
    )
    if(NOT CMAKE_COMPILER_IS_GNUCXX)
      set(OSSIA_COMPILE_OPTIONS ${OSSIA_COMPILE_OPTIONS}
        -Wno-cast-align
        -Wno-unused-local-typedef)
    endif()
    if(OSSIA_CI)
        if(NOT CMAKE_COMPILER_IS_GNUCXX)
            set(OSSIA_LINK_OPTIONS ${OSSIA_LINK_OPTIONS} -Wl,-S)
        endif()
    endif()

    if("${SUPPORTS_MISLEADING_INDENT_FLAG}")
        set(OSSIA_COMPILE_OPTIONS ${OSSIA_COMPILE_OPTIONS} -Wmisleading-indentation)
    endif()

#    if(APPLE)
#        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-warn_weak_exports")
#        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-warn_weak_exports")
#        set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,-warn_weak_exports")
#    endif()
endif()

if(OSSIA_LTO)
  setup_lto()
endif()
