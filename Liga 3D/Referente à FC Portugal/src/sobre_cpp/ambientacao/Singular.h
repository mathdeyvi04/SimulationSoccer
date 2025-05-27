/*
Estava com um bug imenso, pois estava '#define SINGULAR',
logo este header era importado duas vezes, o que causava erro de 
definição.
*/
#ifndef SINGULAR_H
#define SINGULAR_H

/*
Garantir que a classe possa ser usada para qualquer outra.
*/
template <class classe_qualquer>  
class Singular {
	/*
	No exemplo original, ele define como Singleton:
	
		Para garantir que uma classe tenha APENAS uma instância
		em toda a aplicação.
		
		Isso é útil para:
			-> Gerenciar Recursos Globais.
			-> Controlar acesso ao Hardware.
			
	Exemplo de uso:
		MinhaClasse& instancia = Singular<MinhaClasse>::obter_instancia();
	*/
public:
	
	static classe_qualquer& obter_instancia() {
		
		// O static garante que seja única e seja thread-safe.
		static classe_qualquer instancia;
		
		// Retorna referência para permitir modificações futuras.
		// Não há cópia, apenas o endereço é compartilhado.
		return instancia; 
	}
	
private:
	
	/*
	Observe a genialidade disso:
	
	- Construtor e Destrutor bloqueados.
		Não é possível inicializar ou destruir uma instância.
	
	- Cópia e Atribuição bloqueados.
		Isso violaria a unicidade.
	
	*/
	Singular(); 
	
	~Singular();
	
	Singular( Singular const& );
	
	Singular& operator=(Singular const&);
	
};

#endif  // SINGULAR_H
