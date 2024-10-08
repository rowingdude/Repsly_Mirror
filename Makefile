CC = gcc
CFLAGS = -I./include -Wall -Wextra -pedantic
LDFLAGS = -lpq

SRCDIR = src
MODDIR = modules
OBJDIR = obj

SOURCES = $(wildcard $(SRCDIR)/*.c) $(wildcard $(MODDIR)/*.c)
OBJECTS = $(patsubst %.c,$(OBJDIR)/%.o,$(notdir $(SOURCES)))

TARGET = repsly_mirror

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(MODDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $@

clean:
	rm -rf $(OBJDIR) $(TARGET)

.PHONY: clean