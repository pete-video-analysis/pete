source_files := $(wildcard src/*.c)
obj_files := $(source_files:src/%.c=build/%.o)

# utils.h requires the math library
CLIBS  := m

# Position Independent Code is needed for shared library
CFLAGS := -fPIC -l$(CLIBS)

all: static shared

static: build/libpete.a
	
shared: build/libpete.so

objects: build/main.o

build/main.o: $(obj_files)
	@# Link into single object file before stripping symbols
	$(LD) -Ur -o build/main.o $(obj_files)
	@# Remove all static methods (non-api symbols)
	objcopy --strip-unneeded build/main.o build/main.o
	
build/%.o: src/%.c
	$(CC) -Iinclude $(CFLAGS) -c $< -o $@

build/libpete.a: objects
	ar rc build/libpete.a build/main.o

build/libpete.so: objects
	$(CC) -shared -o build/libpete.so build/main.o -l$(CLIBS)

clean:
	rm $(obj_files)
	rm build/libpete.a
	rm build/libpete.so