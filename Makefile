CC = gcc
CFLAGS = -I./include -Wall -Wextra -pedantic -g
LDFLAGS = -lpq -lcurl -ljansson -lssl -lcrypto
SRCDIR = src
MODDIR = modules
OBJDIR = obj
BINDIR = bin

# Add header files to track dependencies
DEPS = $(wildcard include/*.h)

# Separate source files by directory
SRC_SOURCES = $(wildcard $(SRCDIR)/*.c)
MOD_SOURCES = $(wildcard $(MODDIR)/*.c)

# Generate object file names
SRC_OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRC_SOURCES))
MOD_OBJECTS = $(patsubst $(MODDIR)/%.c,$(OBJDIR)/%.o,$(MOD_SOURCES))
OBJECTS = $(SRC_OBJECTS) $(MOD_OBJECTS)

TARGET = $(BINDIR)/repsly_mirror

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS) | $(BINDIR)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPS) | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(MODDIR)/%.c $(DEPS) | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR) $(BINDIR):
	mkdir -p $@

clean:
	rm -rf $(OBJDIR) $(BINDIR)

# Add a 'run' target for convenience
run: $(TARGET)
	./$(TARGET)