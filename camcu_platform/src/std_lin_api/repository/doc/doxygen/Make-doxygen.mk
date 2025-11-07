#
# Copyright (C) 2018 Melexis N.V.
#
# Software Platform
#


# Dependencies from the environment (defined for the Make):
# -$(GEN_DIR)
# -$(LIN_SLAVE_API)
# -$(LIN_MASTER_API)


#
# DEFINITIONS
#

# Verbosity level
ifndef DEBUG
DEBUG = 2
endif

ifeq ($(DEBUG), 3)
HIDE_CMD =
NO_OUTPUT =
else
HIDE_CMD = @
NO_OUTPUT = > /dev/null
endif

# Varialbes
DOXYDIR = ./doc/doxygen
DOXY_APPBUILDDIR = $(GEN_DIR)/docs_lin_api
DOXY_BUILDDIR = $(DOXYDIR)/build
DOXY_STAGEDIR = $(DOXYDIR)/staged
DOXY_README = $(DOXYDIR)/html/README.html
DOXY_STAGED_API = src/common/*.* inc/common/*.* 
DOXY_STAGED_API += $(GEN_DIR)/lin_signals.c $(GEN_DIR)/lin_signals.h
ifeq ($(LIN_SLAVE_API), 1)
	DOXY_STAGED_API += src/slave/*.* inc/slave/*.*
endif
ifeq ($(LIN_MASTER_API), 1)
	DOXY_STAGED_API += src/master/*.* inc/master/*.*
endif


#
# RULES
#

.PHONY: doxy
doxy:
	$(HIDE_CMD)echo; echo "***** Running Standard LIN API documentation build *****"
	$(HIDE_CMD)echo "-- Preparing the sources"
	$(HIDE_CMD)mkdir -p $(DOXY_STAGEDIR)
	$(HIDE_CMD)cp $(DOXY_STAGED_API) $(DOXY_STAGEDIR)
ifdef LM_DRIVER_REL_FIND_PATH
	$(HIDE_CMD)echo "-- Finding the LIN Master driver sources"
	$(HIDE_CMD)files=$$(find $(LM_DRIVER_REL_FIND_PATH) -name 'lm_uart.h' -o -name 'lm_driver.c' -o -name 'lm_driver.h'); \
	for file in $$files; do cp $$file $(DOXY_STAGEDIR)/$$(basename $$file); done
endif
	$(HIDE_CMD)mkdir -p $(DOXY_BUILDDIR)
	$(HIDE_CMD)echo "-- Running doxygen on source code"
	$(HIDE_CMD)doxygen $(DOXYDIR)/doxyfile $(NO_OUTPUT) || ( echo; echo "Doxygen installation isn't found. Please, find download link below:" echo "https://sourceforge.net/projects/doxygen/files/latest/download" echo ; exit 1)
	$(HIDE_CMD)mkdir -p $(DOXY_APPBUILDDIR)
	$(HIDE_CMD)cp -a $(DOXY_README) $(DOXY_APPBUILDDIR)
	$(HIDE_CMD)cp -a $(DOXY_BUILDDIR)/* $(DOXY_APPBUILDDIR)
	$(HIDE_CMD)echo "-- Clean-up temporary files"
ifneq ($(DOXY_BUILDDIR), $(DOXY_APPBUILDDIR))
	$(HIDE_CMD)rm -rf $(DOXY_BUILDDIR)
	$(HIDE_CMD)rm -rf $(DOXY_STAGEDIR)
endif
	$(HIDE_CMD)echo "-- Generation has been finished; path for the output:"; echo "$(DOXY_APPBUILDDIR)"

.PHONY: clean_doxy	
clean_doxy:
	$(HIDE_CMD)echo "-- Removing $(DOXY_APPBUILDDIR) ..."
	$(HIDE_CMD)rm -rf $(DOXY_APPBUILDDIR)
