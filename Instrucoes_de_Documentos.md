# Para Competição Brasileira de Robótica

Todo ano a CBR e a RoboCup pedem por este documento, o Team Description Paper.

Como não sabemos se o site continuará, apresentarei o conteúdo 
disponibilizado no [site oficial da CBR](https://cbr.robocup.org.br/index.php/papers-tdps/):

> Os TDPs devem ser escritos em Inglês ou Português.
> 
> Os times que não enviarem um TDP ou que tiverem seu TDP
> recusados, **não** poderão participar do evento mesmo que
> tenham realizado o pagamento da inscrição. Os valores
> pagos pelas inscrições **não** serão devolvidos.
> 
> Times da mesma instituição de ensino, mas que participam
> em categorias diferentes, devem inscrever um TDP diferente
> para cada categoria.
> 
> O TDP deve descrever as ideias implementadas pelo time.
> Você pode descrever todo o sistema ou focar em um ou dois
> componentes – nós recomendamos que os tópicos inovadores
> e importantes ou abordagens alternativas adotadas sejam
> incluídos no TDP, pois o mesmo problema e os tópicos que
> podem interessar as outras equipes.
> 
> É muito importante que as explicações sejam feitas de
> forma clara e objetiva, partindo do princípio que os
> leitores estão familiarizados a tópicos relacionados
> a categoria (descrição, regras, etc). Foque seu texto
> nos tópicos importantes do seu trabalho. Ele deve explicar
> os detalhes técnicos sobre como sua abordagem funciona para
> que outros pesquisadores possam replicar seus experimentos
> e resultados. Assim, todos os pesquisadores podem se
> beneficiar das experiências apresentadas.
> 
> Máximo: 6 páginas por TDP.

Segue um exemplo de [TDP desenvolvido pela equipe BahiaRT](https://github.com/user-attachments/files/19074305/BahiaRT_TDP.pdf).

[Template do TDP](https://github.com/user-attachments/files/19074329/Robocup-Brazil-Template-1.zip)
a ser desenvolvido pelas equipes e disponibilizado pela CBR.

Cada equipe deve fazer um pré-registro no [Sistema Olimpo](https://olimpo.robocup.org.br/)
dentro do período especificado. Para realizar o pré-registro, siga estas etapas:

* Crie sua conta no Novo Sistema Olimpo
* Vincule a uma ou mais instituições na opção: Gerenciar Instituições
* Escolha a opção: Eventos > Eventos Abertos> CBR 2025 e escola a sua categoria. Siga as orientações.


# Para RoboCup

Um pouco diferente...

### Documento de Descrição da Equipe (TDP)

O TDP deve descrever seu foco de pesquisa e ideias implementadas na equipe.
Ele deve descrever claramente seu próprio trabalho e suas contribuições, 
além de especificar explicitamente o que você usou dos esforços de outros 
(incluindo, mas não se limitando a, qualquer código-fonte lançado por outras 
equipes ou seu trabalho científico). Na qualificação, as equipes devem ser 
julgadas com base em seu próprio trabalho, portanto, deixar de reconhecer o 
trabalho de outros pode resultar em desqualificação imediata. O comprimento 
do TDP deve ser de pelo menos quatro (4) páginas e não deve exceder doze (12)
páginas no estilo Springer LNCS. Não inclua TDPs que você enviou para a RoboCup
em anos anteriores.

Envie o TDP apenas como um documento PDF, com o nome da sua equipe no nome do 
arquivo, ou seja, Teamname_TDP.pdf.

### Repertório

Uma lista de publicações e conquistas em simpósios anteriores da RoboCup
e em outros periódicos e conferências internacionais relevantes. Inclua
também as conquistas da sua equipe na RoboCup e eventos relacionados de
anos anteriores. Se você é novo na comunidade RoboCup 3D, também pode
incluir referências a pesquisas relevantes feitas pela sua equipe que
mostrem seu potencial. Envie a lista de contribuições como um documento
PDF, com o nome da sua equipe no nome do arquivo, ou seja, Teamname_list.pdf.


### Arquivo Binário

Todas as equipes devem enviar um binário funcional de sua equipe.
O Comitê usará esses binários para jogar partidas de 11 contra 11,
sob as regras e com o simulador usado durante as competições de 2024.

Os binários enviados devem aderir ao seguinte:

* Dois scripts devem ser incluídos: um script de inicialização, chamado start.sh, para executar uma equipe completa de agentes,
e um script de eliminação, chamado kill.sh, para eliminar todos os agentes da equipe. Os requisitos e exemplos desses scripts
podem ser encontrados na página de regras.

* Todas as bibliotecas externas necessárias devem ser incluídas e usadas localmente pelo binário. O OC não fará nenhum esforço
para instalar bibliotecas extras nos sistemas de qualificação. Pode-se presumir que um tempo de execução Java esteja disponível.

* Os binários não devem criar nenhuma saída, seja por meio de saída padrão ou para arquivos, e nenhuma interface gráfica (depuração)
deve ser usada.

* O binário deve ser compilado para sistemas de 64 bits e deve funcionar em uma distribuição GNU/Linux moderna,
como o Ubuntu 24.04. Você também pode enviar binários de 32 bits, mas é sua responsabilidade garantir que ele rode
corretamente em distribuições de 64 bits. Você pode assumir que bibliotecas básicas de 32 bits (por exemplo, libc)
estão instaladas no Sistema Operacional de 64 bits. Para resumir: o binário deve rodar imediatamente em um sistema
padrão, limpo e sem interface em um ambiente sandbox restrito com o servidor de simulação (possivelmente) rodando
em uma máquina diferente. O Comitê não tentará corrigir erros. **Quando um binário não roda, a respectiva equipe será
notificada e terá que reenviar seu material, antes do prazo**.

### Entrega

Coloque todo o material de qualificação em uma pasta com o nome da sua equipe, crie um tarball chamado teamname.tar.gz
e carregue-o no formulário de inscrição que será apresentado.

