# Index's playable:

* Responsible for detail what kind of playable is.

For example:
    
* i = 0:
  * Ball

* i = 1, ..., 10:
  * Player's Soccer and one GoalKeeper

* ...

# Player's Generation:

* In the real world, the couch organizes the team in the configuration's 
game.


For this, we split the image field in one matrix, each column and
row has the width and height of the player's soccer. With this, we
can put the players in locations specifics.

For example:
    

````C
players_location = {
    {
        // Time A
        {x_0, y_0}, 
        {x_1, y_1},
        ...
    },
    {
        // Time B
        ...
    }
}
````

In the future, we ideally want to have a new interface asking the 
user to enter the appropriate desired locations.

# Player's Interations:

We divide one thread for each team, to improve the performance.

We will divide to develop in some develop phases:

### Without Interations Between Player's Soccer: v0_C

* Each thread calculates the respective result force/impulse in the
ball. After this, sum up the result in the ball's movement.


* In this field, despite allow the interation with the ball, there's
only one player in the field soccer and the user will control him.
Of course, we will test more players.

### Allow Interations Between Player's Soccer (?)

* In the real world, the field is too big and interactions between
players away from the ball become minimal.


* Players focus on the ball and run towards it in order to take 
control of it from other players. In addition, other possible
interactions are marking between players from opposing teams and 
fouls, but these are beyond our current objectives.


### 'Smart' Players

* Finally, we put all the players in the field soccer and eliminates
the control's user.

* We separate the third phase in sections of AI.

# Smart Players

* We divide the versions in folders. Each folder has initially the
basic setup feature, Pure version, which has no input's user.

* We will focus in to three types of intelligence:

  1. **Random Actions**
  2. **Neural Network**
  3. **(Deep?) Neural Network**
  

* As the project grows and other developers join, more forms of 
intelligence will be used and other methods of enhancement will also
be tested.
