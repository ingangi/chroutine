CPP	= @echo " g++ $@"; g++
CC	= @echo " gcc $@"; gcc
LD	= @echo " ld  $@"; ld
AR	= @echo " ar  $@"; ar
RM	= @echo " RM	$@"; rm -f

CFLAGS += -Wall -std=c++11 -DDEBUG_BUILD #-Wextra -pedantic -Wconversion
CFLAGS += -g3

#LDFLAGS += "-Wl" -lpthread -lc
LDFLAGS += -L. -lpthread  ##-lenc -lsyslink  -lbinder -llog    -losa -lpdi -lsdk  -lz -lrt -lsplice -lmem -ldec
AFLAGS += -r

INC = -I../engin -I../util -I../vendors  #-I./rpc

BINDIR = .


SRCS_PATH = . ../engin  ../util #./rpc ./proto_code/grpc_code ./proto_code/msg_code

LIB_SRCS += $(foreach dir,$(SRCS_PATH),$(wildcard $(dir)/*.cpp))

LIB_OBJS += $(patsubst %.cpp,%.o,$(LIB_SRCS))

LIB_APP = test.a

DEMO_NAME	= test
TARGET_DEMO = $(BINDIR)/$(DEMO_NAME)

all: $(TARGET_DEMO)

$(TARGET_DEMO) : $(LIB_APP)
	$(CPP) -o $@ $(LDFLAGS) $(INC) $^

$(LIB_APP): $(LIB_OBJS)
	$(RM) $@;
	$(AR) $(AFLAGS) $@ $^

.c.o:
	$(CC) -c $(CFLAGS) $(INC) $^ -o $@

.cpp.o:
	$(CPP) -c $(CFLAGS) $(INC) $^ -o $@

clean:
	$(RM) $(LIB_OBJS) $(LIB_APP) $(TARGET_DEMO)

cleanobj:
	$(RM) $(LIB_OBJS)

RPCS:
	(cd ./rpc_example; make clean; make;)
	
CHAN:
	(cd ./channel_example; make clean; make;)
	
LOCK:
	(cd ./chutex_example; make clean; make;)

TIMER:
	(cd ./timer_example; make clean; make;)
	
RPCC:
	(cd ./rpc_example/test_client; make clean; make;)
	
HTTPC:
	(cd ./http_client_example; make clean; make;)

SCHED:
	(cd ./sched_test; make clean; make;)
	
STACK:
	(cd ./stack_test; make clean; make;)

ECHOS:
	(cd ./tcp_echo_server_example; make clean; make;)

CLEAN:
	(make clean;cd ./rpc_example; make clean; cd ../rpc_example/test_client; make clean; cd ../../http_client_example; make clean;cd ../channel_example; make clean;cd ../chutex_example; make clean;cd ../timer_example; make clean;cd ../sched_test; make clean;cd ../stack_test; make clean;cd ../tcp_echo_server_example; make clean;)

ALLTEST:
	(make clean;make;make RPCS;make RPCC;make HTTPC;make CHAN;make LOCK;make TIMER;make SCHED;make STACK;make ECHOS)
