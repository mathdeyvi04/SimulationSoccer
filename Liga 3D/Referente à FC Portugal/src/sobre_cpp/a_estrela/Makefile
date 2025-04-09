src = $(wildcard *.cpp)
obj = $(src:.cpp=.o)

CFLAGS = -O3 -shared -std=c++11 -fPIC -Wall $(PYBIND_INCLUDES)

all: $(obj)
	g++ $(CFLAGS) -o a_estrela.so $^

debug: $(filter-out ponte_de_ligacao_entre_python_e.cpp,$(obj))
	g++ -O0 -std=c++14 -Wall -g -o debug.bin debug_main.cc $^

.PHONY: clean
clean:
	rm -f $(obj) all
