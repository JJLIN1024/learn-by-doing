STDVERSION = c++11
CPPFLAGS = -std=${STDVERSION} -g -Wall -Wextra -Wshadow -Wconversion -Wpedantic -Werror
 
SOURCES = test.cpp
OBJS = test.o

all: $(OBJS)

%.o: %.c
	$(CC) -c -o $@ $< $(CPPFLAGS) $(CFLAGS)

check: $(SOURCES)
	clang-tidy -checks=cert-* $(SOURCES)
	cppcheck . --enable=all --quiet

clean:
	rm -f ${OBJS}

.PHONY : clean
