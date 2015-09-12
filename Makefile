

TARGET = intel
#project name
OUTPUT	= xp_wpa_cracker

ifeq ($(TARGET),intel)
CC = gcc
LINKER = gcc -o
else
CC	= /opt/intel/bin/icc 
LINKER	= /opt/intel/bin/icc -o
endif

# compiling flags here
CFLAGS	= -Wall -I./src -L/opt/intel/composerxe/lib/intel64/ 
CFLAGS += -fopenmp -O3
CFLAGS += -DUSE_LIGHT_CRYPTO
#CFLAGS += -vec-report2 -g

# linking flags here
LFLAGS	= -Wall
LFLAGS += -fopenmp -O3
#LFLAGS += -g

#LIBSSL	= -lssl -lcrypto
LIBS = $(LIBSSL)

ifeq ($(TARGET),intel)

else
CFLAGS += -openmp-link static
CFLAGS += -mmic
LFLAGS += -openmp-link static
LFLAGS += -mmic
endif

# change these to set the proper directories where each files shoould be
SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin

SOURCES  := $(wildcard $(SRCDIR)/*.c)
INCLUDES := $(wildcard $(SRCDIR)/*.h) 
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
rm       = rm -f

$(BINDIR)/$(OUTPUT): $(OBJECTS)
	@$(LINKER) $@ $(LFLAGS) $(OBJECTS) $(LIBS)
	@echo "Linking complete!"

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

.PHONEY: clean
clean:
	@$(rm) $(OBJECTS)
	@echo "Cleanup complete!"

.PHONEY: remove
remove: clean
	@$(rm) $(BINDIR)/$(OUTPUT)
	@echo "Executable removed!"

.PHONEY: test
test: FORCE 
ifeq ($(TARGET),intel)
	@$(BINDIR)/$(OUTPUT) ./test/target.mimo.hccap
else
	@sshpass -f passwd scp $(BINDIR)/$(OUTPUT) dekaf@mic0:/home/dekaf/
	@sshpass -f passwd scp test/target.mimo.hccap dekaf@mic0:/home/dekaf/
	@sshpass -f passwd ssh dekaf@mic0 time /home/dekaf/$(OUTPUT) target.mimo.hccap
endif
#	@cat test/sample1.txt
FORCE:

