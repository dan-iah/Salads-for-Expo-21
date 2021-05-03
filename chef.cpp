#include<iostream>
#include<cstring>
#include<cstdlib>
#include<ctype.h>
#include<string.h>
#include <sys/times.h> 
#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>  
#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <sys/msg.h> 
#include <time.h> 
#include <stdlib.h> 
#include<string>
#include<random>

using namespace std;

//citations: stackoverflow.com

//struct to attach to shared mem segement saving neccessary vars
struct ingredient{
		//weights of vegies for each salad maker
		int weight_v1; 
		int weight_v2;
		//defining which sm vegies are sent to
		int sm; 
		//var for total num of salads to be made (decremented when each sm makes a salad)
		int totalsalads; 
	};

int main(int argc, char* const argv[]){
	int num_salads= 0;
	int id=0;
	int req=0;
	float chef_time=0;
	unsigned int rand_chef_time=0;
	int micro_time=0;
	int min_time=0;
	srand(time(NULL));

	//reading command line input after each specified flag
	for(int i=0;i<argc;i++){ 
		if(strcmp(argv[i], "-n") == 0){num_salads = stoi(argv[i+1]); req++;}
		else if(strcmp(argv[i], "-m") == 0){chef_time = stof(argv[i+1]); micro_time=chef_time*1000000; req++; }	
	}
	if(req!=2){
		cout<<endl<<"Please provide command line input in correct format: "<<endl<<"./chef -n numOfSalads -m cheftime"<<endl;
		exit(1);}

	//removing file so that when it is opened in append mode it is clear of contents
	remove("saladstats.log");

	//Random Number Generation - to select vegies for saladmaker and for chef time
    auto gen=std::mt19937{std::random_device{}()};

   	//generating random number within range for max waiting time for chef
    min_time=0.5*micro_time;
	auto dist=std::uniform_int_distribution<int>{min_time,micro_time};
	rand_chef_time=dist(gen);
	
	/* shared memory segment */
	id = shmget (IPC_PRIVATE ,sizeof(struct ingredient)*100 ,0666) ; 
	//status of shared mem creation
	if (id == -1){perror ("Creation error");}
	else{printf ("\nAllocated: %d\n" , id);}

	//attaching struct to shared mem segment
	struct ingredient *ingr;
	ingr=(struct ingredient*)shmat(id , NULL, 0);
   
   /* semaphores */
   sem_t *chef;
   sem_t *sm1;
   sem_t *sm2;
   sem_t *sm3;

   //creating semaphores
   	if((chef=sem_open("main_salad_semaphore",O_CREAT, 0644,1))==SEM_FAILED){
   		perror("semaphore initialization failed\n");
   		exit(1);}
   	if((sm1=sem_open("sm1_semaphore",O_CREAT, 0644,0))==SEM_FAILED){
   		perror("semaphore initialization failed\n");
   		exit(1);}
   	if((sm2=sem_open("sm2_semaphore",O_CREAT, 0644,0))==SEM_FAILED){
   		perror("semaphore initialization failed\n");
   		exit(1);}
   	if((sm3=sem_open("sm3_semaphore",O_CREAT, 0644,0))==SEM_FAILED){
   		perror("semaphore initialization failed\n");
   		exit(1);}

   	//initializing semaphore values
   	sem_init(chef,1,1);
   	sem_init(sm1,1,0);
   	sem_init(sm2,1,0);
   	sem_init(sm3,1,0);

   	//initializing neccessary variables to send vegies to saladmakers
   	time_t t;
   	int gp_weight=0;
   	int tom_weight=0;
   	int onion_weight=0;
   	int salmkrnum=0;

    //setting totalsalads var in struct in shared mem to the user specified total num in command line
   	ingr[3].totalsalads=num_salads;
   
   	//Sending veggies to saladmakers and looping until all salads made (while loop breaks when all salads made)
   	while(1){

   		//chef is accessing shared memory, so blocking access to saladmakers
		sem_wait(chef); 
		
		//randomly selecting vegies to send to saladmaker that needs that pair
   		//1: onions, 2: green peppers, 3: tomatoes
		auto dist=std::uniform_int_distribution<int>{1,3};
		salmkrnum=dist(gen);
		ingr[0].sm=salmkrnum;

		//generate a random num for two vegs being passed within range of weight
		//approx 30g of onions, 80g of tomatoes, 50g of green peppers
		//pass two veg weights to saladmaker that needs those vegs)
	   if(salmkrnum==1){ //onion
	   	tom_weight=(0.8*80)+(rand()%(96-64+1)); ingr[1].weight_v1=tom_weight; 
	   	gp_weight=(0.8*50)+(rand()%(60-40+1)); ingr[2].weight_v2=gp_weight;
	   }
	   else if(salmkrnum==2){ //green pepper
	   	int tom_weight=(0.8*80)+(rand()%(96-64+1)); ingr[1].weight_v1=tom_weight; 
	   	int onion_weight=(0.8*30)+(rand()%(36-24+1)); ingr[2].weight_v2=onion_weight;
	   }
	   else if(salmkrnum==3){ //tomato
	   	gp_weight=(0.8*50)+(rand()%(60-40+1)); ingr[1].weight_v1=gp_weight; 
	   	onion_weight=(0.8*30)+(rand()%(36-24+1)); ingr[2].weight_v2=onion_weight;
	   }

	   else{
	   	printf("error");
	   }

	   //terminating condition, if all salads made (counter==0), end execution
	   if(ingr[3].totalsalads==0){
	   		//detaching from shared memory segment
	  		shmdt(ingr);
	  		shmctl(id, IPC_RMID, NULL);

	  		//close and unlink named semaphores
		    sem_close(chef);
		    sem_unlink("/main_salad_semaphore");
		    sem_close(sm1);
		    sem_unlink("/sm1_semaphore");
		    sem_close(sm2);
		    sem_unlink("/sm2_semaphore");
		    sem_close(sm3);
		    sem_unlink("/sm3_semaphore");
	  		printf("All %d salads made.\nCreated temporal log with final stats is 'saladstats.log'.\n\n",num_salads);
	  		exit(1);
  		}

  		//can be used to check what weights are being sent (saladmakers will reweigh them to check they are the required weight)
		//printf("Sent veg 1: %d\n", ingr[1].weight_v1);
		//printf("Sent veg 2: %d\n\n", ingr[2].weight_v2);

		//once chef done accessing shared memory, saladmakers can
		if(salmkrnum==1){sem_post(sm1);}
		else if(salmkrnum==2){sem_post(sm2);}
		else if(salmkrnum==3){sem_post(sm3);}

		//chef break
		usleep(rand_chef_time); 

	}

	return 0;

}