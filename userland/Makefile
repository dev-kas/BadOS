CC      ?= gcc
LD      ?= $(CC)
KEXLD   := ~/kex/kex-ld

BASE_CFLAGS := \
    -ffreestanding \
    -fno-stack-protector \
    -mno-red-zone \
    -nostdlib

BASE_LDFLAGS := -static -nostdlib

# discover program dirs
PROGRAM_DIRS := $(patsubst %/,%,$(dir $(wildcard */make.config)))

ALL_KEX :=

define PROGRAM_template

include $(1)/make.config

OBJ_$(PROG) := $$(addprefix $(1)/,$$(SRCS:.c=.o))
ELF_$(PROG) := $(1)/$$(PROG).elf
KEX_$(PROG) := $(1)/$$(PROG).kex

ALL_KEX += $$(KEX_$(PROG))

# compile
$(1)/%.o: $(1)/%.c
	$(CC) $(BASE_CFLAGS) $$(CFLAGS) -c $$< -o $$@

# link ELF
$$(ELF_$(PROG)): $$(OBJ_$(PROG)) $(1)/userland.ld
	$(CC) \
	    -T $(1)/userland.ld \
	    $(BASE_LDFLAGS) \
	    $$(LDFLAGS) \
	    -o $$@ $$(OBJ_$(PROG)) \
	    -L$(SYSROOT)/usr/lib -lc $$(LIBS)

# build KEX
$$(KEX_$(PROG)): $$(ELF_$(PROG))
	$(KEXLD) $$< $$@

endef

$(foreach dir,$(PROGRAM_DIRS),$(eval $(call PROGRAM_template,$(dir))))

# targets

all: $(ALL_KEX)

install: all
	mkdir -p $(DESTDIR)/ramdisk
	cp $(ALL_KEX) $(DESTDIR)/ramdisk/

clean:
	rm -f */*.o */*.elf */*.kex

.PHONY: all install clean
