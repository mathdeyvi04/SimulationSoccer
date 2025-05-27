src = $(wildcard *.cpp)
obj = $(src:.cpp=.o)

# Para executar a compilação manual, descomente a seguinte linha: 
FLAGS_DE_COMPILACAO_MANUAL = -I/usr/include/python3.12 -I/usr/include/pybind11

# E substitua o termo $(PYBIND_INCLUDES) por $(FLAGS_DE_COMPILACAO_MANUAL)

LDFLAGS = -lgsl -lgslcblas
CXXFLAGS = -O3 -shared -std=c++11 -fPIC -Wall $(FLAGS_DE_COMPILACAO_MANUAL)
all: $(obj)
	g++ $(CXXFLAGS) -o localization.so $^ $(LDFLAGS) 

teste:
	python3 debug.py

.PHONY: clean

clean:
	rm -f $(obj) all

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Retirado a inicialização dos slots de tag dentro de cada opção de elemento de campo.
# Reordenado inicialização de fatores dentro de strucs como sMarcador.
# Havia definido funções especiais e únicas em um header que era importado em dois .cpp, e isso gerava dualidade.
# SUPER ERRO presente na verificação das linhas!.