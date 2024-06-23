CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic -g
EXECUTABLE = file_io
SRC = file_io.c

.PHONY: all clean

all: $(EXECUTABLE)

$(EXECUTABLE): $(SRC:.c=.o)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(EXECUTABLE) $(SRC:.c=.o)

check:
	cppcheck --enable=warning,style,performance,portability,information,missingInclude --std=c11 $(SRC)

valgrind: $(EXECUTABLE)
	if [ -x "$(EXECUTABLE)" ]; then \
		valgrind --leak-check=full ./$(EXECUTABLE); \
	else \
		echo "Error: $(EXECUTABLE) does not exist or is not executable."; \
	fi

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

