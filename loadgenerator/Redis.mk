INCLUDES += -I$(INCLUDEDIR)/redis
LDFLAGS += -lhiredis
OBJS += redisdriver.o rediscluster.o
