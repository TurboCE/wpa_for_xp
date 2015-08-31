

#project name
TARGET	= xp_wpa_cracker

CC	= /opt/intel/bin/icc 
#CC = gcc

# compiling flags here
CFLAGS	= -Wall -I./src -L/opt/intel/composerxe/lib/intel64/ 
CFLAGS += -fopenmp -O3 -openmp-link static
CFLAGS += -mmic
CFLAGS += -DUSE_LIGHT_CRYPTO
#CFLAGS += -vec-report2 -g

LINKER	= /opt/intel/bin/icc -o
#LINKER = gcc -o

# linking flags here
LFLAGS	= -Wall
LFLAGS += -fopenmp -O3 -openmp-link static
LFLAGS += -mmic
#LFLAGS += -g

#LIBSSL	= -lssl -lcrypto
LIBS = $(LIBSSL)

# change these to set the proper directories where each files shoould be
SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin

SOURCES  := $(wildcard $(SRCDIR)/*.c)
INCLUDES := $(wildcard $(SRCDIR)/*.h) 
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
rm       = rm -f

$(BINDIR)/$(TARGET): $(OBJECTS)
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
	@$(rm) $(BINDIR)/$(TARGET)
	@echo "Executable removed!"

.PHONEY: test
test: FORCE 
	@sshpass -f passwd scp $(BINDIR)/$(TARGET) dekaf@mic0:/home/dekaf/
	@sshpass -f passwd scp test/target.mimo.hccap dekaf@mic0:/home/dekaf/
	@sshpass -f passwd ssh dekaf@mic0 time /home/dekaf/$(TARGET) target.mimo.hccap
#	$(BINDIR)/$(TARGET) ./test/target.mimo.hccap
#	@cat test/sample1.txt
FORCE:

