.PHONY: all build clean

BINARY_NAME=mini-docker
ESCAPE_TOOL=escape-tool

all: clean build

build:
	go build -o $(BINARY_NAME) ./container/
	go build -o $(ESCAPE_TOOL) ./escape_tool/

clean:
	rm -f $(BINARY_NAME)
	rm -f $(ESCAPE_TOOL)
	@echo "已清除舊的執行檔: $(BINARY_NAME)"