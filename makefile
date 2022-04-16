# compiler
CC := g++

# compiler flags:
CCFLAGS  := -Wall -std=c++11
CCDBGFLAGS := -DDEBUG -G -g 
LDFLAGS :=
OBJFLAGS := -c

# tool macros
INC_DIRS=./inc
LIBS= 
LIB_DIRS=
CCFLAGS+= -I $(INC_DIRS)

# path macros
BIN_PATH := bin
OBJ_PATH := obj
SRC_PATH := src
DBG_PATH := debug
DATA_PATH := data

# targets
TARGETS = $(foreach T, $(wildcard $(SRC_PATH)/*.cpp), $(BIN_PATH)/$(notdir $T))
CLEAN_LIST = $(basename $(TARGETS))

default: makedir clean all

# non-phony rule
all: $(basename $(TARGETS))

$(basename $(TARGETS)):
	$(CC) $(CCFLAGS) -o $(BIN_PATH)/$(notdir $@) $(SRC_PATH)/$(notdir $@).cpp

# phony rules
.PHONY: makedir
makedir:
	@mkdir -p $(BIN_PATH) $(OBJ_PATH) $(DBG_PATH) $(DATA_PATH)

.PHONY: clean
clean:
	@echo CLEAN $(CLEAN_LIST)
	@rm -f $(CLEAN_LIST)