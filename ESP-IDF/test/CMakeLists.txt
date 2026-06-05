cmake_minimum_required(VERSION 3.16)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(befine_tests)

# Include component under test
set(EXTRA_COMPONENT_DIRS "../components/dsp" "../components/session")
