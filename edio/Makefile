
OUTPUT	    = edio

all:$(OUTPUT).z64
.PHONY: all

N64_INST    = E:/devtools/n64-toolchain
N64_ROOTDIR = E:/devtools/n64-toolchain
MAKEFLAGS   = -j

BUILD_DIR = build
include $(N64_INST)/include/n64.mk

SRCC += $(wildcard *.c)
SRCS += $(wildcard *.s) 
OBJ  += $(SRCC:.c=.o)
OBJ  += $(SRCS:.s=.o)
OBJS = $(OBJ:%.o=$(BUILD_DIR)/%.o)

$(OUTPUT).z64: N64_ROM_TITLE = "EVERDRIVE DEMO"

$(BUILD_DIR)/$(OUTPUT).elf: $(OBJS)

clean:
	rm -rf $(BUILD_DIR) *.z64
.PHONY: clean

-include $(wildcard $(BUILD_DIR)/*.d)
