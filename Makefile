# parser-experiments

BUILD_DIR=build
debug=no

.POSIX:

all: options parser-exp

config:
	@mkdir -p $(BUILD_DIR)/

options:
	@echo "debug = $(debug)"

parser-exp: config
	@make -C calc/ debug=$(debug)
	@make -C graph/ debug=$(debug)
	@make -C genexpr/ debug=$(debug)

	@cp calc/build/calc build/
	@cp graph/build/graph build/
	@cp genexpr/build/genexpr build/

clean:
	@rm -rf $(BUILD_DIR)/

.PHONY: all options clean

