CC		= g++
CC_FLAGS	= -g

SOURCES	= latency-test.cc timer.cc

OBJECTS	= $(SOURCES:%.cc=%.o)

CC_LINK_FLAGS	= -lzmq

%.o:		%.cc
		$(CC) $(CC_FLAGS) -c $< -o $@

all: latency-test

latency-test:	$(OBJECTS)
		$(CC) $^ -o $@ $(CC_LINK_FLAGS)

clean:
	rm -f *.o latency-test *~
