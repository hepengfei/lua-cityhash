
CITYHASH_VER=1.1.1
CITYHASH_URL=http://cityhash.googlecode.com/files/cityhash-$(CITYHASH_VER).tar.gz

CITYHASH_SRC=cityhash-$(CITYHASH_VER)
CITYHASH_HEADER=-I$(CITYHASH_SRC)/src

LUA_HEADER="-I/usr/include/lua5.2"

all:cityhash.so

cityhash.so:lua_cityhash.o $(CITYHASH_SRC)/src/.libs/city.o
	g++ -ggdb -Wall -fPIC -shared $^ -o cityhash.so

lua_cityhash.o:lua_cityhash.cpp
	g++ -ggdb -Wall -c $< -o $@ -fPIC $(CITYHASH_HEADER) $(LUA_HEADER)

prepare:
	rm -rf cityhash-1.1.1
	curl -sq $(CITYHASH_URL) | tar -zxv
	cd cityhash-1.1.1 && ./configure --enable-shared && make

clean:
	rm -f lua_cityhash.o cityhash.so

test:
	lua test_cityhash.lua
