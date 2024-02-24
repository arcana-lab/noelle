SCAF_INSTALL_DEBUG?=$(shell pwd)/scaf-install-debug
SCAF_INSTALL_RELEASE?=$(shell pwd)/scaf-install-release
SVF_AVAILABLE?=0
JOBS?=16
SPECULATION_MODULES?=1
GENERATOR?="Unix Makefiles"

CC=clang
CXX=clang++

ifeq ($(SPECULATION_MODULES), 1)
	BUILD_SPECULATION=-DENABLE_SPECULATION=ON
else
	BUILD_SPECULATION=-DENABLE_SPECULATION=OFF
endif

all: scaf-debug scaf-release

clean:
		rm -rf scaf-build-debug scaf-build-release

uninstall:
		rm -rf $(SCAF_INSTALL_DEBUG) $(SCAF_INSTALL_RELEASE)

scaf-debug:
	mkdir -p scaf-build-debug
	cd ./scaf-build-debug && \
	cmake -G${GENERATOR} -DCMAKE_INSTALL_PREFIX="$(SCAF_INSTALL_DEBUG)" -DCMAKE_BUILD_TYPE=Debug -DLLVM_ENABLE_UNWIND_TABLES=On -DCMAKE_CXX_FLAGS="-std=c++17" -DSVF_AVAILABLE=$(SVF_AVAILABLE) $(BUILD_SPECULATION) ../ && \
	cmake --build . -j${JOBS} && \
	cmake --install .

scaf-release:
	mkdir -p scaf-build-release
	cd ./scaf-build-release && \
	cmake -G${GENERATOR} -DCMAKE_INSTALL_PREFIX="$(SCAF_INSTALL_RELEASE)" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-std=c++17" -DSVF_AVAILABLE=$(SVF_AVAILABLE) $(BUILD_SPECULATION) ../ && \
	cmake --build . -j${JOBS} && \
	cmake --install .
