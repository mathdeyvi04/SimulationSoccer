# Coach Threads

> Refers to the threads that manage players.

# Purpose

* Standby (0)

> At First:
> 
>   * To ensure that before applying, all players have been created.
>
> Others:
>   * For thread synchronization.

* Standby (1)

> Allow main to receive and update values that will be used by the coach threads.

* Standby (2)

> Allows coach threads to calculate, update and draw players.


# Flow

>* Main spawns ball and creates coach threads, Immediately after, Main goes into standby (0).

>* Coach threads generate their respective team and which releases main. These threads go into standby (1) again.

>* While coach threads are waiting in standby (1), main starts:
>   * Pre-rendering;
>   * Receives input from the user;
>   * Get ready for timelapse.
> 
>* Main thread release coach threads that are in standby (1). Main goes to standby (2).

>* While Main thread is waiting in standby (2):
>   * Calculates each interation between players and ball;
>   * Calculates the moviment each player;
>   * Under mutex, draw each player on the screen.
>   * 
>* Coach threads releases main thread from standby (2). These go to standby (0).

>* Main thread update and render ball. After, releases the standby (0).
 
>* Threads repeat loop.





