# a convenience makefile

.PHONY : all

all : ./build/Makefile
	make -C "build"
	./build/runtests

./build/Makefile : ./build
	cd "build" && cmake ..

./build :
	mkdir "build"

.PHONY : clean

clean :
	rm -rf "build"


