CPP	= @echo " g++ $@"; g++
CC	= @echo " gcc $@"; gcc
LD	= @echo " ld  $@"; ld
AR	= @echo " ar  $@"; ar
RM	= @echo " RM	$@"; rm -f

CFLAGS += -Wall -std=c++11
CFLAGS += -g

#LDFLAGS += "-Wl" -lpthread -lc
LDFLAGS += -lcurl
LDFLAGS += -L.  ##-lenc -lsyslink  -lbinder -llog    -losa -lpdi -lsdk  -lz -lrt -lsplice -lmem -ldec
AFLAGS += -r

BINDIR = .
SRCDIR = .#/src ./src/io


SRCS_PATH = $(SRCDIR) 

LIB_SRCS += $(foreach dir,$(SRCS_PATH),$(wildcard $(dir)/*.cpp))

LIB_OBJS += $(patsubst %.cpp,%.o,$(LIB_SRCS))

LIB_APP = curldemo.a

DEMO_NAME	= curldemo
TARGET_DEMO = $(BINDIR)/$(DEMO_NAME)

all: $(TARGET_DEMO)

$(TARGET_DEMO) : $(LIB_APP)
	$(CPP) -o $@ $(LDFLAGS) $^

$(LIB_APP): $(LIB_OBJS)
	$(RM) $@;
	$(AR) $(AFLAGS) $@ $^

.c.o:
	$(CC) -c $(CFLAGS) $^ -o $@

.cpp.o:
	$(CPP) -c $(CFLAGS) $^ -o $@

clean:
	$(RM) $(LIB_OBJS) $(LIB_APP) $(TARGET_DEMO)

cleanobj:
	$(RM) $(LIB_OBJS)