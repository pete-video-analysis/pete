# Detect platform
ifeq ($(OS),Windows_NT)
	object := obj
	static := lib
	shared := dll
else
	object := o
	static := a
	shared := so
endif

# Use only gcc
CC := gcc

source_files := $(wildcard src/*.c)
obj_files := $(source_files:src/%.c=build/%.$(object))

# utils.h requires the math library
CLIBS  := m

# Position Independent Code is needed for shared library
CFLAGS := -fPIC -l$(CLIBS)

all: static shared

static: build/libpete.$(static)
	
shared: build/libpete.$(shared)

objects: build/main.$(object)

# Link into single object file before stripping symbols
# Then remove all static methods (non-api symbols)
build/main.$(object): $(obj_files)
	$(LD) -Ur -o build/main.$(object) $(obj_files)
	objcopy --strip-unneeded build/main.$(object) build/main.$(object)
	
build/%.$(object): src/%.c
	$(CC) -Iinclude $(CFLAGS) -c $< -o $@

build/libpete.$(static): objects
	ar rc build/libpete.$(static) build/main.$(object)

build/libpete.$(shared): objects
	$(CC) -shared -o build/libpete.$(shared) build/main.$(object) -l$(CLIBS)

clean:
	rm $(obj_files)
	rm build/libpete.$(static)
	rm build/libpete.$(shared)