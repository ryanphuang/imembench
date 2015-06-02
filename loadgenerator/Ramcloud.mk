VALGRIND ?= yes
INCLUDES += -I$(INCLUDEDIR)/ramcloud -I$(INCLUDEDIR)/gtest
CXXFLAGS += -std=c++0x
LDFLAGS += $(OBJDIR)/ramcloud/OptionParser.o \
		-lramcloud -lboost_program_options -lboost_system

ifeq ($(VALGRIND),yes)
CXXFLAGS += -DVALGRIND
endif

OBJS += ramcloud.o
