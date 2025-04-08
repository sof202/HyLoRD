BUILD_DIR ?= build

# Detect number of processor cores
NPROC := $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1)

VERBOSE ?= 0
ifeq ($(VERBOSE),1)
	MAKE_FLAGS += VERBOSE=1
endif

all: build

configure:
	@echo "Configuring CMake in '$(BUILD_DIR)'..."
	@mkdir -p $(BUILD_DIR) && \
	 cd $(BUILD_DIR) && \
	 cmake $(CMAKE_FLAGS) ..

build: configure
	@echo "Building with $(NPROC) jobs..."
	@cmake --build $(BUILD_DIR) --parallel $(NPROC) $(MAKE_FLAGS)

clean:
	@echo "Cleaning build directory..."
	@cmake --build $(BUILD_DIR) --target clean


.PHONY: all configure build clean
