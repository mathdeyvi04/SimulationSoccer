src = $(wildcard *.cpp)
obj = $(src:.cpp=.o)

# Para executar a compilação manual, descomente a seguinte linha: 
# FLAGS_DE_COMPILACAO_MANUAL = -I/usr/include/python3.12 -I/usr/include/pybind11

# E substitua o termo $(PYBIND_INCLUDES) por $(FLAGS_DE_COMPILACAO_MANUAL)

CXXFLAGS = -O3 -shared -std=c++11 -fPIC -Wall $(PYBIND_INCLUDES)

all: $(obj)
	g++ -O3 $(CXXFLAGS) -o a_estrela.so $^

teste:
	g++ -g debug.cc -o main; ./main;

.PHONY: clean

clean:
	rm -f $(obj) all

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@