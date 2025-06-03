src = $(wildcard *.cpp)
obj = $(src:.cpp=.o)

# Para executar a compilação manual, descomente a seguinte linha: 
# FLAGS_DE_COMPILACAO_MANUAL = -I/usr/include/python3.12 -I/usr/include/pybind11

# E substitua o termo $(PYBIND_INCLUDES) por $(FLAGS_DE_COMPILACAO_MANUAL)

LDFLAGS = -lgsl -lgslcblas
CXXFLAGS = -O3 -shared -std=c++11 -fPIC -Wall $(PYBIND_INCLUDES)

all: $(obj)
	g++ $(CXXFLAGS) -o ambientacao.so $^ $(LDFLAGS) 

teste: $(filter-out module_main.o, $(obj))
	g++ -O0 -std=c++14 -Wall -g -o debug debug.cc $^ $(LDFLAGS); ./debug

.PHONY: clean

clean:
	rm -f $(obj) debug all



