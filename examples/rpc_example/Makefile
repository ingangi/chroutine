CPP	= @echo " g++ $@"; g++
CC	= @echo " gcc $@"; gcc
LD	= @echo " ld  $@"; ld
AR	= @echo " ar  $@"; ar
RM	= @echo " RM	$@"; rm -f

CFLAGS += -Wall -std=c++11
CFLAGS += -g3

#LDFLAGS += "-Wl" -lpthread -lc
LDFLAGS += -L. -lpthread -lprotobuf -lgrpc++ -lgrpc
AFLAGS += -r

INC = -I../../engin -I../../net/rpc -I../../net/proto_code -I../../util -I../../vendors

BINDIR = .

SRCS_PATH = . ../../engin ../../net/rpc ../../net/proto_code  ../../util

LIB_SRCS += $(foreach dir,$(SRCS_PATH),$(wildcard $(dir)/*.cpp))
LIB_CC_SRCS += $(foreach dir,$(SRCS_PATH),$(wildcard $(dir)/*.cc))

LIB_OBJS += $(patsubst %.cpp,%.o,$(LIB_SRCS))
LIB_OBJS += $(patsubst %.cc,%.o,$(LIB_CC_SRCS))

LIB_APP = rpcsrv.a

DEMO_NAME	= rpcsrv
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
	
.cc.o:
	$(CPP) -c $(CFLAGS) $(INC) $^ -o $@

clean:
	$(RM) $(LIB_OBJS) $(LIB_APP) $(TARGET_DEMO)

cleanobj:
	$(RM) $(LIB_OBJS)
