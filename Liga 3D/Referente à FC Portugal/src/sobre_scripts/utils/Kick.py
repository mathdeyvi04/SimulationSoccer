from Agent.BaseAgent import BaseAgent
from math_ops.GeneralMath import GeneralMath
from sobre_scriptsA.commons.Script import Script
import numpy as np


class Kick:
    def __init__(self, script: Script) -> None:
        self.script = script

    def execute(self):

        a = self.script.args
        player = BaseAgent(a.i, a.p, a.m, a.u, a.r, a.t)  # Args: Server IP, Agent Port, Monitor Port, Uniform No., Robot Type, Team Name
        player.path_manager.draw_options(enable_obstacles=True, enable_path=True)  # enable drawings of obstacles and path to ball
        behavior = player.behavior
        w = player.world
        r = w.robot

        print("\nThe robot will kick towards the center of the field")
        print("Try to manually relocate the ball")
        print("Press ctrl+c to return\n")

        player.scom.unofficial_set_play_mode("PlayOn")
        player.scom.unofficial_beam((-3, 0, r.beam_height), 0)
        vec = (1, 0)

        while True:
            player.scom.unofficial_set_game_time(0)
            b = w.ball_abs_pos[:2]

            if 0 < np.linalg.norm(w.get_ball_abs_vel(6)) < 0.02:  # speed of zero is likely to indicate prolongued inability to see the ball
                if np.linalg.norm(w.ball_rel_head_cart_pos[:2]) > 0.5:  # update kick if ball is further than 0.5 m
                    if max(abs(b)) < 0.5:
                        vec = np.array([6, 0])
                    else:
                        vec = GeneralMath.normalize((0, 0) - b) * 6

                w.draw.point(b + vec, 8, w.draw.Color.pink, "target")

            behavior.execute("BasicKick", GeneralMath.angle_horizontal_from_vector2D(vec))

            player.scom.commit_and_send(r.get_command())
            player.scom.receive()

            if behavior.is_ready("GetUp"):
                player.scom.unofficial_beam((*r.loc_head_position[0:2], r.beam_height), 0)
                behavior.execute_to_completion("Zero_Bent_Knees")


