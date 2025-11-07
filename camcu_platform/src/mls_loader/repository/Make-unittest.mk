.PHONY: utest
utest: utest_build utest_run

UNITTEST_FRAMEWORK?= $(PROJECT_ROOT)/$(UTEST_DIR)
CMOCK_FRAMEWORK ?= $(UNITTEST_FRAMEWORK)/tools/cmock
UNIT_TEST_FRAMEWORK_LIBNAME = unit_test_framework

COMPONENT_UNDER_TEST=LIB_MLX_LIN_LDR

# define IO-ports for terminal printing from ISS
SIM_INP   ?= 0x0300
SIM_OUTP  ?= 0x0300
SIM_STATP ?= 0x0301
SIM_RETP  ?= 0x0302

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

# Target file name (without extension)
.PHONY: all clean libs print_component prep_component clean_component build code_coverage.xml index.html


# =============
# Tool settings
# =============
CC     = mlx16-gcc
AS     = mlx16-as
AR     = mlx16-ar
ODP    = mlx16-objdump
OCP    = mlx16-objcopy
NM     = mlx16-nm
SIZE   = mlx16-size
RM     = rm -rf
CP     = cp -f
MKDIR  = mkdir -p
LS     = /bin/ls -1
OBJDIR = build
SRCDIR = .

CAT = cat
GENHTML = perl $(UNITTEST_FRAMEWORK)/tools/lcov/bin/genhtml
DOS2UNIX = dos2unix

include Make-rules.mk

# -----------------------------------------------------------------------------
# C_FILES and OBJS
#
# Determine which files should be compiled.
# -----------------------------------------------------------------------------
C_FILES_UNDER_TEST = $(wildcard $(SRCDIR)/$(COMPONENT)/*.c)
C_FILES = $(C_FILES_UNDER_TEST)
# C_FILES += $(filter-out $(SRCDIR)/$(COMPONENT)/unit_test/utest%.c, $(wildcard $(SRCDIR)/$(COMPONENT)/unit_test/*.c))
C_FILES += $(wildcard $(SRCDIR)/unit_test/src/*.c)

COMPONENT_INCLUDES := -I $(SRCDIR)/$(COMPONENT)

# Lists for all the utest executables for this component
TEST_C_FILES = $(wildcard $(SRCDIR)/unit_test/utest*.c)
TEST_OBJ_FILES = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(TEST_C_FILES))
TEST_ELF_FILES = $(patsubst $(OBJDIR)/%.o, $(PROJECT_ROOT)/$(RELEASE_DIR_UNIT)/%.elf, $(TEST_OBJ_FILES))
TEST_HEX_FILES = $(patsubst %.elf, %.hex, $(TEST_ELF_FILES))
TEST_LSS_FILES = $(patsubst %.elf, %.lss, $(TEST_ELF_FILES))
TEST_LOG_FILES = $(patsubst %.elf, %.log, $(TEST_ELF_FILES))
TEST_COV_CSV_FILES = $(patsubst %.elf, %_coverage.csv, $(TEST_ELF_FILES))
TEST_COV_XML_FILES = $(patsubst %.csv, %.xml, $(TEST_COV_CSV_FILES))
TEST_COV_HTML_FILES = $(patsubst %.csv, %.html, $(TEST_COV_CSV_FILES))
TEST_ELF_DUMP_FILES = $(patsubst %.elf, %_elf.dump, $(TEST_ELF_FILES))
TEST_DUT_DUMP_FILES = $(patsubst %.elf, %_dut.dump, $(TEST_ELF_FILES))
TEST_JUNIT_FILES = $(patsubst $(PROJECT_ROOT)/$(RELEASE_DIR_UNIT)/unit_test/%.log, $(PROJECT_ROOT)/$(RELEASE_DIR_UNIT)/utest_summary/$(COMPONENT_UNDER_TEST)_%_report.xml, $(TEST_LOG_FILES))
TEST_RST_FILES = $(patsubst $(PROJECT_ROOT)/$(RELEASE_DIR_UNIT)/unit_test/%.log, $(PROJECT_ROOT)/$(RELEASE_DIR_UNIT)/utest_summary/$(COMPONENT_UNDER_TEST)_%_report.rst, $(TEST_LOG_FILES))

OVERALL_LCOV_INFO = overall_coverage.info

# Variables for the unit test output of this component
TEST_COV_XML_FILE = $(PROJECT_ROOT)/$(RELEASE_DIR_UNIT)/coverage_summary/$(COMPONENT_UNDER_TEST)_coverage.xml
TEST_COV_HTML_FILE = $(PROJECT_ROOT)/$(RELEASE_DIR_UNIT)/coverage_html/$(COMPONENT_UNDER_TEST)_coverage.html

S_FILES_UNDER_TEST = $(wildcard $(SRCDIR)/$(COMPONENT)/*.S)
S_FILES = $(S_FILES_UNDER_TEST)
S_FILES += $(wildcard $(SRCDIR)/unit_test/*.S)
S_FILES += $(wildcard $(SRCDIR)/unit_test/src/*.S)

# find the mocks for the unit tests
H_FILES := $(wildcard $(TEMPLATE_DIR)/*.h)
MOCK_H_FILES := $(H_FILES:$(TEMPLATE_DIR)/%.h=$(SRCDIR)/unit_test/mockcomponents/%_mock.h)
MOCK_C_FILES := $(MOCK_H_FILES:%.h=%.c)
MOCK_OBJ_FILES := $(MOCK_C_FILES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
MOCK_INCLUDES := -I $(SRCDIR)/unit_test/mockcomponents
MOCK_LIB_NAME := mockcomponents
MOCK_LIB := $(RELEASE_DIR_UNIT)/lib$(MOCK_LIB_NAME).a


C_OBJ_FILES_UNDER_TEST = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(C_FILES_UNDER_TEST))
S_OBJ_FILES_UNDER_TEST = $(patsubst $(SRCDIR)/%.S, $(OBJDIR)/%.o, $(S_FILES_UNDER_TEST))
OBJS_UNDER_TEST = $(C_OBJ_FILES_UNDER_TEST) $(S_OBJ_FILES_UNDER_TEST)

LINK_C_OBJ_FILES = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(C_FILES))
LINK_S_OBJ_FILES = $(patsubst $(SRCDIR)/%.S, $(OBJDIR)/%.o, $(S_FILES))
C_OBJ_FILES = $(LINK_C_OBJ_FILES) $(TEST_OBJ_FILES) $(MOCK_OBJ_FILES)
S_OBJ_FILES = $(LINK_S_OBJ_FILES)
OBJS = $(LINK_S_OBJ_FILES) $(LINK_C_OBJ_FILES)
OBJS_NOT_UNDER_TEST = $(filter-out $(OBJS_UNDER_TEST),$(OBJS)) $(TEST_OBJ_FILES) $(MOCK_OBJ_FILES)

# Include automatic dependencies
DEPS = $(OBJS:%.o=%.d)
ifneq (${MAKECMDGOALS},clean)
-include $(DEPS)
endif

UTEST_LIBS := $(UNITTEST_FRAMEWORK)/libmlxunit.a $(UNITTEST_FRAMEWORK)/libmlxprintf.a $(UNITTEST_FRAMEWORK)/libcmock.a

# -----------------------------------------------------------------------------
# CFLAGS & LDFLAGS
#
# Determine the flags for linking and compiling the source code
# -----------------------------------------------------------------------------

# Basic settings
INSTSET = -mlx16-e8
LD_SCRIPT := unit_test/mupet_unittest.ld

ASFLAGS = -gdwarf-2

# OPTIMIZATION = -Os
# The component under test should not be built with optimization flags
# However, it is ok for the other source files (test files, startup files, mock files)
# to be built with this optimization, as otherwise the executable grows too big,
# and we are forced to split the test up in too many separate tests.
$(OBJS_NOT_UNDER_TEST): EXTRA_CFLAGS := -ffunction-sections -fdata-sections -Os
$(OBJS_UNDER_TEST): EXTRA_CFLAGS := -O0

# CFLAGS  = $(OPTIMIZATION)
CFLAGS = -g3
CFLAGS += -std=gnu99
CFLAGS += -mram-align-word

# Warnings
CFLAGS += -Wall -Wextra		# all + extra basic warnings!
CFLAGS += -Wstrict-prototypes	# warn if function argument types are not defined
CFLAGS += -Wundef		# warn of use of undefined macros
CFLAGS += -Winline		# warn when functions requested as inline are not inlined
CFLAGS += -Wno-packed-bitfield-compat # false warning on bitfields
CFLAGS += -Wno-unused # don't warn about unused variables
CFLAGS += -Wno-unknown-pragmas # don't warn about unknown pragma's in cmock generated mock files
CFLAGS += $(INSTSET) \
		  -mram-align-word \
		  -mdpage-explicit \
		  -DNO_STDIO_PRINTF \
		  $(EXTRA_CFLAGS) \
		  -D_DEBUG_
CFLAGS += $(INCLUDES) $(COMPONENT_INCLUDES) $(MOCK_INCLUDES) \
          -I$(UNITTEST_FRAMEWORK)/inc \
          -I$(CMOCK_FRAMEWORK)/src \
          -D_SIM_DEBUG_ -DUNITTEST \
          -DCFG_NV_RAM_SETTERS_PRESENTED \
          -DNO_STDIO_PRINTF

CPPFLAGS  = -MMD
CPPFLAGS += $(INCLUDES) $(COMPONENT_INCLUDES) $(MOCK_INCLUDES)
CPPFLAGS += -DSIM_INP=$(SIM_INP) -DSIM_OUTP=$(SIM_OUTP) -DSIM_STATP=$(SIM_STATP) -DSIM_RETP=$(SIM_RETP)
CPPFLAGS += -DFPLL="(250ul * )"
CPPFLAGS += -DF_CPU=24000000UL # Legacy, yo.
CPPFLAGS += -DMCU_PLL_MULT=

ASFLAGS  = -gdwarf-2

LDFLAGS := -N
LDFLAGS += -L$(RELEASE_DIR_UNIT) \
		   -L$(UNITTEST_FRAMEWORK) \
		   -l$(MOCK_LIB_NAME) \
           -l$(UNIT_TEST_FRAMEWORK_LIBNAME)
LDFLAGS += -T $(LD_SCRIPT) \
           $(INSTSET) \
           -Wl,--defsym,ROM_CRC_L=0xDEAD \
		   -Wl,--defsym,ROM_CRC_H=0xBEEF

include Make-src.mk

# -----------------------------------------------------------------------------
# Libraries
#
# Build the libraries needed to compile and link the unit tests.
# -----------------------------------------------------------------------------
libs: $(UNITTEST_FRAMEWORK)/lib$(UNIT_TEST_FRAMEWORK_LIBNAME).a $(MOCK_LIB)

$(UNITTEST_FRAMEWORK)/lib$(UNIT_TEST_FRAMEWORK_LIBNAME).a:
	@if [ $(DEBUG) -ge 1 ]; then echo Building unit test framework; fi
	$(HIDE_CMD)$(MAKE) --directory=$(UNITTEST_FRAMEWORK) INSTSET=$(INSTSET) OBJDIR=$(OBJDIR) LIBNAME=lib$(UNIT_TEST_FRAMEWORK_LIBNAME)

# -----------------------------------------------------------------------------
# Library of mock objects for all components
# -----------------------------------------------------------------------------
$(MOCK_C_FILES):$(SRCDIR)/unit_test/mockcomponents/%_mock.c: $(TEMPLATE_DIR)/%.h
	@if [ $(DEBUG) -ge 2 ]; then $(COLOR_VIOLET) echo " - $< --> $@"; $(COLOR_RESET) fi
	$(HIDE_CMD)$(MKDIR) $(dir $@)
	$(HIDE_CMD)$(RUBY) $(UNITTEST_FRAMEWORK)/tools/cmock/lib/cmock.rb -o$(UNITTEST_FRAMEWORK)/tools/CMockConfig.yml $< $(NO_OUTPUT)
	# $(HIDE_CMD)mv unit_test/mockcomponents/$(notdir $(basename $@)).* $(dir $@)

$(MOCK_LIB): $(MOCK_OBJ_FILES)
	@if [ $(DEBUG) -ge 2 ]; then echo Archiving $@; fi
	$(HIDE_CMD)$(AR) rcs $@ $?

# -----------------------------------------------------------------------------
# utest
#
# This target is actually called from the top makefile.
# It generates the JUnit xml report and the coverage report.
# -----------------------------------------------------------------------------
utest_build: print_component_build libs prep_component $(TEST_LSS_FILES) $(TEST_DUT_DUMP_FILES) $(TEST_ELF_DUMP_FILES) $(TEST_SE_FILES) $(TEST_SEF_FILES)

utest_run: print_component_run $(TEST_JUNIT_FILES) $(TEST_RST_FILES) $(TEST_COV_XML_FILE) $(TEST_COV_HTML_FILE)

# -----------------------------------------------------------------------------
# ELF file
#
# Link the executable(s) for testing
# -----------------------------------------------------------------------------
$(TEST_ELF_FILES): $(PROJECT_ROOT)/$(RELEASE_DIR_UNIT)/%.elf : $(OBJDIR)/%.o $(OBJS) $(LD_SCRIPT) Makefile Make-unittest.mk
	@if [ $(DEBUG) -ge 2 ]; then echo Linking $@; fi
	$(HIDE_CMD)$(MKDIR) $(dir $@)
	$(HIDE_CMD)$(CC) $< $(OBJS) -o $@ $(LDFLAGS) -Wl,-Map,$(strip $(patsubst %.elf, %.map, $@))

# -----------------------------------------------------------------------------
# Test log and coverage csv file
#
# Creates the log file of a utest executable. Also creates the coverage csv file.
# These are the result of running the ELF in the simulator
# -----------------------------------------------------------------------------
$(TEST_LOG_FILES): %.log: %.elf %.lss
	@if [ $(DEBUG) -ge 3 ]; then echo "Initialization sim environment"; fi
	$(HIDE_CMD)cscript unit_test/simulation/mlx16-setcfg.vbs unit_test/simulation/mlx16_ex_ut_nbr.ini $(NO_OUTPUT)
	@echo "Simulating $(PRODUCT): $<"
	$(HIDE_CMD)mlx16p -inp:0x0300 -outp:0x0300 -statusp:0x0301 -returnp:0x0302 $<
	$(HIDE_CMD)mv $(dir $<)/simout.txt $@
	$(HIDE_CMD)mv $(dir $<)/ccov.csv $(patsubst %.log, %_coverage.csv, $@)

#$(TEST_COV_CSV_FIlES): $(PROJECT_ROOT)/$(RELEASE_DIR_UNIT)/$(COMPONENT)/unit_test/%_coverage.csv: $(PROJECT_ROOT)/$(RELEASE_DIR_UNIT)/$(COMPONENT)/unit_test/ccov.csv
#	$(HIDE_CMD)mv $< $@

# -----------------------------------------------------------------------------
# JUnit XML report
#
# Creates the JUnit XML report, by feeding the Log file of the utest executable through a script.
# -----------------------------------------------------------------------------
$(TEST_JUNIT_FILES): $(PROJECT_ROOT)/$(RELEASE_DIR_UNIT)/utest_summary/$(COMPONENT_UNDER_TEST)_%_report.xml: $(PROJECT_ROOT)/$(RELEASE_DIR_UNIT)/unit_test/%.log $(TEST_DUT_DUMP_FILES) $(TEST_ELF_DUMP_FILES)
	@if [ $(DEBUG) -ge 3 ]; then echo "Generating JUnit XML report"; fi
	$(HIDE_CMD)$(PYTHON3) $(UNITTEST_FRAMEWORK)/scripts/mlxunit-to-junit.py -i $< -o $@
	# Check for failures, but igoring error code in case of failure, in order to not stop the build here
	$(HIDE_CMD)mlx-warnings $@ --junit ; true

$(TEST_RST_FILES): $(PROJECT_ROOT)/$(RELEASE_DIR_UNIT)/utest_summary/$(COMPONENT_UNDER_TEST)_%_report.rst: $(PROJECT_ROOT)/$(RELEASE_DIR_UNIT)/unit_test/%.log $(TEST_DUT_DUMP_FILES) $(TEST_ELF_DUMP_FILES)
	@if [ $(DEBUG) -ge 3 ]; then echo "Generating RST report"; fi
	$(HIDE_CMD)$(PYTHON3) $(UNITTEST_FRAMEWORK)/scripts/mlxunit-to-rst.py -i $< -o $@

# -----------------------------------------------------------------------------
# Dump files
#
# Creates dump files using mlx16-nm. These are then used to determine coverage information
# -----------------------------------------------------------------------------
$(TEST_DUT_DUMP_FILES): %_dut.dump: $(OBJS_UNDER_TEST)
	@if [ $(DEBUG) -ge 3 ]; then echo "Dumping DUT object symbols in $@"; fi
	$(HIDE_CMD)$(NM) -n -S $^ > $@

$(TEST_ELF_DUMP_FILES): %_elf.dump: %.elf
	@if [ $(DEBUG) -ge 3 ]; then echo "Dumping all elf symbols in $@"; fi
	$(HIDE_CMD)$(NM) -n -S $< > $@

# -----------------------------------------------------------------------------
#  Coverage HTML report
#
#  Creates the coverage HTML reports for the component under test. This report
#  is a colored version (color indicates coverage) of the lss file.
# -----------------------------------------------------------------------------
$(COMPONENT_UNDER_TEST)_html :
	$(HIDE_CMD)if [ $(DEBUG) -ge 3 ]; then echo "Generating HTML to view code coverage"; fi
	#$(HIDE_CMD) $(DOS2UNIX) $(PROJECT_ROOT)/$(RELEASE_DIR_UNIT)/unit_test/$(COMPONENT_UNDER_TEST)_coverage.info


# -----------------------------------------------------------------------------
#  Component coverage XML report (merged coverage)
#
#  Creates the merged coverage XML reports for the component under test. This report
#  can be used in jenkins to display a summary (column with colors in the job page).
# -----------------------------------------------------------------------------
$(TEST_COV_XML_FILE):
	@if [ $(DEBUG) -ge 3 ]; then echo "Generating coverage information"; fi
	$(HIDE_CMD)$(PYTHON3) $(UNITTEST_FRAMEWORK)/scripts/mapping_main.py \
        --dir=$(PROJECT_ROOT)/$(RELEASE_DIR_UNIT)/unit_test/ \
        --compname=$(COMPONENT_UNDER_TEST) \
        --outfile=$(PROJECT_ROOT)/$(RELEASE_DIR_UNIT)/unit_test/$(@F) \
        --format lcov cobertura \
        --files_under_test $(notdir $(C_FILES_UNDER_TEST))
	@if [ $(DEBUG) -ge 3 ]; then echo "TODO: summarizing coverage"; fi
	$(HIDE_CMD)$(CP) $(PROJECT_ROOT)/$(RELEASE_DIR_UNIT)/unit_test/$(@F) $@


# -----------------------------------------------------------------------------
#  Component coverage HTML report (merged coverage)
#
#  Creates the merged coverage HTML report for the component under test. This report
#  is a colored version (color indicates coverage) of the lss file.
# -----------------------------------------------------------------------------
$(TEST_COV_HTML_FILE): $(COMPONENT_UNDER_TEST)_html
	$(HIDE_CMD)if [ -a $(PROJECT_ROOT)/$(RELEASE_DIR_UNIT)/unit_test/$(COMPONENT_UNDER_TEST)_coverage.info ]; then \
		if [ $(DEBUG) -ge 3 ]; then echo "TODO: summarizing coverage"; fi;\
		$(CAT) $(PROJECT_ROOT)/$(RELEASE_DIR_UNIT)/unit_test/$(COMPONENT_UNDER_TEST)_coverage.info >> $(PROJECT_ROOT)/$(RELEASE_DIR_UNIT)/coverage_html/$(OVERALL_LCOV_INFO);\
	fi

# -----------------------------------------------------------------------------
#  Print component and product information
# -----------------------------------------------------------------------------
print_component:
	@echo "-------------------------------------------------"
	@echo "Unit testing $(PRODUCT)"
	@echo "-------------------------------------------------"

# -----------------------------------------------------------------------------
# Component preparation
#
# Prepare the component for unit test, by running the default target on the component unit test Makefile (if available)
# -----------------------------------------------------------------------------
prep_component:
	$(HIDE_CMD)if [ -f "$(COMPONENT)/unit_test/Makefile" ]; then $(MAKE) -C $(COMPONENT)/unit_test; fi
clean_component:
	$(HIDE_CMD)if [ -f "$(COMPONENT)/unit_test/Makefile" ]; then $(MAKE) -C $(COMPONENT)/unit_test clean; fi

# -----------------------------------------------------------------------------
# Reports
#
# This target produces the output reports, like the html coverage report and the xml coverage report for Jenkins.
# -----------------------------------------------------------------------------
.PHONY: reports command-line-summary
reports: code_coverage.xml index.html command-line-summary

code_coverage.xml:
	$(HIDE_CMD)$(PYTHON3) $(UNITTEST_FRAMEWORK)/scripts/xlmcover_cobertura.py --generatesummary --xmlfolder=$(PROJECT_ROOT)/$(RELEASE_DIR_UNIT)/coverage_summary

index.html:
	$(HIDE_CMD)if [ -a  $(PROJECT_ROOT)/$(RELEASE_DIR_UNIT)/coverage_html/$(OVERALL_LCOV_INFO) ]; then \
		#$(DOS2UNIX) $(PROJECT_ROOT)/$(RELEASE_DIR_UNIT)/coverage_html/$(OVERALL_LCOV_INFO) && \
		$(PYTHON3) $(UNITTEST_FRAMEWORK)/scripts/extract_prefixes.py -g "$(GENHTML)" -i $(PROJECT_ROOT)/$(RELEASE_DIR_UNIT)/coverage_html/$(OVERALL_LCOV_INFO) -o   $(PROJECT_ROOT)/$(RELEASE_DIR_UNIT)/coverage_html;\
	fi

# index.html:
# 	$(HIDE_CMD)$(PYTHON3) $(UNITTEST_FRAMEWORK)/scripts/visualize_coverage.py --generateindex --htmlfolder=$(PROJECT_ROOT)/$(RELEASE_DIR_UNIT)/coverage_html

command-line-summary:
	for junit in $(wildcard $(PROJECT_ROOT)/$(RELEASE_DIR_UNIT)/utest_summary/*_report.xml) ; do \
		mlx-warnings $$junit --junit ; \
	done

# -----------------------------------------------------------------------------
# Clean
# -----------------------------------------------------------------------------
clean:
	@echo Cleaning up unit test workspace.
	$(HIDE_CMD)$(RM) $(PROJECT_ROOT)/$(RELEASE_DIR_UNIT)/unit_test/*
	$(HIDE_CMD)$(RM) $(OBJS) $(TEST_OBJ_FILES)
	$(HIDE_CMD)$(RM) $(TEST_JUNIT_FILES)
	$(HIDE_CMD)$(RM) $(TEST_COV_XML_FILE) $(TEST_COV_HTML_FILE)
	# $(HIDE_CMD)$(RM) $(SRCDIR)/unit_test/mockcomponents/*.*

