MAKEFLAGS += --no-builtin-rules

CXX = g++
CPPFLAGS = -std=c++17 -Wall -Wextra -Werror

simpleserver: simpleserver.cc http11.tab.cc lex.yy.cc http11driver.cc http11.cc
	$(CXX) $(CFLAGS) $(CPPFLAGS) -o $@ $^ /usr/lib/x86_64-linux-gnu/libcrypto.a

%.o: %.cc
	$(CXX) -c $(CFLAGS) $(CPPFLAGS) -o $@ $^

parseronly: http11.tab.cc http11.tab.hh location.hh lex.yy.cc lex.yy.hh

http11.tab.cc http11.tab.hh location.hh: http11.yy
	bison --defines --language=c++ $<

lex.yy.cc lex.yy.hh: http11.l
	flex --header-file=lex.yy.hh $<

clean:
	rm http11.tab.cc http11.tab.hh location.hh lex.yy.cc lex.yy.hh simpleserver