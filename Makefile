# a convenience makefile
BUILD_DIR := ./build/$(shell hostname)
SOURCE_DIR := $(shell pwd)

.PHONY : all

all : $(BUILD_DIR)/runtests
	$(BUILD_DIR)/runtests

$(BUILD_DIR)/runtests : $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake $(SOURCE_DIR) && cmake --build .

$(BUILD_DIR) :
	mkdir -p $(BUILD_DIR)

.PHONY : clean

clean :
	rm -rf $(BUILD_DIR)


