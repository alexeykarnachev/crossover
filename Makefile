SOURCES := \
	$(wildcard ./src/*.c) \
	$(wildcard ./src/**/*.c)

# -Wall -pedantic \
# g++ \
# -std=c++2a \

all:
	gcc -Wall -std=c99 -D_DEFAULT_SOURCE -Wno-missing-braces \
	-o crossover \
	-I./deps/include -I./src/ \
	$(SOURCES) \
	-L./deps/lib/linux/ \
	-Wl,-rpath=./deps/lib/linux/ \
	-lraylib -l:cimgui.so -l:libcimgui.so -lnfd \
	$(shell pkg-config --libs gtk+-3.0) \
	-lm -lpthread -ldl -lGL -lstdc++

# -l:cimgui.so -l:libcimgui.so -lnfd -lraylib -lpthread -lm -ldl
