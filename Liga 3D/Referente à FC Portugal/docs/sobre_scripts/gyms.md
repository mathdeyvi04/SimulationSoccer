# gyms

A pasta `gyms` contém ambientes customizados desenvolvidos no padrão OpenAI Gym, focados em
simulações de controle e aprendizado por reforço aplicados a robôs.

O objetivo da pasta é fornecer uma coleção modular de ambientes de simulação que possam ser
utilizados para treinar, testar e comparar algoritmos de aprendizado por reforço em diferentes
tarefas e comportamentos robóticos.

## Como adicionar novos comportamentos

Para adicionar um novo comportamento à pasta `gyms`, siga os passos abaixo:

1. **Crie uma nova classe de ambiente**  
   Implemente uma nova classe seguindo o padrão das já existentes (`Fall`, `BasicRun`), herdando de `gym.Env` e implementando os métodos essenciais: `__init__`, `reset`, `step`, `observe`, `sync`, `render` e `close`.

2. **Defina os espaços de observação e ação**  
   Configure adequadamente os atributos `observation_space` e `action_space` para refletir as características do novo comportamento.

3. **Implemente a lógica do comportamento**  
   No método `step`, inclua a lógica específica da tarefa que o ambiente irá simular, bem como o cálculo da recompensa e das condições de término do episódio.

4. **Adapte o fluxo de simulação**  
   Se necessário, implemente métodos auxiliares para sincronizar, estabilizar ou coletar observações do robô.

5. **Documente o comportamento**  
   Inclua docstrings nos métodos e uma breve explicação no início da nova classe, detalhando seu propósito e suas particularidades.

6. **Teste e integre**  
   Teste o novo ambiente usando exemplos ou scripts de treinamento já existentes, garantindo sua compatibilidade e funcionamento adequado dentro da estrutura da pasta.
