from sobre_scripts.commons.Script import Script

script = Script() 

a = script.args

from Agent.Agent import Agent

# Args: Server IP, Agent Port, Monitor Port, Uniform No., Team name, Enable Log, Enable Draw
script.batch_create(Agent, ((a.i, a.p, a.m, a.u, a.t,        True, True),)) 
script.batch_create(Agent, ((a.i, a.p, a.m, a.u, "Opponent", True, True),)) 


while True:
    script.batch_execute_agent()
    script.batch_receive()