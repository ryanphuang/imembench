INCLUDES += -I$(INCLUDEDIR)/redis
LDFLAGS += -lhiredis
OBJS += redis.o
