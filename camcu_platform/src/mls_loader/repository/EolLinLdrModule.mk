# @file MlxLinApiModule.mk
# @brief Melexis LIN API module configurations
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


#
# Module configuration
#

# Relative to the module root dir
MODULE_INCDIR := inc

# Application dependencies from the module
MODULE_DEPS := $(LIN_LDR_LIBDIR)/lib$(LIN_LDR_LIB).a

# High level make variables
LIN_LDR_CONFIGS = CPPFLAGS='$(CPPFLAGS)' CFLAGS='$(CFLAGS)' ASFLAGS='$(ASFLAGS)' \
INSTSET='$(INSTSET)' LIBDIR=$(LIN_LDR_LIBDIR) LIBNAME=$(LIN_LDR_LIB)


#
# Rules
#

# MLX LIN API library build
.PHONY: module_ldr_lin_ldr
module_ldr_lin_ldr:
	-$(MKDIR) $(LIN_LDR_LIBDIR)
	@$(MAKE) --directory=$(LIN_LDR_DIR) all $(LIN_LDR_CONFIGS)

.PHONY: module_clean_ldr_lin_ldr
module_clean_ldr_lin_ldr:
	@$(MAKE) --directory=$(LIN_LDR_DIR) clean $(LIN_LDR_CONFIGS)
	-$(RM) $(LIN_LDR_LIBDIR)
