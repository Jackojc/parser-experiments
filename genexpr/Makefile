# genexpr

BUILD_DIR=build
TARGET=genexpr
LIBS=$(LDLIBS)
INC=-I../inc/

CXX?=clang++

SRC=main.cpp
STD=c++17
CXXWARN=-Wall -Wextra -Wcast-align -Wcast-qual -Wformat=2 -Wredundant-decls -Wshadow -Wundef -Wwrite-strings
CXXFLAGS+=-fno-rtti -fno-exceptions

debug?=yes

ifeq ($(debug),no)
	CXXFLAGS+=-O3 -march=native -flto -DNDEBUG -s

else ifeq ($(debug),yes)
	CXXFLAGS+=-Og -g -march=native -finstrument-functions

else
$(error debug should be either yes or no)
endif

ifeq ($(CXX),clang++)
	CXXWARN+=-ferror-limit=2
endif


.POSIX:

all: options genexpr

config:
	@mkdir -p $(BUILD_DIR)/

options:
	@echo "cc    = $(CXX)"
	@echo "debug = $(debug)"
	@echo "flags = -std=$(STD) $(CXXWARN) $(CXXFLAGS)"

genexpr: config
	@$(CXX) -std=$(STD) $(CXXWARN) $(CXXFLAGS) $(LDFLAGS) $(CPPFLAGS) $(INC) $(LIBS) -o $(BUILD_DIR)/$(TARGET) $(SRC)

clean:
	@rm -rf $(BUILD_DIR)/

.PHONY: all options clean

