# @file
# @brief BU Libraries definitions
# @internal
#
# @copyright (C) 2020 Melexis N.V.
#
# Melexis N.V. is supplying this code for use with Melexis N.V. processor based microcontrollers only.
#
# THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY,
# INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.  MELEXIS N.V. SHALL NOT IN ANY CIRCUMSTANCES,
# BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
#
# @endinternal
#
# @ingroup application
#
# @details Extension Makefile to the application Makefile.
#          It contains list of BU Libraries available for the application.
#          This Makefile requires the application to set some variables:
#          - BU_LIBS : list of libraries to be included for the application
#          - BU_LIBS_DIR : path to the directory where this file is stored
#          It returns next variables to be used by the application Makefile
#          - VPATH : VPATH is extended with the source file directories
#          - BU_LIBS_SRCS : list of source files to include in the build
#          - BU_LIBS_INC_DIRS : list of include directories
#

BU_LIBS_FILTERED = $(filter-out buffers,$(BU_LIBS))

# Get list of all the derived libraries
-include $(patsubst %,$(BU_LIBS_DIR)/%/derivedlibs.mk,$(BU_LIBS_FILTERED))

BU_LIBS_TOTAL = $(sort $(BU_LIBS_FILTERED) $(BU_LIBS_DERIVED))

# Getting definitions from each application module
include $(patsubst %,$(BU_LIBS_DIR)/%/srclist.mk,$(BU_LIBS_TOTAL))

# SWCC buffers submodule workaround
ifeq (buffers,$(findstring buffers, $(BU_LIBS)))
VPATH += $(BU_LIBS_DIR)/buffers/src
BU_LIBS_INC_DIRS += $(BU_LIBS_DIR)/buffers/inc
BU_LIBS_SRCS += $(notdir $(wildcard $(BU_LIBS_DIR)/buffers/src/*.c))
endif
