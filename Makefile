
CC=g++
CCFLAGS=-shared
LDLIB=libfilelock.so libhashlock.so

all: $(LDLIB)

libhashlock.so: hashLock.o
	$(CC) hashLock.o -fPIC -shared -o libhashlock.so
	mv libhashlock.so ./lib/libhashlock.so

libfilelock.so: fileLock.o
	$(CC) fileLock.o -fPIC -shared -o libfilelock.so
	mv libfilelock.so ./lib/libfilelock.so

hashLock.o: hashLock.cpp hashLock.hpp
	$(CC) -fPIC -c hashLock.cpp 

fileLock.o: fileLock.cpp fileLock.hpp
	$(CC) -fPIC -c fileLock.cpp 

test: test.cpp
	$(CC) -o test test.cpp -L./lib -lhashlock 

clean:
	rm -rf *.o
