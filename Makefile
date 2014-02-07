# a convenience makefile
BUILD_DIR := ./build/$(shell hostname)
SOURCE_DIR := $(shell pwd)

.PHONY : all runtests

all : runtests
	$(BUILD_DIR)/runtests

valgrind : runtests
	CK_FORK=no valgrind $(BUILD_DIR)/runtests

runtests : $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake $(SOURCE_DIR) && cmake --build .

$(BUILD_DIR) :
	mkdir -p $(BUILD_DIR)

.PHONY : clean

clean :
	rm -rf $(BUILD_DIR)


