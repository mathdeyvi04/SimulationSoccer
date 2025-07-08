from sobre_scripts.commons.Script import Script

script = Script(construtor_cpp=1)
args = script.args

# Importamos o agente correto.
if args.P:
    from Agent.AgentPenalty import Agent
else:
    from Agent.Agent import Agent

# Fazemos a criação do agente.
if args.D:
    player = Agent(args.i, args.p, args.m, args.u, args.t, True, True, False, args.F)
else:
    player = Agent(args.i, args.p, None, args.u, args.t, False, False, False, args.F)

# Colocamos o jogador em campo para executar.
while True:
    player.think_and_send()
    player.scom.receive()
