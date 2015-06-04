INCLUDES += -I$(INCLUDEDIR)/redis
LDFLAGS += -lhiredis
OBJS += hash.o redisdriver.o rediscluster.o
