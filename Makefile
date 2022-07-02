#
# Makefile:
#
CC	    	:= g++

TARGET		:= stenosys

SRCDIR		:= ./src
INCDIR		:= ./src
OBJDIR		:= ./obj
LOGDIR	    := ./log
TARGETDIR	:= ./bin

SRCEXT		:= cpp
OBJEXT		:= o

INC			:= -I/usr/include

# -O0       No optimisation
# -Wall		All warnings

# Use "make OUTPUT=X11" to build with X11 support
#     "make" alone builds for Pro Micro output (steno-in-the-middle)
ifeq ($(strip $(OUTPUT)),)
	CFLAGS		:= -O0 -Wall $(INCLUDE) -pipe
	LDLIBS  	:= -L/usr/lib -L/usr/local/lib -lpthread -lconfig++ -lm
else
	CFLAGS		:= -O0 -Wall $(INCLUDE) -pipe -D X11
	LDLIBS  	:= -L/usr/lib -L/usr/local/lib -lpthread -lconfig++ -lm -lX11 -lXtst
endif

STENOSYS_SOURCES := \
	cmdparser.cpp \
	cmdparserstate.cpp \
	config.cpp \
	dictionary.cpp \
	distribution.cpp \
	formatter.cpp \
	geminipr.cpp \
	kbdraw.cpp \
	kbdsteno.cpp \
	keyboard.cpp \
	log.cpp \
	miscellaneous.cpp \
	promicrooutput.cpp \
	serial.cpp \
	state.cpp \
	stenokeyboard.cpp \
	stenosys.cpp \
	stroke.cpp \
	strokefeed.cpp \
	strokes.cpp \
	symbols.cpp \
	textfile.cpp \
	translator.cpp \
	utf8.cpp \
	x11output.cpp

# Precede each source file with the source directory
STENOSYS_SOURCES_DIR := $(patsubst %,$(SRCDIR)/%,$(STENOSYS_SOURCES))
# Create a list of object files with their paths
STENOSYS_OBJECTS := $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(STENOSYS_SOURCES_DIR:.$(SRCEXT)=.$(OBJEXT)))

#TBW
#DICTBUILD_SOURCES :=
#DICTBUILD_SOURCES_DIR := \
#DICTBUILD_OBJECTS := \

.DEFAULT_GOAL := $(TARGET)

# Make the directories
directories:
	@mkdir -p $(TARGETDIR)
	@mkdir -p $(OBJDIR)
	@mkdir -p $(LOGDIR)

# Clean only object files
clean:
	@$(RM) -rf $(OBJDIR)
	@$(RM) -rf $(TARGETDIR)
	@$(RM) -rf $(LOGDIR)

# Link
$(TARGET):	$(STENOSYS_OBJECTS)
	@echo [link]
	@mkdir -p $(TARGETDIR)
	$(CC) -o $(TARGETDIR)/$(TARGET) $^ $(LDLIBS)

# Compile
$(OBJDIR)/%.$(OBJEXT):	$(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

all             :       $(TARGET)
	@echo ${X11}
