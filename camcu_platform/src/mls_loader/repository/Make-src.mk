# Default rules for compiling files.
#
CC ?= $(shell which mlx16-gcc)

ifndef DEBUG
DEBUG = 0
endif

ifeq ($(DEBUG), 3)
HIDE_CMD =
else
HIDE_CMD = @
endif

ifdef COLORS
COLOR_YELLOW = echo -n "\033[0;33m";
COLOR_BLUE = echo -n "\033[0;34m";
COLOR_GREEN = echo -n "\033[0;32m";
COLOR_RED = echo -n "\033[0;31m";
COLOR_VIOLET = echo -n "\033[0;35m";
COLOR_RESET = echo -n "\033[0m"; # Reset terminal colour to normal.
endif

# --- Confirm variables needed to run default source rules ---
ifndef CC
$(error 'Variable CC (for compiler) is not defined')
endif
ifndef MKDIR
$(error 'Variable MKDIR (for make directory) is not defined')
endif

# --- Paths for compilation and assembly ---
ifndef C_OBJ_FILES
$(error 'Variable C_OBJ_FILES (for C-object files) is not defined')
endif
ifndef S_OBJ_FILES
$(warning 'Variable S_OBJ_FILES (for S-object files) is not defined')
endif
ifndef SRCDIR
$(error 'Variable SRCDIR (location of source files) is not defined')
endif
ifndef OBJDIR
$(error 'Variable OBJDIR (location of object files) is not defined')
endif

# --- Compiler/assembler flags used ---
ifndef CFLAGS
$(error 'Variable CFLAGS (for compiler flags) is not defined')
endif
ifndef ASFLAGS
$(warning 'Variable ASFLAGS (for compiler flags) is not defined')
endif
ifndef INSTSET
$(error 'Variable INSTSET (for compiler instruction set) is not defined')
endif

# ---------------------------------------------------------
# If we change header file we want also make to compile all
# c files including that header file. Since we have objects
# here, we just expand CFLAGS to make sure this is
# happening.
#
# For future reference:
# it was never checked what happens if CFLAGS already
# contain the below flags
# ---------------------------------------------------------
#DEPFLAGS = -Wp,-MM,-MP,-MT,$@,-MF,$(@:.o=.d)
DEPFLAGS = -MMD

# ========================
# Rules to compile C files
# ========================
$(C_OBJ_FILES): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	@# Printing the messages
	@if [ $(DEBUG) -ge 2 ]; then $(COLOR_GREEN) echo " -" $< "->" $@; $(COLOR_RESET) fi

	@# Doing the actual work.
	@$(MKDIR) $(dir $@)
	$(HIDE_CMD)$(CC) -c $(DEPFLAGS) $(INSTSET) $(CFLAGS) $(CPPFLAGS) $(CPPFLAGS_INTERNAL) $< -o $@

# ========================
# Rules to compile S files
# ========================
$(OBJDIR)/%.o : $(SRCDIR)/%.S
	@# Printing the messages.
	@if [ $(DEBUG) -ge 2 ]; then $(COLOR_BLUE) echo " -" $< "->" $@; $(COLOR_RESET) fi

	@# Doing the actual work.
	@$(MKDIR) $(dir $@)
	$(HIDE_CMD)$(CC) -c $(DEPFLAGS) $(INSTSET) $(ASFLAGS) $(CPPFLAGS) $(CPPFLAGS_INTERNAL) $< -o $@
