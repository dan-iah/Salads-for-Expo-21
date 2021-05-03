# Salads-for-Expo-21
In this project, virtual salads were created through coordinating between independent chef and saladmaker processes using semaphores and shared memory. The chef program is in charge of randomly selecting two ingredients of three (tomato, green pepper and onion) and randomly generating a weight for the two vegies to send over to the salad maker that needs them (each saladmaker is always in need of the same two ingredients). The chef writes the weights and type of the vegies into variables in a strcut attahced in the shared memeory segment. The saladmakers reads those values and ensures the weights are enough to make the salad. There are 4 named semaphores (chef, sm1,sm2,sm3) to ensure single process access to shared memory at a time. Stats for the salad are produced with information on total weight of vegies used, time taken making the salad for ingredients, start and end time of program execution and are logged into a temporal log file created by the program called 'saladstats.log'.

To run the chef and saladmaker programs:
1. Open terminal 
2. Change the directory to where program files chef and saladmaker are stored
3. Type: 

\>\> make

Use 4 terminals to run chef and saladmakers programs
One terminal for each salad maker with a -n flag specifying which saldmaker is executing.
(flags can appear in any order)

Terminal 1: 
\>\> ./chef -n numOfSalads -m cheftime

Terminal 2: 
>> ./saladmaker -m salmkrtime -n 1 -s shmid

Terminal 3: 
\>\> ./saladmaker -m salmkrtime -n 2 -s shmid

Terminal 4: 
\>\> ./saladmaker -m salmkrtime -n 3 -s shmid

--------------------------------------------------

The Chef Program 

The chef program creates the shared memory segment which the saladmaker program (for each saladmaker) will attach to to access the variables. A structure is created and accessed as an array where variables weight_v1, weight_v2, sm, and totalsalads are stored. weight_v1 and weight_v2 are the weights of the pair of vegetables that are randomly generated by the chef and given to the saladmaker that requires that specififc pair. sm identifies to which saladmaker these weights will be going to (and accordingly chef program performs sem_post for that saladmaker to begin execution) (which saladmaker needs tomato and onion, or green pepper and tomato, etc). totalsalads stores the total number of salads that needs to be made by all saladmakers and is decremented each time a salad is made. The program terminates once totalsalads==0. The chef also takes a break in between distributing pairs of veges for time cheftime assigned at terminal. Finally the chef program removes the shared mem segement and unlinks and closes the names semaphores.

The Saladmaker Program

The saladmaker program is one cpp file that contains if/else statements specifying which saladmaker is going to execute for the specific pair of ingredients it receives. The saladmaker program accesses the shared memory segment to obtain the weights of vegies and to know which saladmaker they are given (i.e. which saladmaker is to be executed). Before salad making begins, sem_wait() is called for the saladmaker receiving the ingredients (sm1 or sm2 or sm3) to block all other processes from accessing shared memory segment as it will be accessing it. Finally, saladmaker program prints the total working and waiting time stats and total vegie weights and time of program from beginning of execution till end and unlinks/closes the semaphores and detaches from shared memory.

Temporal Log

The log file "saladstats.log" stores info for each salad being made, and final stats for each salad maker. For each salad, 'saladstats.log' records the time each saladmaker has begun making its salad, the weights it received, the chopping of the vegies, and the end time of that salad creation. The final stats after each saladmaker has made all salads based on random distrubiton of ingredients are printed in the log with info on the total weight used for each pair of vegies, total time spent making the salads and waiting for the ingredients, beginning time of execution of the program and complete end time of program execution, as well as number of salads made by that saladmaker.





