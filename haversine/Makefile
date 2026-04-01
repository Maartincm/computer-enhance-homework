CPP="g++"

BUILD=$(PWD)/build

RELEASE ?= 0
PROFILE ?= 0

OPTIMIZATION_FLAG=-O0
DEBUG_FLAG=-ggdb3 -g
SWITCHES=

ifeq ($(RELEASE),1)
OPTIMIZATION_FLAG=-O3
DEBUG_FLAG=-g
endif

ifeq ($(PROFILE),1)
SWITCHES+= -D PROFILE
# DEBUG_FLAG+= -g -pg
endif

COMMON_CPP_FLAGS=-Wall $(DEBUG_FLAG) $(OPTIMIZATION_FLAG) -I/usr/local/include

LINK_FLAGS=

MAIN_CPP=
EXTRA_CPP=

build-generator:
	$(CPP) $(COMMON_CPP_FLAGS) $(SWITCHES) -o haversine_json_generator haversine_json_generator.cpp $(EXTRA_CPP) $(LINK_FLAGS)

build-processor:
	$(CPP) $(COMMON_CPP_FLAGS) $(SWITCHES) -o haversine_process haversine_process.cpp $(EXTRA_CPP) $(LINK_FLAGS)

.PHONY: build-processor
