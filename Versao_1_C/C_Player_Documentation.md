# Index's playable:
* Responsible for detail what kind of playable is.

 
    For example:

       i = 0 -> Ball;
 
       i = 1, ..., 10 -> Player's Soccer;

       i = 11 -> Goalkeeper;

       ...

# Player's Generation:

* In the real world, the couch organizes the team in the configuration's game.


    For this, we split the image field in one matrix, each column and row has the width
    and height of the player's soccer. With this, we can put the player in locations specifics.

    For example:
        
        players_location = {
            {
                // Time A
                {a_0, b_0}, 
                {a_1, b_1},
                ...
            },
            {
                // Time B
                ...
            }
        }
    
    For pratices, we can use random locations for while.
	
# Player's Interations:
	We divide one thread for each team, in fact this improves the performance.
	
	We will divide the develop in some develop lines:
	
		First -> Without Interations Between Player's Soccer
		
			Each thread calculates the result force/impulse in the ball.
			After this, sum up the result in the ball movement.
			
			Observation:
				Do this for not using the mutex, because the mutex would not
				allow the parallel programming.
			
			In this field, despite allow the interation with the ball, there's
			only one player in the field soccer and the user will control him.
            Of course we will test more players.
			
		Second -> Allow Interations Between Player's Soccer
		
			In this field, despite allow the interation with the ball, there's
			only one player in the field soccer and the user will control him.
            Of course we will test more players.
		
		Third -> 'Smart' Players

            Finally, we put all the players in the field soccer and eliminates the control's user.
            
            We separates the third develop line in sections of AI.

# Smart Players

* We divide the sections in folders. Each folder has initially the basic setup feature,
in another words, the first and second develop lines.

  * First Section: **Random Actions**
  * Second Section: **Neural Network**
  * Third Section: **(Deep?) Neural Network**
			
		
			
	
	
	
	
	
	
	

