MAKEFLAGS += --no-builtin-rules

CXX = g++
CPPFLAGS = -std=c++2a -Wall -Wextra -Werror -ggdb3 -O0 -Iinclude -I../httpserver

COMMONHPP=include/iot/core/error.hh include/iot/core/varadic.hh include/iot/message.hh include/iot/log.hh include/iot/core/math.hh include/iot/core/types.hh include/iot/core/ipv6addr.hh

TESTCPPFILES=test/test.cc lib/iotclouderror.cc lib/log.cc ../httpserver/http11.tab.cc ../httpserver/lex.yy.cc ../httpserver/http11driver.cc ../httpserver/http11.cc
TESTHPPFILES=include/iot/serversocket.hh test/test.hh include/iot/socket.hh $(COMMONHPP)
TESTLIBFILES=/usr/lib/x86_64-linux-gnu/libcrypto.a /usr/lib/x86_64-linux-gnu/libresolv.a -pthread

SERVERCPPFILES=lib/iotclouderror.cc lib/iotmessage.cc lib/log.cc server.cc
SERVERHPPFILES=include/iot/serversocket.hh server.hh include/iot/socket.hh $(COMMONHPP)
SERVERLIBFILES=/usr/lib/x86_64-linux-gnu/libcrypto.a /usr/lib/x86_64-linux-gnu/libresolv.a -pthread

CLIENTCPPFILES=client.cc lib/iotmessage.cc lib/iotclouderror.cc lib/log.cc
CLIENTHPPFILES=include/iot/message.hh include/iot/socket.hh $(COMMONHPP)
CLIENTLIBFILES=/usr/lib/x86_64-linux-gnu/libcrypto.a /usr/lib/x86_64-linux-gnu/libresolv.a -pthread

.PHONY: all

all: testipv6addr testcommon iotcloudserver iotcloudclient

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
	rm testcommon iotcloudserver iotcloudclient