from sobre_scripts.commons.Script import Script

script = Script()

a = script.args

from Agent.Agent import Agent

# Args: Server IP, Agent Port, Monitor Port, Uniform No., Team name, Enable Log, Enable Draw
team_args = ((a.i, a.p, a.m, u, a.t, True, True) for u in range(1,12))
script.batch_create(Agent, team_args)

while True:
    script.batch_execute_agent()
    script.batch_receive()
