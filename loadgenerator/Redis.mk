INCLUDES += -I$(INCLUDEDIR)/redis
LDFLAGS += -lhiredis
OBJS += hash.o redis.o
