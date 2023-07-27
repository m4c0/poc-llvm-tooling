LLVM_DIR=$(shell brew --prefix llvm@16)

CXX=$(LLVM_DIR)/bin/clang++
INCLUDE_DIR=$(LLVM_DIR)/include
LIB_DIR=$(LLVM_DIR)/lib

LIBS=-lclang -lclang-cpp -lLLVM

all: test
	./test

test: test.cpp
	${CXX} -I${INCLUDE_DIR} -L${LIB_DIR} -std=c++20 test.cpp -o test ${LIBS}
