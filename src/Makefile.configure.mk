# @file
# @brief Build Configuration File
# @internal
#
# @copyright (C) 2018-2022 Melexis N.V.
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
# @details Build configuration file for the application.
#


#
# Target name
#
TARGET_EXT := Woory_4Way_Valve


#
# Application product name (string of 8 chars max)
#
APP_PRODUCT_ID = DCValve


#
# MCU Order Code
#

# QFN24
ORDER_CODE ?= 81332-xLW-BMx-202

# QFN32
#ORDER_CODE ?= 81334-xLW-AMx-001


#
# Application Version Number
#
APP_VERSION_MAJOR = 2
APP_VERSION_MINOR = 7
APP_VERSION_PATCH = 1
APP_VERSION_BUILD = 0


#
# Special application options to be added to CPPFLAGS
#
#CMD_OPTIONS ?= -DAPP_HAS_LINAA

#
# 64-bit flash readout protection key
# when upper 32-bit == lower 32-bit protection is disabled
#
PROTECTION_KEY ?= 0x0123012301230123


#
# LDF Nodegen
#
#LDF_FILE = ldf/8133x_lin22_pwm.ldf
#LIN_NODE = MLX_Node

#LDF_FILE = ldf/Woory_HLLD.ldf
#LIN_NODE = MOTORX

LDF_FILE = ldf/VPC_Fwv_LDF.ldf
LIN_NODE = VPC_Fwv_Slave

# Baudrate detection mode
#LDF_NODEGEN_FLAGS += -bd	# set baudrate detection mode to default baudrate from ldf-file (default)
LDF_NODEGEN_FLAGS += -bf	# autobaudrate detection on first frame

# Special functions
LDF_NODEGEN_FLAGS += -Sa	# force enable HAS_ASSIGN_NAD_SERVICE
#LDF_NODEGEN_FLAGS += -Sc	# force enable HAS_CONDITIONAL_CHANGE_NAD_SERVICE
LDF_NODEGEN_FLAGS += -Sd	# enable callback for diagnostic messages
LDF_NODEGEN_FLAGS += -Sn	# enable HAS_SERIAL_NUMBER_CALLOUT (enabled by default)
LDF_NODEGEN_FLAGS += -Sr	# enable HAS_READ_BY_ID_CALLOUT
LDF_NODEGEN_FLAGS += -Ss	# enable HAS_SAVE_CONFIGURATION_SERVICE (ldf schedule tables shall also be evaluated)


# Special functions used for ISO17987 only
#LDF_NODEGEN_FLAGS += -Sb	# enable READ_BY_ID_BIT_TIMING_TEST

# Special functions used for LIN20 and J2602 only
#LDF_NODEGEN_FLAGS += -Jc	# has ld_read_configuration and ld_set_configuration

# Special functions used for J2602 only
#LDF_NODEGEN_FLAGS += -Jf	# 0xB1 SID: has assign frame id service
#LDF_NODEGEN_FLAGS += -Ji	# 0xB2 SID: has read by identifier service
#LDF_NODEGEN_FLAGS += -Jn	# 0xB0 SID: has assign nad service


#
# Application CPU clock frequency (in MHz)
#
#FW_FREQ = 12
#FW_FREQ = 14
#FW_FREQ = 16
#FW_FREQ = 24
#FW_FREQ = 28
FW_FREQ = 32


#
# Flash bist page count definition
# @note Number of pages from start of flash to be checked by flash bist (< 0x0100).
#       The platform make infrastructure will configure this parameter per default to the maximum (= full flash memory).
#
#BIST_PAGE_COUNT = 0x0100

#
# Melexis LIN loader configuration
#
ifeq ($(filter -DAPP_HAS_LOADER,$(CMD_OPTIONS)),-DAPP_HAS_LOADER)
# Enable EEPROM programming (default range is full memory if other needs overwrite
# LDR_PROG_EEPROM_START_INDEX and LDR_PROG_EEPROM_END_INDEX)
LDT_HAS_EEPROM_FUNCTIONS = 1

# Enable flash programming (default range is full memory if other needs overwrite
# LDR_PROG_FLASH_START_INDEX and LDR_PROG_FLASH_END_INDEX)
LDT_HAS_FLASH_FUNCTIONS = 1
endif


#
# COLIN and Melexis LIN API priority settings
# see https://softdist.melexis.com/custassetsJRightFrame?assetname=MLX81330_32_AppNote_Base_Software_Improvements section 2.3.1
#

# COLIN interrupt priority
MLS_IRQ_PRIO = 5

# LIN API resources blocking CPU priority level
LINAPI_RESOURCES_BLOCKING_PRIORITY = 4

#
# Special application options to be added to CPPFLAGS
#

# Interrupt priority level for COLIN_LIN : LIN interrupt [3, 6]. See also Base Software Improvements AN
APP_OPTIONS += -DMLS_IRQ_PRIO=$(MLS_IRQ_PRIO)
APP_OPTIONS += -DLINAPI_RESOURCES_BLOCKING_PRIORITY=$(LINAPI_RESOURCES_BLOCKING_PRIORITY)
