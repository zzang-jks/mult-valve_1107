# Default rules for project binaries.
#

OCP ?= $(shell which mlx16-objcopy)
ODP ?= $(shell which mlx16-objdump)
NM  ?= $(shell which mlx16-nm)

ifndef DEBUG
DEBUG = 0
endif

ifeq ($(DEBUG), 3)
HIDE_CMD =
else
HIDE_CMD = @
endif

# --- Confirm programs needed to run default rules ----------------------------
ifndef OCP
$(error 'Variable OCP (for objcopy) is not defined')
endif
ifndef ODP
$(error 'Variable ODP (for objdump) is not defined')
endif
ifndef NM
$(error 'Variable NM (for nm) is not defined')
endif

# --- Default rules -----------------------------------------------------------
# Create hex / bin files from ELF output file
%.hex: %.elf
	@if [ $(DEBUG) -ge 2 ]; then echo "Building" $@; fi
	$(HIDE_CMD)$(OCP) -O ihex $< $@

%.bin: %.elf
	@if [ $(DEBUG) -ge 2 ]; then echo "Building" $@; fi
	$(HIDE_CMD)$(OCP) -O binary $< $@

# Create extended listing file from ELF output file
%.lss: %.elf
	@if [ $(DEBUG) -ge 2 ]; then echo "Building" $@; fi
	$(HIDE_CMD)$(CC) --version > $@
	$(HIDE_CMD)$(ODP) -h -S -z $< >> $@

# Create a symbol table from ELF output file
%.sym: %.elf
	@if [ $(DEBUG) -ge 2 ]; then echo "Building" $@; fi
	$(HIDE_CMD)$(NM) -n $< > $@

