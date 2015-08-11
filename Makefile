CC = gcc
CFLAGS=-Wall -O2

PRUFLAGS=-Wno-unused-result -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast

LD = gcc
LDFLAGS=

TARGET=tikitank

EFFECT=effects
PRUDRV=prussdrv

SRCDIR=src
OBJDIR=obj
BINDIR=bin

LIBS=-lpthread -lrt

COMMON_SRC := $(wildcard $(SRCDIR)/*.c)
COMMON_INC := $(wildcard $(SRCDIR)/*.h)
COMMON_OBJ := $(COMMON_SRC:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

PRUDRV_SRC := $(wildcard $(SRCDIR)/$(PRUDRV)/*.c)
PRUDRV_INC := $(wildcard $(SRCDIR)/$(PRUDRV)/*.h)
PRUDRV_OBJ := $(PRUDRV_SRC:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

EFFECT_SRC := $(wildcard $(SRCDIR)/$(EFFECT)/*.c)
EFFECT_INC := $(wildcard $(SRCDIR)/$(EFFECT)/*.h)
EFFECT_OBJ := $(EFFECT_SRC:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# pasm -V3 -b firmware.p firmware => firmware.bin

# TODO: Add CFLAGS=-mtune=cortex-a8 -march=armv7-a -O3

.PHONY: all clean

all: $(BINDIR)/$(TARGET)

$(BINDIR)/$(TARGET): $(COMMON_OBJ) $(PRUDRV_OBJ) $(EFFECT_OBJ)
	$(LD) $(LDFLAGS) -o $@ $(COMMON_OBJ) $(PRUDRV_OBJ) $(EFFECT_OBJ) $(LIBS)

$(COMMON_OBJ): | $(OBJDIR)
$(PRUDRV_OBJ): | $(OBJDIR)/$(PRUDRV)
$(EFFECT_OBJ): | $(OBJDIR)/$(EFFECT)

$(OBJDIR):
	@mkdir -p $@

$(OBJDIR)/$(PRUDRV):
	@mkdir -p $@

$(OBJDIR)/$(EFFECT):
	@mkdir -p $@

$(BINDIR)/$(TARGET) : | $(BINDIR)

$(BINDIR):
	@mkdir -p $@

$(COMMON_OBJ): $(OBJDIR)/%.o : $(SRCDIR)/%.c $(COMMON_INC) $(PRUDRV_INC)
	$(CC) $(CFLAGS) -I$(SRCDIR)/prussdrv -c -o $@ $<

$(PRUDRV_OBJ): $(OBJDIR)/%.o : $(SRCDIR)/%.c $(PRUDRV_INC)
	$(CC) $(CFLAGS) $(PRUFLAGS) -I$(SRCDIR)/prussdrv -c -o $@ $<

$(EFFECT_OBJ): $(OBJDIR)/%.o : $(SRCDIR)/%.c $(COMMON_INC) $(EFFECT_INC)
	$(CC) $(CFLAGS) -I$(SRCDIR) -c -o $@ $<

clean:
	rm -rf $(OBJDIR) $(BINDIR)
