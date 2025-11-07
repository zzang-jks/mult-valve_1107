# @file Makefile
# @brief Melexis bu-act libraries project
# @internal
#
# @copyright (C) 2017 Melexis N.V.
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
# @details
# Makefile to verify the libraries
#

# project folders
ROOT_DIR  = .
UNCR_DIR  = $(ROOT_DIR)/_verification/uncrustify
UTEST_DIR = $(ROOT_DIR)/_verification/unit_test
DOC_DIR   = $(ROOT_DIR)/_doc
PRODUCT   ?= 81340
PROFILE   ?= 81340B01

EXT_LIBRARIES = buffers

# list of all available libraries
LIBRARIES ?= $(filter-out $(EXT_LIBRARIES),$(filter-out _%,$(patsubst %/,%,$(sort $(dir $(wildcard */))))))

# files
SRCS_UNCRUSTIFY  = $(foreach lib,$(LIBRARIES),$(wildcard $(lib)/src/*.c))
SRCS_UNCRUSTIFY += $(foreach lib,$(LIBRARIES),$(wildcard $(lib)/src/*.h))
SRCS_UNCRUSTIFY += $(foreach lib,$(LIBRARIES),$(wildcard $(lib)/products/$(PRODUCT)/src/*.c))
SRCS_UNCRUSTIFY += $(foreach lib,$(LIBRARIES),$(wildcard $(lib)/products/$(PRODUCT)/src/*.h))
SRCS_UNCRUSTIFY += $(foreach lib,$(LIBRARIES),$(wildcard $(lib)/unit_test/*.c))
SRCS_UNCRUSTIFY += $(foreach lib,$(LIBRARIES),$(wildcard $(lib)/unit_test/*.h))

# tools
UNCRUSTIFY ?= uncrustify
ECHO ?= echo

.PHONY: uncrustify
uncrustify:
	$(UNCRUSTIFY) -c $(UNCR_DIR)/uncrustify.cfg --replace --no-backup -l C $(SRCS_UNCRUSTIFY)

.PHONY: uncrustify-check
uncrustify-check:
	$(UNCRUSTIFY) -c $(UNCR_DIR)/uncrustify.cfg --check -l C $(SRCS_UNCRUSTIFY)

.PHONY: utest
utest:
	'$(MAKE)' --directory=$(UTEST_DIR) utest

.PHONY: clean_utest
clean_utest:
	'$(MAKE)' --directory=$(UTEST_DIR) clean

.PHONY: doxy
doxy:
	'$(MAKE)' --silent --directory=$(DOC_DIR) doxy LIBRARIES="$(LIBRARIES)"

