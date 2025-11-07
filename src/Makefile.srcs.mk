# @file
# @brief Application sources list
# @internal
#
# @copyright (C) 2018-2021 Melexis N.V.
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
# @details List of application sources to be build into the firmware application.
#


#
# SOURCE FILES LIST
# note: files can be located in subfolders of src/, the SRCS_APP entry needs to be
#       relative to src/ folder.
#
SRCS_APP += adc.c
SRCS_APP += diagnostic.c
SRCS_APP += eeprom_app.c
SRCS_APP += lin22.c
SRCS_APP += main.c
SRCS_APP += pwm.c
SRCS_APP += protection.c
SRCS_APP += AppValve.c
SRCS_APP += AppLin.c
SRCS_APP += dcm_driver.c
SRCS_APP += app_sensor.c
SRCS_APP += uart.c
#
# EXTRA PLATFORM MODULES TO COMPILE IN
#
APP_MODULES += app_descriptor
APP_MODULES += mls_api
APP_MODULES += mls_device_id
ifeq ($(filter -DAPP_HAS_LOADER,$(CMD_OPTIONS)),-DAPP_HAS_LOADER)
#APP_MODULES += mls_loader
endif
APP_MODULES += std_lin_api
APP_MODULES += user_startup


#
# PLATFORM LIBRARIES USED BY THIS APPLICATION
#
PLTF_LIBS += $(PRODUCT)
PLTF_LIBS += buffers
PLTF_LIBS += math
PLTF_LIBS += mlx_lin_api

#
# EXTRA BU-LIBRARIES USED BY THIS APPLICATION
#
BU_LIBS += adc_conv_8133x
BU_LIBS += filter_avg
#BU_LIBS += filter_lpf
#BU_LIBS += filter_pid
BU_LIBS += swtimer
BU_LIBS += unirom
#BU_LIBS += pwm_capture
#BU_LIBS += pwm_communication
BU_LIBS += uart_plot
