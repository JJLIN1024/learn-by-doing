CFLAGS = -std=c99 -Wall
objects = main.o 
target = lsh

all: $(target)

$(target): $(objects)
	$(CC) $(CFLAGS) $(objects) -o $@

clean:
	rm $(target) $(objects)

.PHONY : clean
