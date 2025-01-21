#include "A_Simulando.h"

extern Player playables[NUMBER_OF_PLAYERS];

extern pthread_barrier_t coachs_command_flow;
	
int
main(
	/*
	How many arguments were passed.
	The .exe name already counts as 1 argument.
	*/
	int argc,
	/*
	Contains the arguments passed.
	Each element of the array is a string representing an argument.
	*/
	char* argv[]
){
	setlocale(LC_ALL, "");
	
	Display display = initialize_display();
	
	if (
		(display.window == NULL) || (display.renderer == NULL)
	){
		destroy_display(
			display
		);
		
		display_error(
			"\n(A) An error occurred while initializing the window."
		);
		
		return 1;
	}
	
	pthread_barrier_init(
		&coachs_command_flow,
		NULL,
		NUMBER_OF_COACHS
	);
	
	generate_players(
		playables
	);
	
	int simulation_is_running = 1;
	
	while(
		simulation_is_running
	){
		input_user(
			&simulation_is_running
		);
		
		update();
		
		render(
			display
		);
		
	}
	
	destroy_display(
		display
	);
    
    pthread_barrier_destroy(
		&coachs_command_flow
	);
    
    printf("\n\nEncerrado Com Sucesso.\n");
	system("pause");
    return 0;
}

