UNAME := $(shell uname)

PASM := $(shell which pasm)

CC = gcc
CFLAGS=-Wall -O3 -D$(UNAME)

CFLAGS_Linux=-Wno-unused-result -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast

LD = gcc
LDFLAGS_Linux=-L/lib/arm-linux-gnueabihf

TARGET=tikitank

EFFECT=effects
PAL=pal/$(UNAME)

SRCDIR=src
OBJDIR=obj
BINDIR=bin
PREFIX=/opt/tikitank

LIBS_Linux=-lrt -l:libusb-0.1.so.4.4.4
LIBS=-lpthread -lm

COMMON_SRC := $(wildcard $(SRCDIR)/*.c)
COMMON_INC := $(wildcard $(SRCDIR)/*.h)
COMMON_OBJ := $(COMMON_SRC:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

EFFECT_SRC := $(wildcard $(SRCDIR)/$(EFFECT)/*.c)
EFFECT_INC := $(wildcard $(SRCDIR)/$(EFFECT)/*.h)
EFFECT_OBJ := $(EFFECT_SRC:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

PAL_SRC := $(wildcard $(SRCDIR)/$(PAL)/*.c)
PAL_INC := $(wildcard $(SRCDIR)/$(PAL)/*.h)
PAL_OBJ := $(PAL_SRC:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# TODO: Add CFLAGS=-mtune=cortex-a8 -march=armv7-a -O3

.PHONY: all clean

all: $(BINDIR)/$(TARGET)

$(BINDIR)/$(TARGET): $(COMMON_OBJ) $(EFFECT_OBJ) $(PAL_OBJ)
	$(LD) $(LDFLAGS) $(LDFLAGS_$(UNAME)) -o $@ $(COMMON_OBJ) $(EFFECT_OBJ) $(PAL_OBJ) $(LIBS) $(LIBS_$(UNAME))

ifneq ($(strip $(PASM)),)

all: | $(BINDIR)/firmware.bin

$(BINDIR)/firmware.bin: $(SRCDIR)/$(PAL)/firmware.p
	$(PASM) -V3 -b $< $(basename $@)

endif

$(COMMON_OBJ): | $(OBJDIR)
$(EFFECT_OBJ): | $(OBJDIR)/$(EFFECT)
$(PAL_OBJ):    | $(OBJDIR)/$(PAL)

$(OBJDIR):
	@mkdir -p $@

$(OBJDIR)/$(EFFECT):
	@mkdir -p $@

$(OBJDIR)/$(PAL):
	@mkdir -p $@

$(BINDIR)/$(TARGET) : | $(BINDIR)

$(BINDIR):
	@mkdir -p $@

$(COMMON_OBJ): $(OBJDIR)/%.o : $(SRCDIR)/%.c $(COMMON_INC) $(EFFECT_INC)
	$(CC) $(CFLAGS) -c -o $@ $<

$(EFFECT_OBJ): $(OBJDIR)/%.o : $(SRCDIR)/%.c $(COMMON_INC) $(EFFECT_INC)
	$(CC) $(CFLAGS) -I$(SRCDIR) -c -o $@ $<

$(PAL_OBJ): $(OBJDIR)/%.o : $(SRCDIR)/%.c $(COMMON_INC) $(EFFECT_INC)
	$(CC) $(CFLAGS) $(CFLAGS_$(UNAME)) -I$(SRCDIR) -I$(SRCDIR)/$(PAL) -c -o $@ $<

clean:
	rm -rf $(OBJDIR) $(BINDIR)

install: all
	mkdir -p $(PREFIX)
	cp $(BINDIR)/$(TARGET) $(BINDIR)/firmware.bin $(PREFIX)
	cp -a static $(PREFIX)
	cp init /etc/init.d/tikitank
	update-rc.d tikitank defaults 99
