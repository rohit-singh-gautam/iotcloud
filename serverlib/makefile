MAKEFLAGS += --no-builtin-rules

CXX = g++
CPPFLAGS = -std=c++2a -Wall -Wextra -Werror -ggdb3 -O0 -Iinclude -I../httpserver

COMMONHPP=include/iotclouderror.hh include/iotcloudmath.hh include/iotcloudvaradic.hh include/iotmessage.hh include/log.hh include/iotcloudmath.hh include/iotcloudtypes.hh

TESTCPPFILES=test.cc lib/serversocket.cc lib/iotclouderror.cc lib/log.cc ../httpserver/http11.tab.cc ../httpserver/lex.yy.cc ../httpserver/http11driver.cc ../httpserver/http11.cc
TESTHPPFILES=include/serversocket.hh test.hh include/socket.hh $(COMMONHPP)
TESTLIBFILES=/usr/lib/x86_64-linux-gnu/libcrypto.a /usr/lib/x86_64-linux-gnu/libresolv.a -pthread

SERVERCPPFILES=lib/serversocket.cc lib/iotclouderror.cc lib/iotmessage.cc lib/log.cc server.cc
SERVERHPPFILES=include/serversocket.hh include/server.hh include/socket.hh $(COMMONHPP)
SERVERLIBFILES=/usr/lib/x86_64-linux-gnu/libcrypto.a /usr/lib/x86_64-linux-gnu/libresolv.a -pthread

CLIENTCPPFILES=client.cc lib/iotmessage.cc lib/iotclouderror.cc lib/serversocket.cc lib/log.cc
CLIENTHPPFILES=include/iotmessage.hh include/socket.hh $(COMMONHPP)
CLIENTLIBFILES=/usr/lib/x86_64-linux-gnu/libcrypto.a /usr/lib/x86_64-linux-gnu/libresolv.a -pthread

.PHONY: all

all: test iotcloudserver iotcloudclient

test: $(TESTCPPFILES) $(TESTHPPFILES)
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o $@ $(TESTCPPFILES) $(TESTLIBFILES)

iotcloudserver: $(SERVERCPPFILES) $(SERVERHPPFILES)
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o $@ $(SERVERCPPFILES) $(SERVERLIBFILES)

iotcloudclient: $(CLIENTCPPFILES) $(CLIENTHPPFILES)
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o $@ $(CLIENTCPPFILES) $(CLIENTLIBFILES)

%.o: %.cc
	$(CXX) -c $(CFLAGS) $(CPPFLAGS) -o $@ $^

clean:
	rm test iotcloudserver iotcloudclient