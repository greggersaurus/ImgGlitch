# Set the compiler being used.
CC = g++

# Includes needed for proper compilation
INCLUDES =
# Libraries needed for linking
LDFLAGS = -ltiff

# Set Compiler Flags
CFLAGS =-g -c -Wall $(INCLUDES) -std=gnu++0x

# List of source to be compiled
SOURCES = main.cpp \
	  ImgGlitch.cpp

OBJECTS = $(SOURCES:.cpp=.o)
	EXECUTABLE = ImgGlitch

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	 $(CC)  $(OBJECTS) -o $@ $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(OBJECTS) $(EXECUTABLE)

