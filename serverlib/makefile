MAKEFLAGS += --no-builtin-rules

CXX = g++-10
CPPFLAGS = -std=c++20 -Wall -Wextra -Werror -ggdb3 -O0 -Iinclude -I../httpserver

COMMONHPP=include/iot/core/error.hh include/iot/core/varadic.hh include/iot/message.hh include/iot/core/log.hh \
			include/iot/core/math.hh include/iot/core/types.hh include/iot/core/ipv6addr.hh include/iot/core/guid.hh \
			include/iot/states/states.hh include/iot/core/memory.hh

TESTCPPFILES=test/test.cc lib/log.cc lib/states/event_distributor.cc \
				lib/memory/memory.cc \
				../httpserver/http11.tab.cc ../httpserver/lex.yy.cc ../httpserver/http11driver.cc \
				../httpserver/http11.cc
TESTHPPFILES=include/iot/net/serversocket.hh test/test.hh include/iot/net/socket.hh $(COMMONHPP)
TESTLIBFILES=/usr/lib/x86_64-linux-gnu/libcrypto.a /usr/lib/x86_64-linux-gnu/libresolv.a -pthread

SERVERCPPFILES=lib/message.cc lib/log.cc server.cc
SERVERHPPFILES=include/iot/net/serversocket.hh server.hh include/iot/net/socket.hh $(COMMONHPP)
SERVERLIBFILES=/usr/lib/x86_64-linux-gnu/libcrypto.a /usr/lib/x86_64-linux-gnu/libresolv.a -pthread

CLIENTCPPFILES=client.cc lib/message.cc lib/log.cc
CLIENTHPPFILES=include/iot/message.hh include/iot/net/socket.hh $(COMMONHPP)
CLIENTLIBFILES=/usr/lib/x86_64-linux-gnu/libcrypto.a /usr/lib/x86_64-linux-gnu/libresolv.a -pthread

.PHONY: all

all: testipv6addr testcommon iotcloudserver iotcloudclient testmemory

testmemory: test/testmemory.cc lib/memory/memory.cc include/iot/core/memory.hh
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o $@ test/testmemory.cc lib/memory/memory.cc

testipv6addr: test/testipv6addr.cc include/iot/core/ipv6addr.hh include/iot/core/types.hh
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o $@ test/testipv6addr.cc /usr/lib/x86_64-linux-gnu/libresolv.a

testcommon: $(TESTCPPFILES) $(TESTHPPFILES)
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o $@ $(TESTCPPFILES) $(TESTLIBFILES)

iotcloudserver: $(SERVERCPPFILES) $(SERVERHPPFILES)
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o $@ $(SERVERCPPFILES) $(SERVERLIBFILES)

iotcloudclient: $(CLIENTCPPFILES) $(CLIENTHPPFILES)
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o $@ $(CLIENTCPPFILES) $(CLIENTLIBFILES)

%.o: %.cc
	$(CXX) -c $(CFLAGS) $(CPPFLAGS) -o $@ $^

clean:
	rm testipv6addr testcommon iotcloudserver iotcloudclient testmemory