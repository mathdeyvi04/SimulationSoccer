src = $(wildcard *.cpp)
obj = $(src:.c=.o)

# Atente-se a isso
CFLAGS = -O3 -shared -std=c++11 -fPIC -Wall $(PYBIND_INCLUDES)

all: $(obj)
    # Atente-se a isso
	g++ $(CFLAGS) -o a_estrela.so $^

debug: $(filter-out lib_main.cpp,$(obj))
    # Atente-se a isso
	g++ -O0 -std=c++14 -Wall -g -o debug.bin debug_main.cc $^

.PHONY: clean
clean:
	rm -f $(obj) all