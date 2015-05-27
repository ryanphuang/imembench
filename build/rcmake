TOP=../..

DEBUG ?= yes
SSE ?= sse4.2
COMPILER ?= gnu
VALGRIND ?= yes

OBJDIR	:= obj.master

EXTRALIBS=-v -L$(TOP)/$(OBJDIR) \
	$(TOP)/$(OBJDIR)/OptionParser.o

GTEST_DIR ?= $(TOP)/gtest

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
ifeq ($(VALGRIND),yes)
COMFLAGS += -DVALGRIND
endif

COMWARNS := -Wall -Wformat=2 -Wextra \
            -Wwrite-strings -Wno-unused-parameter -Wmissing-format-attribute
CXXWARNS := $(COMWARNS) -Wno-non-template-friend -Woverloaded-virtual \
		-Wcast-qual \
		-Wcast-align -Wconversion

LIBS := $(EXTRALIBS) -lramcloud -lboost_program_options -lboost_system

INCLUDES := -I$(TOP)/src -I$(TOP)/$(OBJDIR) \
	-I$(GTEST_DIR)/include

CXXFLAGS := $(COMFLAGS) -std=c++0x $(INCLUDES) $(LIBS) -Werror $(CXXWARNS)

CC ?= gcc
CXX ?= g++
AR ?= ar

all: RCI

RCI: rcclient

rcclient: RCClient.cc
	$(CXX) $(CXXFLAGS) $< \
	-o $@

run:
	(export LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(OBJDIR); \
	scripts/cluster.py --verbose --timeout=1000 --client=rcclient )

clean:
	/bin/rm -f rcclient *.o *~

check:
	echo OBJDIR=$(OBJDIR)
	echo INCLUDES=$(INCLUDES)
	echo CXXFLAGS=$(CXXFLAGS)
	echo LDFLAGS=$(LDFLAGS)
