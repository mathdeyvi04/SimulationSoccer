#ifndef SINGULAR_H
#define SINGULAR_H


template <class classe_qualquer>
class Singular {
	/*
	Descrição:
	    Implementa o padrão de projeto Singleton genérico em C++.
	    Garante que exista apenas uma instância de uma determinada classe
	    durante toda a execução do programa, fornecendo acesso global a essa instância.
	    
	    Excelente para gerenciar recursos globais, como controladores, gerenciadores de estado
	    ou acesso centralizado a configurações.

	Método público:
	    - static classe_qualquer& obter_instancia():
	        Retorna uma referência para a única instância existente da classe T.
	        A instância é criada na primeira chamada e é garantidamente única.

	Comportamento:
	    - Construtor, destrutor, cópia e atribuição são privados para impedir múltiplas instâncias.
	*/
public:
	
	static classe_qualquer& obter_instancia() {

		static classe_qualquer instancia;

		return instancia;
	}

private:

	Singular();
	~Singular();
	Singular (Singular const&);
	Singular& operator=(Singular const&);
};

#endif // SINGULAR_H