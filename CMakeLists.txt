cmake_minimum_required( VERSION 3.8 )

project( EOSIO )
include(CTest) # suppresses DartConfiguration.tcl error
enable_testing()

if ("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr/local")
   message(WARNING "CMAKE_INSTALL_PREFIX is set to /usr/local. This is not recommended.")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")

if (UNIX AND APPLE)
   list(APPEND CMAKE_PREFIX_PATH "/usr/local/opt/llvm@7" "/usr/local/opt/gettext")
endif()

include( GNUInstallDirs )
include( InstallDirectoryPermissions )
include( MASSigning )

set( BLOCKCHAIN_NAME "CODEXIO" )
set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_EXTENSIONS ON )
set( CXX_STANDARD_REQUIRED ON)

set(VERSION_MAJOR 2)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)
set(VERSION_SUFFIX rc2)

if(VERSION_SUFFIX)
    set(VERSION_FULL "force.${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}-${VERSION_SUFFIX}")
else()
    set(VERSION_FULL "force.${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
endif()

set( CLI_CLIENT_EXECUTABLE_NAME cleos )
set( NODE_EXECUTABLE_NAME nodeos )
set( KEY_STORE_EXECUTABLE_NAME keosd )

# http://stackoverflow.com/a/18369825
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.0)
        message(FATAL_ERROR "GCC version must be at least 7.0!")
    endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0)
        message(FATAL_ERROR "Clang version must be at least 5.0!")
    endif()
endif()

if ("${CMAKE_GENERATOR}" STREQUAL "Ninja")
   add_compile_options(-fdiagnostics-color=always)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")
set(BUILD_DOXYGEN FALSE CACHE BOOL "Build doxygen documentation on every make")
set(BUILD_MONGO_DB_PLUGIN FALSE CACHE BOOL "Build mongo database plugin")

# add defaults for openssl
if(APPLE AND UNIX AND "${OPENSSL_ROOT_DIR}" STREQUAL "")
   set(OPENSSL_ROOT_DIR "/usr/local/opt/openssl")
endif()

# WASM runtimes to enable. Each runtime in this list will have:
#  * definition EOSIO_<RUNTIME>_RUNTIME_ENABLED defined in public libchain interface
#  * ctest entries with --runtime
list(APPEND EOSIO_WASM_RUNTIMES wabt)   #always enable wabt; it works everywhere and parts of eosio still assume it's always available
if(CMAKE_SIZEOF_VOID_P EQUAL 8 AND NOT WIN32)
   list(APPEND EOSIO_WASM_RUNTIMES wavm)
   # WAVM requires LLVM, but move the check up here to a central location so that the EosioTester.cmakes
   # can be created with the exact version found
   find_package(LLVM REQUIRED CONFIG)
   if(LLVM_VERSION_MAJOR VERSION_LESS 7 OR LLVM_VERSION_MAJOR VERSION_GREATER 9)
	   message(FATAL_ERROR "EOSIO requires an LLVM version 7.0 to 9.0")
   endif()

   if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux" AND "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
      list(APPEND EOSIO_WASM_RUNTIMES eos-vm-oc)
   endif()
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8 AND NOT WIN32)
   list(APPEND EOSIO_WASM_RUNTIMES eos-vm)
   if(CMAKE_SYSTEM_PROCESSOR STREQUAL x86_64)
      list(APPEND EOSIO_WASM_RUNTIMES eos-vm-jit)
   endif()
endif()

if(UNIX)
  if(APPLE)
    set(whole_archive_flag "-force_load")
    set(no_whole_archive_flag "")
  else()
    set(whole_archive_flag "--whole-archive")
    set(no_whole_archive_flag "--no-whole-archive")
  endif()
else()
  set(whole_archive_flag "--whole-archive")
  set(no_whole_archive_flag "--no-whole-archive")
endif()

set( Boost_USE_STATIC_LIBS ON CACHE STRING "ON or OFF" )
# Most boost deps get implictly picked up via fc, as just about everything links to fc. In addition we pick up
# the pthread dependency through fc.
find_package(Boost 1.67 REQUIRED COMPONENTS program_options unit_test_framework)

if( APPLE AND UNIX )
# Apple Specific Options Here
    message( STATUS "Configuring EOSIO on macOS" )
    set( CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS} -Wall -Wno-deprecated-declarations" )
else()
    # Linux Specific Options Here
    message( STATUS "Configuring EOSIO on Linux" )
    set( CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS} -Wall" )
    if ( FULL_STATIC_BUILD )
      set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc")
    endif ( FULL_STATIC_BUILD )

    if( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )
        if( CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 4.0.0 OR CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.0.0 )
            set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-partial-specialization" )
        endif()
    endif()
endif()

# based on http://www.delorie.com/gnu/docs/gdb/gdb_70.html
# uncomment this line to tell GDB about macros (slows compile times)
# set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -gdwarf-2 -g3" )

set(ENABLE_COVERAGE_TESTING FALSE CACHE BOOL "Build EOSIO for code coverage analysis")

if(ENABLE_COVERAGE_TESTING)
  SET(CMAKE_CXX_FLAGS "--coverage ${CMAKE_CXX_FLAGS}")
  find_program( LCOV_PATH lcov )
  find_program( LLVMCOV_PATH  llvm-cov )
  find_program( GENHTML_PATH NAMES genhtml)
endif()

include(utils)

if ("${CORE_SYMBOL_NAME}" STREQUAL "")
  set( CORE_SYMBOL_NAME "SYS" )
endif()
string(TOUPPER ${CORE_SYMBOL_NAME} CORE_SYMBOL_NAME)

# TO Set Datas
set(ROOT_ACCOUNT "codex")
set(USE_PUB_KEY_LEGACY_PREFIX 1)
set(MAX_PRODUCERS "21")
set(BLOCK_INTERVAL_MS "3000")
set(PRODUCER_REPETITIONS "1")
set(RESOURCE_MODEL "1")
#set(USE_MULTIPLE_VOTE "1")
set(USE_BONUS_TO_VOTE "1")

# set(USE_PUB_KEY_LEGACY_PREFIX 1)

if ("${PUB_KEY_LEGACY_PREFIX}" STREQUAL "")
    set(PUB_KEY_LEGACY_PREFIX "FOSC")
endif()
string(TOUPPER ${PUB_KEY_LEGACY_PREFIX} PUB_KEY_LEGACY_PREFIX)
message( STATUS "Using '${PUB_KEY_LEGACY_PREFIX}' as pubkey legacy prefix" )

if ("${PUB_KEY_BASE_PREFIX}" STREQUAL "")
    set(PUB_KEY_BASE_PREFIX "PUB")
endif()
string(TOUPPER ${PUB_KEY_BASE_PREFIX} PUB_KEY_BASE_PREFIX)
message( STATUS "Using '${PUB_KEY_BASE_PREFIX}' as pubkey common prefix" )

if (USE_PUB_KEY_LEGACY_PREFIX)
    set(USE_PUB_KEY_LEGACY_PREFIX 1)
    message( STATUS "Using legacy prefix for public or private key" )
else()
    set(USE_PUB_KEY_LEGACY_PREFIX 0)
    message( STATUS "Using common prefix for public or private key" )
endif(USE_PUB_KEY_LEGACY_PREFIX)

string(LENGTH ${CORE_SYMBOL_NAME} CORE_SYMBOL_NAME_LENGTH)
if (CORE_SYMBOL_NAME_LENGTH GREATER 7)
  message(FATAL_ERROR "CORE_SYMBOL_NAME length must be between 1 and 7 characters")
endif()

message( STATUS "Using '${CORE_SYMBOL_NAME}' as CORE symbol name" )

if ("${SYSTEM_ACCOUNT_ROOT_KEY}" STREQUAL "")
   set(SYSTEM_ACCOUNT_ROOT_KEY "${PUB_KEY_LEGACY_PREFIX}1111111111111111111111111111111114T1Anm")
endif()

message( STATUS "Using '${SYSTEM_ACCOUNT_ROOT_KEY}' as public key for system root account" )

if ("${CHAIN_INIT_TIMESTAMP}" STREQUAL "")
    set(CHAIN_INIT_TIMESTAMP "2018-12-01T12:00:00")
endif()

message( STATUS "Using '${CHAIN_INIT_TIMESTAMP}' as chain init timestamp" )

if ("${ROOT_ACCOUNT}" STREQUAL "")
    set(ROOT_ACCOUNT "codex")
endif()

message( STATUS "Using '${ROOT_ACCOUNT}' as root account" )

#add contract_config
if ("${FROZEN_DELAY}" STREQUAL "")
  set( FROZEN_DELAY "3 * 24 * 60 * 20" )
endif()
if ("${MAX_PRODUCERS}" STREQUAL "")
  set( MAX_PRODUCERS "21" )
endif()
if ("${BLOCK_INTERVAL_MS}" STREQUAL "")
  set( BLOCK_INTERVAL_MS "3000" )
endif()
if ("${PRODUCER_REPETITIONS}" STREQUAL "")
  set( PRODUCER_REPETITIONS "1" )
endif()
if ("${BLOCK_REWARDS_BP}" STREQUAL "")
  set( BLOCK_REWARDS_BP "10 * 10000" )
endif()
if ("${UPDATE_CYCLE}" STREQUAL "")
  set( UPDATE_CYCLE "100" )
endif()
if ("${RESOURCE_MODEL}" STREQUAL "")
  set( RESOURCE_MODEL "1" )
endif()

# is chain active multiple vote
if(USE_MULTIPLE_VOTE)
    message( STATUS "Using multiple vote for chain" )
    set(IS_ACTIVE_MULTIPLE_VOTE 1)
else()
    set(IS_ACTIVE_MULTIPLE_VOTE 0)
endif(USE_MULTIPLE_VOTE)

# is chain active bonus to vote
if(USE_BONUS_TO_VOTE)
    message( STATUS "Using bonus to vote for chain" )
    set(IS_ACTIVE_BONUS_TO_VOTE 1)
else()
    set(IS_ACTIVE_BONUS_TO_VOTE 0)
endif(USE_BONUS_TO_VOTE)

#include(wasm)
add_subdirectory( libraries )
add_subdirectory( plugins )
add_subdirectory( programs )
add_subdirectory( scripts )
#add_subdirectory( unittests )
#add_subdirectory( tests )
add_subdirectory( tools )

#if (NOT DISABLE_WASM_SPEC_TESTS)
#add_subdirectory( eosio-wasm-spec-tests/generated-tests )
#endif()

install_directory_permissions(DIRECTORY ${CMAKE_INSTALL_FULL_SYSCONFDIR}/eosio)

install(FILES testnet.template DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR}/eosio/launcher)
install_directory_permissions(DIRECTORY ${CMAKE_INSTALL_FULL_SYSCONFDIR}/eosio)
install_directory_permissions(DIRECTORY ${CMAKE_INSTALL_FULL_SYSCONFDIR}/eosio/launcher)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testnet.template ${CMAKE_CURRENT_BINARY_DIR}/etc/eosio/launcher/testnet.template COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/eosio.version.in ${CMAKE_CURRENT_BINARY_DIR}/eosio.version.hpp)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/eosio.version.hpp DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR})

set(EOS_ROOT_DIR ${CMAKE_BINARY_DIR})
configure_file(${CMAKE_SOURCE_DIR}/CMakeModules/eosio-config.cmake.in ${CMAKE_BINARY_DIR}/lib/cmake/eosio/eosio-config.cmake @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/CMakeModules/EosioTesterBuild.cmake.in ${CMAKE_BINARY_DIR}/lib/cmake/eosio/EosioTester.cmake @ONLY)

set(EOS_ROOT_DIR ${CMAKE_INSTALL_PREFIX})
configure_file(${CMAKE_SOURCE_DIR}/CMakeModules/eosio-config.cmake.in ${CMAKE_BINARY_DIR}/modules/eosio-config.cmake @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/modules/eosio-config.cmake DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/cmake/eosio)
configure_file(${CMAKE_SOURCE_DIR}/CMakeModules/EosioTester.cmake.in ${CMAKE_BINARY_DIR}/modules/EosioTester.cmake @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/modules/EosioTester.cmake DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}/cmake/eosio)

configure_file(${CMAKE_SOURCE_DIR}/LICENSE
               ${CMAKE_BINARY_DIR}/licenses/eosio/LICENSE COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/libraries/wabt/LICENSE
               ${CMAKE_BINARY_DIR}/licenses/eosio/LICENSE.wabt COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/libraries/softfloat/COPYING.txt
               ${CMAKE_BINARY_DIR}/licenses/eosio/LICENSE.softfloat COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/libraries/wasm-jit/LICENSE
               ${CMAKE_BINARY_DIR}/licenses/eosio/LICENSE.wavm COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/libraries/fc/secp256k1/upstream/COPYING
               ${CMAKE_BINARY_DIR}/licenses/eosio/LICENSE.secp256k1 COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/libraries/fc/src/network/LICENSE.go
               ${CMAKE_BINARY_DIR}/licenses/eosio/LICENSE.go COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/libraries/yubihsm/LICENSE
               ${CMAKE_BINARY_DIR}/licenses/eosio/LICENSE.yubihsm COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/libraries/eos-vm/LICENSE
               ${CMAKE_BINARY_DIR}/licenses/eosio/LICENSE.eos-vm COPYONLY)

install(FILES LICENSE DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/licenses/eosio/ COMPONENT base)
install(FILES libraries/wabt/LICENSE DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/licenses/eosio/ RENAME LICENSE.wabt COMPONENT base)
install(FILES libraries/softfloat/COPYING.txt DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/licenses/eosio/ RENAME LICENSE.softfloat COMPONENT base)
install(FILES libraries/wasm-jit/LICENSE DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/licenses/eosio/ RENAME LICENSE.wavm COMPONENT base)
install(FILES libraries/fc/secp256k1/upstream/COPYING DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/licenses/eosio/ RENAME LICENSE.secp256k1 COMPONENT base)
install(FILES libraries/fc/src/network/LICENSE.go DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/licenses/eosio/ COMPONENT base)
install(FILES libraries/yubihsm/LICENSE DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/licenses/eosio/ RENAME LICENSE.yubihsm COMPONENT base)
install(FILES libraries/eos-vm/LICENSE DESTINATION ${CMAKE_INSTALL_FULL_DATAROOTDIR}/licenses/eosio/ RENAME LICENSE.eos-vm COMPONENT base)

add_custom_target(base-install
  COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}"
  COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=base -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
  USES_TERMINAL
)

get_property(_CTEST_CUSTOM_TESTS_IGNORE GLOBAL PROPERTY CTEST_CUSTOM_TESTS_IGNORE)
file(WRITE "${CMAKE_BINARY_DIR}/CTestCustom.cmake" "SET(CTEST_CUSTOM_TESTS_IGNORE ${_CTEST_CUSTOM_TESTS_IGNORE})")

include(package)
include(doxygen)
