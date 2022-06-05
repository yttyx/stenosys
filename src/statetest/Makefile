#
# Makefile:
#
CC	    	:= g++

TARGET		:= statetest

SRCDIR		:= ./
INCDIR		:= ./
BUILDDIR	:= ./
LOGDIR	    := ./
TARGETDIR	:= ./

SRCEXT		:= cpp
OBJEXT		:= o

INC			:= -I/usr/include

# -O0       No optimisation
# -Wall		All warnings
CFLAGS		:= -O0 -Wall $(INCLUDE) -pipe
LDLIBS  	:= -L/usr/lib -L/usr/local/lib -lpthread -lconfig++ -lm -lX11 -lXtst

SOURCES 	:= $(shell find $(SRCDIR) -type f -name '*.$(SRCEXT)')
OBJECTS 	:= $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.$(OBJEXT)))

.DEFAULT_GOAL := $(TARGET)

# Make the directories
directories:
	@mkdir -p $(TARGETDIR)
	@mkdir -p $(BUILDDIR)
	@mkdir -p $(LOGDIR)

# Clean only object files
clean:
	@$(RM) -rf $(BUILDDIR)
	@$(RM) -rf $(TARGETDIR)
	@$(RM) -rf $(LOGDIR)

# Link
$(TARGET):	$(OBJECTS)
	@echo [link]
	@mkdir -p $(TARGETDIR)
	$(CC) -o $(TARGETDIR)/$(TARGET) $^ $(LDLIBS)

# Compile
$(BUILDDIR)/%.$(OBJEXT):	$(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

all             :       $(TARGET)
