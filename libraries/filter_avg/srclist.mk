# @file
# @brief Library sources list
# @internal
#
# @copyright (C) 2019-2020 Melexis N.V.
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
# @details Library sources list
#

#
# SEARCH PATHES
#
VPATH += $(BU_LIBS_DIR)/filter_avg/src


#
# SOURCE FILES LIST
#
BU_LIBS_SRCS += filter_avg.c


#
# HEADER SEARCH PATHES
#
BU_LIBS_INC_DIRS += $(BU_LIBS_DIR)/filter_avg/src


#
# LIBRARY AUTHORS
#
AUTHORS = "Benoit Leduc"
