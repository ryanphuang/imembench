DEBUG ?= yes
SSE ?= sse4.2
COMPILER ?= gnu

OBJDIR	:= ../build/objs
INCLUDEDIR := ../build/include/c++
LIBDIR := ../libs/c++

ifeq ($(DEBUG),yes)
BASECFLAGS := -g
OPTFLAG	 :=
else
BASECFLAGS :=
OPTFLAG := -O3
endif

COMFLAGS := $(BASECFLAGS) $(OPTFLAG) -fno-strict-aliasing \
	        -m$(SSE)

ifeq ($(COMPILER),gnu)
COMFLAGS += -march=core2
endif

COMWARNS := -Wall -Wformat=2 -Wextra \
            -Wwrite-strings -Wno-unused-parameter -Wmissing-format-attribute
CXXWARNS := $(COMWARNS) -Wno-non-template-friend -Woverloaded-virtual \
		-Wcast-qual \
		-Wcast-align -Wconversion

INCLUDES :=

CXXFLAGS := $(COMFLAGS) -Werror $(CXXWARNS)
LDFLAGS := -L$(LIBDIR)

CC ?= gcc
CXX ?= g++
AR ?= ar

TARGET := imembench
OBJS := main.o

include Ramcloud.mk
include Tachyon.mk
include Redis.mk

DEPS := $(OBJS:.o=.d)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS)

-include $(DEPS)

%.o :%.cc
	$(CXX) -MMD -c $(CXXFLAGS) $(INCLUDES) $< -o $@

clean:
	rm -f *.o *.d $(TARGET)

check:
	echo OBJDIR=$(OBJDIR)
	echo INCLUDES=$(INCLUDES)
	echo CXXFLAGS=$(CXXFLAGS)
	echo LDFLAGS=$(LDFLAGS)

.PHONY: all clean check
