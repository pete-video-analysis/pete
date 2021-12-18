source_files := $(wildcard src/*.c)
obj_files := $(source_files:src/%.c=build/%.o)

CLIBS  := m
CFLAGS := -fPIC -l$(CLIBS)

all: shared static

static: build/libpete.a
	
shared: build/libpete.so

objects: $(obj_files)
	
build/%.o: src/%.c
	$(CC) -Iinclude $(CFLAGS) -c $< -o $@

build/libpete.a: objects
	ar rc build/libpete.a $(obj_files)

build/libpete.so: objects
	$(CC) -shared -o build/libpete.so $(obj_files) -l$(CLIBS)

clean:
	rm $(obj_files)
	rm build/libpete.a
	rm build/libpete.so