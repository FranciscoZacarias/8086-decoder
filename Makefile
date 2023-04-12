ifeq ($(OS),Windows_NT)
	# Windows
	CC = cl.exe
	CFLAGS = /EHsc
	TARGET = sim8086.exe
	RM = del /q
else ifeq ($(shell uname), Darwin)
	# macOS
	CC = clang
	CFLAGS = -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-variable -Wno-format -Wno-c++14-extensions
	TARGET = sim8086
	RM = rm -f
else
	# Linux
	CC = gcc
	CFLAGS = -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-variable
	TARGET = sim8086
	RM = rm -f
endif

.PHONY: all clean

all: $(TARGET)

$(TARGET): sim8086.cpp
	$(CC) $(CFLAGS) -o $@ $<

clean:
	$(RM) $(TARGET)
