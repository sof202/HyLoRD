BUILD_DIR ?= build
CMAKE_INSTALL_PREFIX ?= /usr/local

# Detect number of processor cores
NPROC := $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1)

CMAKE_FLAGS := -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE)
CMAKE_EXTRA_FLAGS ?=

# Only append INSTALL_PREFIX if it's not already in EXTRA_FLAGS
ifneq (,$(findstring -DCMAKE_INSTALL_PREFIX,$(CMAKE_EXTRA_FLAGS)))
  # It's already there, don't add it
else
  override CMAKE_EXTRA_FLAGS += -DCMAKE_INSTALL_PREFIX=$(CMAKE_INSTALL_PREFIX)
endif

all: build

configure:
	@echo "Configuring CMake in '$(BUILD_DIR)'..."
	@echo "Installation prefix: $(CMAKE_INSTALL_PREFIX)"
	@mkdir -p $(BUILD_DIR) && \
	 cd $(BUILD_DIR) && \
	 cmake $(CMAKE_FLAGS) $(CMAKE_EXTRA_FLAGS) ..

build: configure
	@cmake --build $(BUILD_DIR) --parallel $(NPROC)

install:
	@cmake --install $(BUILD_DIR)

docs:
	@cmake --build $(BUILD_DIR) --target docs

clean:
	@cmake --build $(BUILD_DIR) --target clean

full-clean: clean
	@rm -rf $(BUILD_DIR)


.PHONY: all configure build install docs clean full-clean 
