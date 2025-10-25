### Alerta

Tanto a competição quanto o bom-senso geral estabelece que o uso direto dos binários
de outra equipe configura inválidez, dado o mau-caráter, além de roubo de propriedade
intelectual da mesma.

Portanto, afirma-se que o que será tratado neste local se trata apenas de um estudo
dirigido, focado no conhecimento disponibilizado publicamente pela equipe e explicado, aprimorado e compreendido pelo mesmo que vos escreve.

---

# Visão Geral do Código

Há duas opções de documentação para o código-fonte, seja o original da equipe
ou o modificado da RoboIME.

* [Documentação Disponbilizada pela Equipe](https://docs.google.com/document/d/1aJhwK2iJtU-ri_2JOB8iYvxzbPskJ8kbk_4rb3IK3yc/edit?tab=t.0)

	* Em Inglês e escrita em Word.

    * [Código Completo Construído pela Equipe](https://github.com/m-abr/FCPCodebase)

        * Repositório da equipe responsável.
  	
	* Acredito que seja relevante também apresentar o [canal](https://www.youtube.com/@m_abreu/videos) representante da equipe, no qual é disponibilizado apresentações interessantes.

* [Documentação Traduzida e Explicada Por Mim para Você](Visao_Superficial.md)

	* Explicação Base de Diversos Componentes Base da Aplicação.

	* [Dissertação sobre Pastas e Arquivos](docs/readme.md)
    
		* Como ocorre a distrbuição de arquivos e de módulos, e suas respectivas responsabilidades.

---

# Principais Possíveis Melhorias Observadas

* ### Tratamento de Importações

Alguns arquivos, como os presentes em [communication](src/communication) ou em [sobre_behaviors](src/sobre_behaviors),
possuem uma gambiarra de importação, pois o original gostaria de evitar importações circulares e preservar o 
type hinting.

* ### Inovações 

A aplicação apresenta baixa extensibilidade para a introdução de novas funcionalidades, apesar da existência,
por exemplo, de possibilidades de extensão em [Behavior](src/sobre_behaviors/Behavior.py).

Poderia se introduzir um caminho específico a ser trilhado que permitiria a criação e implementação
de novas funcionalidades e comportamentos para os robôs.

* ### Lógica de Conclusão

Atualmente, todos os comportamentos representados por classes, como `GetUp` ou `Fall`, possuem
um método `is_ready()`, o qual não possui nenhuma lógica, sempre retorna True.

A ideia é desenvolver uma métrica capaz de realmente saber se está pronto.


















