CC = gcc
CFLAGS=-Wall -O2

LD = gcc
LDFLAGS=

TARGET=tikitank

SRCDIR=src
OBJDIR=obj
BINDIR=bin

LIBS=-lpthread

SOURCES  := $(wildcard $(SRCDIR)/*.c)
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

.PHONY: all clean

all: $(BINDIR)/$(TARGET)

$(BINDIR)/$(TARGET): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)

$(OBJECTS): | $(OBJDIR)

$(OBJDIR):
	@mkdir -p $@

$(BINDIR)/$(TARGET) : | $(BINDIR)

$(BINDIR):
	@mkdir -p $@

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c $(INCLUDES)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJDIR) $(BINDIR)

