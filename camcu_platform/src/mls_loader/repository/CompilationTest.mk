#
# Copyright (C) 2017 Melexis N.V.
#
# Software Platform
#


#------------------------------------------------------------------------------
# General block
#------------------------------------------------------------------------------

#--- Toolchain and utilities --------------------------------------------------

# Describe all possible variants for the build options (prefix "OPT_")

# GCC flags
OPT_OPTIMIZATION = -Os -O0 -O1 -O2 -O3 #-Ofast -Og

# NOTE: the OPT_INSTSET need to specified externally depending on the compiler version
#OPT_INSTSET =

# Project compiling options
OPT_ML_MLX4_SLEEP_MODE = 1 0
OPT_ML_MLX4_INIT_TIMEOUT = 1 0
OPT_FPLL = 12000 18000 20000 24000 25000 28000 30000 32000
OPT_ML_HAS_MLX4_CST_TBL = 1 0
OPT_ML_HAS_MLX4_CODE = 1 0
OPT_ML_BAUDRATE = 9600 10417 19200
OPT_ML_FAST_BAUDRATE = 25000 50000 75000 100000 125000 150000
OPT_ML_MLX4_INIT_TIMEOUT_MS = 1 2 4 8 16 32
OPT_ML_FLASH_PAGE_BYTE_SIZE = 64 128 256
OPT_ML_HAS_LIN_PIN_LOADER = 1 0
OPT_ML_HAS_LIN_AUTOADDRESSING = 1 0
OPT_LDT_HAS_ACTIVATE_P2TRANSFER_TIMEOUT = 1 0
OPT_LDT_HAS_CONSECUTIVE_FRAMES = 1 0
OPT_LDT_HAS_TRANSPORT_LAYER = 1 0
OPT_ML_HAS_LIN_EVENT_TABLE_IN_RAM = 1 0
OPT_LDT_DEFAULT_NAD = 1 10 100

# Specify default values
# Project compiling options
SEL_ML_MLX4_SLEEP_MODE = 0
SEL_ML_MLX4_INIT_TIMEOUT = 1
SEL_FPLL = 25000
SEL_ML_HAS_MLX4_CST_TBL = 0
SEL_ML_HAS_MLX4_CODE = 0
SEL_ML_BAUDRATE = 19200
SEL_ML_FAST_BAUDRATE = 100000
SEL_ML_MLX4_INIT_TIMEOUT_MS = 32
SEL_ML_FLASH_PAGE_BYTE_SIZE = 128
SEL_ML_HAS_LIN_PIN_LOADER = 1
SEL_ML_HAS_LIN_AUTOADDRESSING = 1
SEL_LDT_HAS_ACTIVATE_P2TRANSFER_TIMEOUT = 1
SEL_LDT_HAS_CONSECUTIVE_FRAMES = 1
SEL_LDT_HAS_TRANSPORT_LAYER = 1
SEL_ML_HAS_LIN_EVENT_TABLE_IN_RAM = 0
SEL_LDT_DEFAULT_NAD = 1

## Filter out default values as their SEL_ values are always specifed for the build
#OPT_BUILD_VARS_LIST_FILTERED = $(filter-out <opt_variables>, $(OPT_BUILD_VARS_LIST))

## Optional PLTF_DEFS (the default values are filtered-out)
#PLTF_DEFS += $(foreach selection,$(OPT_BUILD_VARS_LIST_FILTERED),\
#			 $(patsubst OPT_%,%,$(selection))="$($(patsubst OPT_%,SEL_%,$(selection)))")

CPPFLAGS += $(foreach folder,$(INC_DIRS),-I$(folder))
CPPFLAGS += $(foreach define,$(PLTF_DEFS),-D$(define))

# Provide standard goals
.DEFAULT_GOAL := all

#
# Rules
#

# MLX LIN API library build
.PHONY: all
all:
	@$(MAKE) all $(MLX_LIN_API_CONFIGS) STANDALONE_BUILD=1

.PHONY: clean
clean:
	@$(MAKE) clean $(MLX_LIN_API_CONFIGS)

ifndef GET_VAR
GET_VAR=OPT_BUILD_VARS_LIST
endif
OPT_BUILD_VARS_LIST = OPT_ML_MLX4_SLEEP_MODE OPT_ML_MLX4_INIT_TIMEOUT OPT_ML_HAS_LIN_EVENT_TABLE_IN_RAM

# the GET_VAR need to be specified from the command line
.PHONY: get_var
get_var:
	@echo $($(GET_VAR))