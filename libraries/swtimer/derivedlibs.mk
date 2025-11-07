# @file
# @brief List of mandatory libraries
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
# @details Provides list of mandatory libraries needed by this library
#

#
# MANDATORY EXTRA BU LIBRARIES
#
ifneq ($(filter $(PRODUCT),81330 81332 81334),)
BU_LIBS_DERIVED += adc_conv_8133x
endif
ifneq ($(filter $(PRODUCT),81340 81344 81346),)
BU_LIBS_DERIVED += adc_conv_8134x
endif
ifeq ($(PRODUCT),81160)
BU_LIBS_DERIVED += adc_conv_81160
endif
