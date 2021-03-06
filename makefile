 
CC := g++ # This is the main compiler
# CC := clang --analyze # and comment out the linker last line for sanity
SRCDIR := src
BUILDDIR := build
TARGET := bin/photobooth
 
SRCEXT := cpp
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
CFLAGS := -std=c++11 -g `pkg-config cairomm-1.0 --cflags`  `pkg-config freetype2 --cflags` `pkg-config libgphoto2 --cflags`# -Wall
LDFLAGS := 
LIB := -pthread -L lib -lfcgi -ljpeg `pkg-config cairomm-1.0 --libs` -lX11 `pkg-config freetype2 --libs` `pkg-config libgphoto2 --libs` -lcups
INC := -I include

TEST_OBJECTS := $(BUILDDIR)/test.o $(BUILDDIR)/Logging.o


$(TARGET): $(OBJECTS)
	@echo "Linking..."
	@echo " $(CC) $^ -o $(TARGET) $(LIB)"; $(CC) $^ -o $(TARGET) $(LIB) $(LDFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo " $(CC) $(CFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	@echo " Cleaning..."; 
	@echo " $(RM) -r $(BUILDDIR) $(TARGET)"; $(RM) -r $(BUILDDIR) $(TARGET)

test: test_target

test_target: $(TEST_OBJECTS)
	@echo $(TEST_OBJECTS)
	@echo "Linking..."
	$(CC) $^ -o bin/test $(LIB) $(LDFLAGS) -DTEST

.PHONY: clean
