# vim: set ai ts=4 expandtab:
#
# Redis-Wrenches ~ Copyright 2021 NSONE, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(wrenches
        LANGUAGES C
        VERSION 1.0.0)

set(CMAKE_C_STANDARD 11)


message(STATUS "CMake version is: ${CMAKE_VERSION}")
message(STATUS "The host system is: ${CMAKE_HOST_SYSTEM}.")
message(STATUS "Project base dir is: ${CMAKE_SOURCE_DIR}")
message(STATUS "Build type is: ${CMAKE_BUILD_TYPE}")

# For DEBUG builds.
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Disable all optimizations.
    add_compile_options("-O0")
endif ()

# Copying details from SDK example Makefile.
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(VALID_REDIS_TARGETS 6 5 4 CACHE INTERNAL "major versions of Redis we can target")
if (NOT DEFINED REDIS_TARGET_VERSION)
    set(REDIS_TARGET_VERSION 6)
    message(STATUS "Assuming Redis major version = 6")
elseif (REDIS_TARGET_VERSION IN_LIST VALID_REDIS_TARGETS)
    message(STATUS "Setting Redis major version = ${REDIS_TARGET_VERSION}")
else ()
    message(FATAL_ERROR "Invalid Redis major version: ${REDIS_TARGET_VERSION}. Set REDIS_TARGET_VERSION to one of ${VALID_REDIS_TARGETS}")
endif ()

# ::-------------------------------------------------------------------------::

add_library(rmutil STATIC
        deps/RedisModulesSDK/rmutil/alloc.c
        deps/RedisModulesSDK/rmutil/heap.c
        deps/RedisModulesSDK/rmutil/periodic.c
        deps/RedisModulesSDK/rmutil/priority_queue.c
        deps/RedisModulesSDK/rmutil/sds.c
        deps/RedisModulesSDK/rmutil/strings.c
        deps/RedisModulesSDK/rmutil/util.c
        deps/RedisModulesSDK/rmutil/vector.c
        )

set_property(TARGET rmutil PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(rmutil PRIVATE deps/RedisModulesSDK/6.0)

# ::-------------------------------------------------------------------------::

add_library(wrenches SHARED
        src/wrenches_main.c
        )

set_property(TARGET wrenches PROPERTY POSITION_INDEPENDENT_CODE ON)

target_include_directories(wrenches SYSTEM PUBLIC
        deps/RedisModulesSDK/${REDIS_TARGET_VERSION}.0
        # This is to root rmutil/ instead of including it directly.
        deps/RedisModulesSDK/
        )

target_compile_options(wrenches PRIVATE
        -fno-common
        -Wall
        -Wextra
        )

target_compile_definitions(wrenches PUBLIC REDIS_VERSION=${REDIS_TARGET_VERSION})
target_link_libraries(wrenches rmutil)
