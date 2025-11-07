# ==================
# Directory settings
# ==================

OBJDIR ?= build$(OBJDIR_POSTFIX)
SRCDIR := ./src

LIBBASENAME ?= mlx_lin_ldr
LIBNAME ?= lib$(LIBBASENAME)

# Specifies where the library will be generated
# The RELEASE_DIR is available to specify externaly where the library will be generated
ifdef RELEASE_DIR
LIBDIR := $(RELEASE_DIR)
else
LIBDIR = .
endif

LIBRARY = $(LIBDIR)/$(LIBNAME)

UTEST_PRODUCT = MLX_LIN_LDR
UTEST_DIR	= tools/unittestframework
RELEASE_DIR_UNIT=$(OBJDIR)/unit_test

# Some makefile variables that can be used as include or library search path
INCDIR = inc
TEMPLATE_DIR = template

ifndef DEBUG
# There are three debug levels in our makefiles.
#  - First level does not print anything except building, warnings and built
#  - Second level (which is default) prints nice output
#  - Third level prints compiler commands
DEBUG = 2
endif

ifneq ($(DEBUG),3)
MAKEFLAGS += --no-print-directory
endif


# ============================================
# Compiler, assembler, and pre-processor flags
# ============================================

# Default instruction set for which the platform is compiled (stand-alone).
# The parent Makefile from a project using the platform can export its instruction set to override this default setting.
ifndef INSTSET
$(info 'Variable INSTSET (for instruction set) is not defined, using default')
INSTSET ?= -mlx16-e8
endif

CPPFLAGS_INTERNAL  = -I $(INCDIR)
ifdef STANDALONE_BUILD
$(info 'The default configurations are used for the standalone build only; \
please make sure all required options and dependencies are specified \
correctly according to project settings.')
CPPFLAGS_INTERNAL  += -I $(TEMPLATE_DIR)
endif

OPTIMIZATION = -Os # Used by both compiler and assembler!

# Basic settings
ifndef CFLAGS
$(info 'Variable CFLAGS (for C flags) is not defined, using default')
CFLAGS = -std=c99
CFLAGS += -g
# CFLAGS += -pedantic
CFLAGS += $(OPTIMIZATION)

# Tweaks
CFLAGS += -mram-align-word
CFLAGS += -ffunction-sections -fdata-sections

# Warnings
CFLAGS += -Wcast-align          # warn when casting to pointer type that requires more strict allignment
CFLAGS += -Wall -Wextra			# all + extra basic warnings!
CFLAGS += -Wstrict-prototypes	# warn if function argument types are not defined
CFLAGS += -Wundef		# warn of use of undefined macros
#CFLAGS += -Werror		# upgrade all warnings to errors!
CFLAGS += -Winline		# warn when functions requested as inline are not inlined
CFLAGS += -Wno-packed-bitfield-compat # false warning on bitfields
endif

ifndef ASFLAGS
$(info 'Variable ASFLAGS (for assembler flags) is not defined, using default')
ASFLAGS  = $(OPTIMIZATION) -gdwarf-2
endif

ARFLAGS = rcs


# =============
# Tool settings
# =============

CC   = mlx16-gcc
AS   = mlx16-as
AR   = mlx16-ar
ODP  = mlx16-objdump
OCP  = mlx16-objcopy
NM   = mlx16-nm
SIZE = mlx16-size

ECHO  = echo
RM    = rm -rf
CP    = cp -f
MKDIR = mkdir -p


# ============
# Find sources
# ============

COMPONENT_DIRS := $(SRCDIR) $(INCDIR)

C_FILES_PATT := $(addsuffix /*.c, $(COMPONENT_DIRS))
H_FILES_PATT := $(addsuffix /*.h, $(COMPONENT_DIRS))
S_FILES_PATT := $(addsuffix /*.S, $(COMPONENT_DIRS))

C_FILES := $(wildcard $(C_FILES_PATT))
H_FILES := $(wildcard $(H_FILES_PATT))
S_FILES := $(wildcard $(S_FILES_PATT))

C_OBJ_FILES := $(sort $(C_FILES:$(SRCDIR)/%.c=$(OBJDIR)/%.o))
S_OBJ_FILES := $(sort $(S_FILES:$(SRCDIR)/%.S=$(OBJDIR)/%.o))

# Include automatic dependencies if exist
OBJS := $(C_OBJ_FILES) $(S_OBJ_FILES)
OBJDEPS = $(OBJS:%.o=%.d)
-include $(OBJDEPS)


# =================
# Recipes and rules
# =================

# ------------------------------
# Default rule: make the library
# ------------------------------
.PHONY: all
all: all_setup $(LIBRARY).a stage-headers

$(LIBRARY).a: $(C_OBJ_FILES) $(S_OBJ_FILES)
	@if [ $(DEBUG) -ge 2 ]; then $(COLOR_YELLOW) echo "Archiving" $@; $(COLOR_RESET) fi
	$(HIDE_CMD)$(AR) $(ARFLAGS) $@ $(C_OBJ_FILES) $(S_OBJ_FILES)

.PHONY: stage-headers
stage-headers: $(RELEASE_DIR)
	$(HIDE_CMD)$(CP) -R $(INCDIR)/* $(RELEASE_DIR)

$(RELEASE_DIR):
	$(HIDE_CMD)$(MKDIR) $(RELEASE_DIR)


.PHONY: all_setup
all_setup:
	@-$(MKDIR) $(LIBDIR)

# -------------------
# Make objs from srcs
# -------------------
include Make-src.mk
include Make-rules.mk

# -----------
# Clean build
# -----------
.PHONY: clean
clean:
ifeq ($(CI), 1)
	@$(RM) $(OBJDIR)
	@$(RM) $(LIBDIR)/$(LIBRARY).a
#	@$(RM) unit_test/mockcomponents/*
#	@$(MAKE) -C $(UTEST_DIR) INSTSET=$(INSTSET) $@
#	@$(RM) $(shell find . -name "*.unc-backup*")
else
	@$(RM) $(OBJDIR)
	@$(RM) $(LIBDIR)/$(LIBRARY).a
#	@$(MAKE) -C $(UTEST_DIR) INSTSET=$(INSTSET) $@
#	@$(RM) unit_test/mockcomponents/*
#	@$(MAKE) -C doc/ $@ #also clean documentation build while we are at it #TODO: enable
#	@$(RM) $(shell find . -name "*.unc-backup*")
endif

# ---------------------------------------
# Compile and run all existing unit tests
# ---------------------------------------
COMPONENT_DIRS_UTEST := $(COMPONENT_DIRS)

# If a component is passed on to the make command, it will only execute the unit test for that component
ifdef COMPONENT
COMPONENT_DIRS_UTEST := $(filter %$(COMPONENT), $(COMPONENT_DIRS_UTEST))
endif

.PHONY: utest utest_setup $(COMPONENT_DIRS_UTEST) reports
utest: utest_setup $(COMPONENT_DIRS_UTEST) reports

utest_setup:
	@$(MKDIR) $(RELEASE_DIR_UNIT)
	@$(MKDIR) $(RELEASE_DIR_UNIT)/coverage_html
	@$(MKDIR) $(RELEASE_DIR_UNIT)/coverage_summary
	@$(MKDIR) $(RELEASE_DIR_UNIT)/utest_summary

$(COMPONENT_DIRS_UTEST):
	@$(MAKE) -f Make-unittest.mk clean COMPONENT=$@ \
			PRODUCT=$(UTEST_PRODUCT) \
			PROJECT_ROOT="." \
			UTEST_DIR=$(UTEST_DIR) \
			INCLUDES="-I $(INCDIR) -I $(TEMPLATE_DIR)" \
			RELEASE_DIR_UNIT=$(RELEASE_DIR_UNIT)
	@$(MAKE) -f Make-unittest.mk utest COMPONENT=$@ \
			PRODUCT=$(UTEST_PRODUCT) \
			PROJECT_ROOT="." \
			UTEST_DIR=$(UTEST_DIR) \
			INCLUDES="-I $(INCDIR) -I $(TEMPLATE_DIR)" \
			RELEASE_DIR_UNIT=$(RELEASE_DIR_UNIT)

reports:
	@$(MAKE) -f Make-unittest.mk reports PROJECT_ROOT="." \
			PRODUCT=$(UTEST_PRODUCT) \
			UTEST_DIR=$(UTEST_DIR) \
			RELEASE_DIR_UNIT=$(RELEASE_DIR_UNIT)

# -------------------
# Documentation build
# -------------------
.PHONY: doc
doc:
	@$(MAKE) html --directory=doc

# --------------------
# Uncrustify
# --------------------
U_CONFIG = tools/uncrustify-cfg-esw/uncrustify.cfg
UNCRUSTIFY_FILES := $(C_FILES) $(H_FILES)

.PHONY: uncrustify-dry-run
uncrustify-dry-run:
	$(HIDE_CMD)uncrustify -c $(U_CONFIG) --check -l C $(UNCRUSTIFY_FILES)

.PHONY: uncrustify
uncrustify:
	$(HIDE_CMD)uncrustify -c $(U_CONFIG) --replace --no-backup -l C $(UNCRUSTIFY_FILES)
