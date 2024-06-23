CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic -g
EXECUTABLE = select poll epoll
SELECT_SRC = select.c
POLL_SRC = poll.c
EPOLL_SRC = epoll.c

.PHONY: all clean

all: $(EXECUTABLE)

select: $(SELECT_SRC:.c=.o)
	$(CC) $(CFLAGS) $^ -o $@

poll: $(POLL_SRC:.c=.o)
	$(CC) $(CFLAGS) $^ -o $@

epoll: $(EPOLL_SRC:.c=.o)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(EXECUTABLE) $(SELECT_SRC:.c=.o) $(POLL_SRC:.c=.o) $(EPOLL_SRC:.c=.o)

check:
	cppcheck --enable=warning,style,performance,portability,information,missingInclude --std=c11 $(SELECT_SRC) $(POLL_SRC) $(EPOLL_SRC)

valgrind: $(EXECUTABLE)
	for exe in $(EXECUTABLES); do \
		if [ -x "$$exe" ]; then \
			valgrind --leak-check=full ./$$exe; \
		else \
			echo "Error: $$exe does not exist or is not executable."; \
		fi; \
	done

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

