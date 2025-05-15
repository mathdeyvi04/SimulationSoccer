/*
Estava com um bug imenso, pois estava '#define SINGULAR',
logo este header era importado duas vezes, oq causava erro de 
defini��o.
*/
#ifndef SINGULAR_H
#define SINGULAR_H

/*
Garantir que a classe possa ser usada para qualquer outra.
*/
template <class classe_qualquer>  
class Singular {
	/*
	No exemplo original, ele define como SingleTon:
	
		Para garantir que uma classe tenha APENAS uma inst�ncia
		em toda a aplica��o.
		
		Isso � �til para:
			-> Gerenciar Recursos Globais.
			-> Controlar acesso ao Hardware.
			
	Exemplo de uso:
		MinhaClasse& instancia = Singular<MinhaClasse>::obter_instancia();
	*/
public:
	
	static classe_qualquer& obter_instancia() {
		
		// O static garante que seja �nica e seja thread-safe.
		static classe_qualquer instancia;
		
		// Retorna refer�ncia para permitir modifica��es futuras.
		// N�o h� c�pia, apenas endere�o � compartilhado.
		return instancia; 
	}
	
private:
	
	/*
	Observe a genialidade disso:
	
	- Construtor e Destrutor bloqueados.
		N�o � poss�vel inicializar ou destruir uma inst�ncia.
	
	- C�pia e Atribui��o bloqueados.
		Isso violaria a unicidade.
	
	*/
	Singular(); 
	
	~Singular();
	
	Singular( Singular const& );
	
	Singular& operator=(Singular const&);
	
};

#endif  // SINGULAR_H
