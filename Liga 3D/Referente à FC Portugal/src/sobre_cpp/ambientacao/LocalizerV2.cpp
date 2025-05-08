#include <cstdio>
#include "LocalizerV2.h"
#include "math.h"
#include "World.h"

/*
Precisamos te um arquivo .cpp para que ele seja executado em conjunto.
*/
using namespace std;

static World& mundo_existente = Singular<World>::obter_instancia();
























int main(){
	printf("\n\nExecutado com sucesso.\n\n");

	return 0;
}