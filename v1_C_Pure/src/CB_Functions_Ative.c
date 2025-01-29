#include "CB_Functions_Ative.h" 

int 
close_enough(
	Player *playable
){
	/*
	Description:
		Verify if the player is close enough to the ball.
		
		If sucess:
			Returns 1.
		
		else:
			Returns 0.
	*/
	
	double part_1 = ((*playable).pos[0] - playables[0].pos[0]) * ((*playable).pos[0] - playables[0].pos[0]);
	double part_2 = ((*playable).pos[1] - playables[0].pos[1]) * ((*playable).pos[1] - playables[0].pos[1]);
	
	if(
		(part_1 + part_2) < MIN_POW_DIST_2_FOR_CATCH
	){	
		return 1;
	}
	
	return 0;
}


int
kick_ball(
	int force
){
	/*
	Description:
		Once you have the last mouse position, kick the ball with force in its direction.
	*/
	
	// Releases the ball.
	*ball_dominator = 0;
	
	double verser_ball_to_mouse[2] = {
		last_position_mouse[0] - playables[0].pos[0],
		last_position_mouse[1] - playables[0].pos[1]
	};
	
	double module_direction = sqrt(
		get_module_squared(
			verser_ball_to_mouse
		)
	);
	
	verser_ball_to_mouse[0] /= module_direction;
	verser_ball_to_mouse[1] /= module_direction;
	
	playables[0].vel[0] = COEF_KICK_CHARGE * (*kick_charge) * verser_ball_to_mouse[0];
	playables[0].vel[1] = COEF_KICK_CHARGE * (*kick_charge) * verser_ball_to_mouse[1];
	
	*kick_charge = 0;
	
	return 0;
}


int 
verify_goal(){
	/*
	Check if it is a goal.
	
	If yes, returns 1.
	*/
	
	if(
		(
			playables[0].pos[0] <= TOPLEFT_X
		) && (
			playables[0].pos[1] >= TOP_GOAL
		) && (
			playables[0].pos[1] <= BOTTOM_GOAL
		)
	){
		// Esquerdo
		
		goals[0]++;
		
		return 1;
	}
	
	if(
		(
			playables[0].pos[0] >= BOTTOMRIGHT_X
		) && (
			playables[0].pos[1] >= TOP_GOAL
		) && (
			playables[0].pos[1] <= BOTTOM_GOAL
		)
	){
		// Direito
		
		goals[1]++;
		
		return 1;
	}
	
	return 0;
}


////////////////////////////////////////////////
/// The Goal
////////////////////////////////////////////////

int 
get_decision_player(
	Player *playable
){
	/*
	Description:
		Function responsible for representing the player's decision criteria.

		Each decision will have its own return integer.
	*/
	
	if(
		close_enough(
			playable
		)
	){
		return 1;
	}
	
	return 0;
}


















