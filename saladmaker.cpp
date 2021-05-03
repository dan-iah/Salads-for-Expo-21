#include<iostream>
#include<fstream>
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
#include <random>

using namespace std;

//citations: stackoverflow.com

struct ingredient{
      int weight_v1;
      int weight_v2;
      int sm;
      int totalsalads;
   };

int main(int argc, char* const argv[]){

	//commanline arg vars, for saladmaking time, shared mem id, which saladmaer executing
   	int salmkrnum= 0;
   	int req=0;
   	int id =0;
   	float salmkrtime=0;
   	unsigned int rand_salmkr_time=0;
	int micro_time=0;
	int min_time=0;

	//for recording salad making time to show parallel executiom
    time_t now, beggining_now, completion_now;

    //recording start time of program exeuction
	beggining_now=time(0);
	struct tm st =*localtime(&beggining_now);

	//opening temporal log 'saladstats.log' to append when making salads below
	FILE *f;
	f=fopen("saladstats.log", "a+");
	if(f==NULL){printf("Error opening temporal log file\n");}


    //reading command line input after each specified flag
   	for(int i=0;i<argc;i++){ 
      if(strcmp(argv[i], "-s") == 0){id = stoi(argv[i+1]); req++; } //shared mem id
      else if(strcmp(argv[i], "-m") == 0){salmkrtime = stof(argv[i+1]); micro_time=salmkrtime*1000000; req++;} //salad making time 
      else if(strcmp(argv[i], "-n") == 0)
         {
            salmkrnum = stoi(argv[i+1]); req++;
            if(salmkrnum != 1 && salmkrnum != 2 && salmkrnum != 3){
               cout<<"Saladmaker number must be 1, 2 or 3"<<endl; exit(1);}
         }
   	}

   	//ensuring that all flags/neccessary input is provided
    if(req!=3){ //making sure all input is provided
      cout<<endl<<"Please provide command line input in the following format: "<<endl<<"./saladmaker -m salmkrtime -s shmid -n salmkrIdentifier(1,2 or 3)"<<endl;
      exit(1);}

    //Random Number Generation - to select salad making time from range 0.8*salmkrtime...salmkrtime
    auto gen=std::mt19937{std::random_device{}()};
    min_time=0.8*micro_time;
	auto dist=std::uniform_int_distribution<int>{min_time,micro_time};
	rand_salmkr_time=dist(gen);

    //struct attached to shared mem by chef initialized here to read struct variables in shared mem
    struct ingredient *ingr;
    ingr=(struct ingredient*)shmat(id, NULL, 0);


   /* semaphores */
   sem_t *chef;
   sem_t *sm1;
   sem_t *sm2;
   sem_t *sm3;
   //creating and initializing
   if((chef=sem_open("main_salad_semaphore", O_CREAT, 0644,1))==SEM_FAILED){
   	perror("semaphore initialization failed\n");
   	exit(1);
   }
   if((sm1=sem_open("sm1_semaphore",O_CREAT, 0644,1))==SEM_FAILED){
   	perror("semaphore initialization failed\n");
   	exit(1);
   }
   if((sm2=sem_open("sm2_semaphore",O_CREAT, 0644,1))==SEM_FAILED){
   	perror("semaphore initialization failed\n");
   	exit(1);
   }
   if((sm3=sem_open("sm3_semaphore",O_CREAT, 0644,1))==SEM_FAILED){
   	perror("semaphore initialization failed\n");
   	exit(1);
   }

   //vars for tracking number of salads made by each saladmaker
   int sm1_salads=0;
   int sm2_salads=0;
   int sm3_salads=0;

   //vars for tracking at the end the total weight of each veg received for each saladmaker
   //sm1
   int on_tom=0;
   int on_gp=0;
   //sm2
   int gp_tom=0;
   int gp_on=0;
   //sm3
   int tom_gp=0;
   int tom_on=0;

    //vars for tracking total time taken to make salads and wait for ingredients
	double work_t1, work_t2,wait_t1, wait_t2, work_cpu_time, wait_cpu_time;
	struct tms tb1, tb2;
	double ticspersec;
	ticspersec =(double)sysconf(_SC_CLK_TCK);

   //total number of salads to be made by all saladmakers
   int total=0;
   total=ingr[3].totalsalads;

   //beginning wait timer as soon as program begins execution and is waiting to receive ingredients from chef
   wait_t1 = (double) times(&tb1);

   while(1){

   //saladmaker 1 -> always has onions
   //saladmaker 2 -> always has green peppers
   //saladmaker 3 -> always has tomatoes

   //depending on which saladmaker has been specified in terminal (-n flag) the rest of the programs are blocked...
   //...while that program is executing once their semaphore is posted by the chef
   if(salmkrnum==1){sem_wait(sm1);}
   else if(salmkrnum==2){sem_wait(sm2);}
   else if(salmkrnum==3){sem_wait(sm3);}

   if(ingr[0].sm==1 && salmkrnum==1){ //saladmaker 1 - has supply of onion

   		//re-weighing vegies
   		//if weights of tomato and green pep less than required, do nothing
   		//i.e. reject them and wait for right weight to be received 
   		if(ingr[1].weight_v1<80 || ingr[2].weight_v2<50){} 

   		else{
   			//ending the waiting time previously started since the ingredients have been received
   			wait_t2 = (double) times(&tb2);
			wait_cpu_time += (wait_t2 - wait_t1) / ticspersec;

			//logging which salad out of the total num of salads specified in chef command line is being made
   			fprintf(f,"Salad #%d - saladmaker 1\n", (total-ingr[3].totalsalads)+1);
   			printf("Salad #%d - saladmaker 1\n", (total-ingr[3].totalsalads)+1);

   			//recording start time of saladmaking
   			now=time(0);
   			struct tm t =*localtime(&now);
			printf("Start time: %d:%d:%d\n", t.tm_hour,t.tm_min, t.tm_sec);
			fprintf(f,"Start time:%d:%d:%d\n", t.tm_hour,t.tm_min, t.tm_sec);
	   		work_t1 = (double) times(&tb1);
	  			   		
		   	//printing weights of vegies received from chef after theyve been reweighed
			printf("Received %dg of tomato\n", ingr[1].weight_v1); //tomato
			printf("Received %dg of green pepper\n", ingr[2].weight_v2); //green pepper
			fprintf(f,"Received %dg of tomato\n", ingr[1].weight_v1); //tomato
	   		fprintf(f,"Received %dg of green pepper\n", ingr[2].weight_v2); //green pepper

			//adding tomato and green pepper weight to total grams of tomato and gp used
			on_tom+=ingr[1].weight_v1;
			on_gp+=ingr[2].weight_v2;
			
			//chopping...
			printf("Chopping vegetables...\n");
			fprintf(f,"Chopping vegetables...\n");

			//salad making time
			usleep(rand_salmkr_time);

			//decrementing total num of salads that need to be made and incrementing num salads made by sm1
			ingr[3].totalsalads=ingr[3].totalsalads-1;
			sm1_salads++;
			printf("Salad maker 1 has created %d salad(s)\n",sm1_salads);
 
			//recording end time of salad making
			work_t2 = (double) times(&tb2);
			work_cpu_time += (work_t2 - work_t1) / ticspersec;
   			now=time(0);
			t =*localtime(&now);
			printf("End time: %d:%d:%d\n\n", t.tm_hour,t.tm_min, t.tm_sec);
			fprintf(f,"End time:%d:%d:%d\n\n", t.tm_hour,t.tm_min, t.tm_sec);
			
			//starting waititng time again until vegies are recieved from chef
			wait_t1 = (double) times(&tb1);
		}

   }
   	else if(ingr[0].sm==2 && salmkrnum==2){ //saladmaker 2 - has supply of green pepper

   		//re-weighing vegies
   		//if weights of tomato and onion less than required, do nothing
   		//i.e. reject them and wait for right weight to be received 
   		if(ingr[1].weight_v1<80 || ingr[2].weight_v2<30){} 

   		else{

   			//ending the waiting time previously started since the ingredients have been received
   			wait_t2 = (double) times(&tb2);
			wait_cpu_time += (wait_t2 - wait_t1) / ticspersec;

			//logging which salad out of the total num of salads specified in chef command line is being made
   			fprintf(f,"Salad #%d - saladmaker 2\n", (total-ingr[3].totalsalads)+1);
   			printf("Salad #%d - saladmaker 2\n", (total-ingr[3].totalsalads)+1);

   			//recording start time of saladmaking
   			now=time(0);
   			struct tm t =*localtime(&now);
			printf("Start time: %d:%d:%d\n", t.tm_hour,t.tm_min, t.tm_sec);
			fprintf(f,"Start time:%d:%d:%d\n", t.tm_hour,t.tm_min, t.tm_sec);
	   		work_t1 = (double) times(&tb1);
   			
		   	//printing weights of vegies received from chef after theyve been reweighed
			printf("Received %dg of tomato\n", ingr[1].weight_v1); //tomato
			printf("Received %dg of green pepper\n", ingr[2].weight_v2); //onion
			fprintf(f,"Received %dg of tomato\n", ingr[1].weight_v1); //tomato
	   		fprintf(f,"Received %dg of green pepper\n", ingr[2].weight_v2); //onion
			
			//adding tomato and onion weight to total grams of tomato and onion used			
	   		gp_tom+=ingr[1].weight_v1;
			gp_on+=ingr[2].weight_v2;
			
			//chopping...
			printf("Chopping vegetables...\n");
			fprintf(f,"Chopping vegetables...\n");

			//salad making time
			usleep(rand_salmkr_time); 

			//decrementing total num of salads that need to be made and incrementing num salads made by sm2
			ingr[3].totalsalads=ingr[3].totalsalads-1;
			sm2_salads++;
			printf("Salad maker 2 has created %d salad(s)\n",sm2_salads);

			//recording end time of salad making
			work_t2 = (double) times(&tb2);
			work_cpu_time += (work_t2 - work_t1) / ticspersec;
   			now=time(0);
   			t =*localtime(&now);
			printf("End time: %d:%d:%d\n\n", t.tm_hour,t.tm_min, t.tm_sec);
			fprintf(f,"End time:%d:%d:%d\n\n", t.tm_hour,t.tm_min, t.tm_sec);
			
			//starting waititng time again until vegies are recieved from chef
			wait_t1 = (double) times(&tb1);
		}
   	}

   else if(ingr[0].sm==3 && salmkrnum==3){ //saladmaker 3 - has supply of tomato

		//re-weighing vegies
   		//if weights of tomato and onion less than required, do nothing
   		//i.e. reject them and wait for right weight to be received
		if(ingr[1].weight_v1<50 || ingr[2].weight_v2<30){} 

		else{
			
			//ending the waiting time previously started since the ingredients have been received
			wait_t2 = (double) times(&tb2);
			wait_cpu_time += (wait_t2 - wait_t1) / ticspersec;

			//logging which salad out of the total num of salads specified in chef command line is being made
			fprintf(f,"Salad #%d - saladmaker 3\n", (total-ingr[3].totalsalads)+1);
			printf("Salad #%d - saladmaker 3\n", (total-ingr[3].totalsalads)+1);

			//recording start time of saladmaking
			now=time(0);
			struct tm t =*localtime(&now);
			printf("Start time: %d:%d:%d\n", t.tm_hour,t.tm_min, t.tm_sec);
			fprintf(f,"Start time:%d:%d:%d\n", t.tm_hour,t.tm_min, t.tm_sec);
			work_t1 = (double) times(&tb1);

			//printing weights of vegies received from chef after theyve been reweighed
			printf("Received %dg of tomato\n", ingr[1].weight_v1); //green pepper
			printf("Received %dg of green pepper\n", ingr[2].weight_v2); //onion
			fprintf(f,"Received %dg of tomato\n", ingr[1].weight_v1); //green pepper
			fprintf(f,"Received %dg of green pepper\n", ingr[2].weight_v2); //onion

			//adding green pepper and onion weight to total grams of green pepper and onion used	
			tom_gp+=ingr[1].weight_v1;
			tom_on+=ingr[2].weight_v2;

			//chopping...
			printf("Chopping vegetables...\n");
			fprintf(f,"Chopping vegetables...\n");

			//salad maker time
			usleep(rand_salmkr_time); 

			//decrementing total num of salads that need to be made and incrementing num salads made by sm3
			ingr[3].totalsalads=ingr[3].totalsalads-1;
			sm3_salads++;
			printf("Salad maker 3 has created %d salad(s)\n",sm3_salads);

			//recording end time of saladmaking
			work_t2 = (double) times(&tb2);
			work_cpu_time += (work_t2 - work_t1) / ticspersec;
			now=time(0);
			t =*localtime(&now);
			printf("End time: %d:%d:%d\n\n", t.tm_hour,t.tm_min, t.tm_sec);
			fprintf(f,"End time:%d:%d:%d\n\n", t.tm_hour,t.tm_min, t.tm_sec);

			//starting waititng time again until vegies are recieved from chef
			wait_t1 = (double) times(&tb1);
		}
	}

	
	if(ingr[3].totalsalads==0){
		if(salmkrnum==1){ //onion
			if(sm1_salads==0){
				wait_t2 = (double) times(&tb2);
				wait_cpu_time += (wait_t2 - wait_t1) / ticspersec;
			}
			completion_now=time(0);
			fprintf(f,"FINAL STATS - Salad Maker 1\n");
			fprintf(f,"Program start time: %d:%d:%d\n", st.tm_hour,st.tm_min, st.tm_sec);
			struct tm ct =*localtime(&completion_now);
			fprintf(f,"Program end time: %d:%d:%d\n", ct.tm_hour,ct.tm_min, ct.tm_sec);
			fprintf(f,"Total salads made:  %d\n", sm1_salads);
			fprintf(f,"Total time making salads (sec): %f\n",work_cpu_time);
	   		fprintf(f,"Total time waiting for ingredients (sec): %f\n",wait_cpu_time);
	   		fprintf(f,"%dg tomato used\n",on_tom);
	   		fprintf(f,"%dg green pepper used\n\n",on_gp);
			sem_post(sm2);sem_post(sm3);sem_post(chef);
		}
	   	else if(salmkrnum==2){ //green pepper
	   		if(sm2_salads==0){
				wait_t2 = (double) times(&tb2);
				wait_cpu_time += (wait_t2 - wait_t1) / ticspersec;
			}
	   		completion_now=time(0);
	   		fprintf(f,"FINAL STATS - Salad Maker 2\n");
	   		fprintf(f,"Program start time: %d:%d:%d\n", st.tm_hour,st.tm_min, st.tm_sec);
	   		struct tm ct =*localtime(&completion_now);
			fprintf(f,"Program end time: %d:%d:%d\n", ct.tm_hour,ct.tm_min, ct.tm_sec);
	   		fprintf(f,"Total salads made:  %d\n", sm2_salads);
			fprintf(f,"Total time making salads (sec): %f\n",work_cpu_time);
	   		fprintf(f,"Total time waiting for ingredients (sec): %f\n",wait_cpu_time);
	   		fprintf(f,"%dg tomato used\n",gp_tom);
	   		fprintf(f,"%dg onion used\n\n",gp_on);
	   		sem_post(sm1);sem_post(sm3);sem_post(chef);
	   	}
	   	else if(salmkrnum==3){ //tomato
	   		if(sm3_salads==0){
				wait_t2 = (double) times(&tb2);
				wait_cpu_time += (wait_t2 - wait_t1) / ticspersec;
			}
	   		completion_now=time(0);
	   		fprintf(f,"FINAL STATS - Salad Maker 3\n");
	   		fprintf(f,"Program start time: %d:%d:%d\n", st.tm_hour,st.tm_min, st.tm_sec);
	   		struct tm ct =*localtime(&completion_now);
			fprintf(f,"Program end time: %d:%d:%d\n", ct.tm_hour,ct.tm_min, ct.tm_sec);
	   		fprintf(f,"Total salads made:  %d\n", sm3_salads);
			fprintf(f,"Total time making salads (sec): %f\n",work_cpu_time);
	   		fprintf(f,"Total time waiting for ingredients (sec): %f\n",wait_cpu_time);
	   		fprintf(f,"%dg green pepper used\n",tom_gp);
	   		fprintf(f,"%dg onion used\n\n",tom_on);
	   		sem_post(sm1);sem_post(sm2);sem_post(chef);
	   	}
  		

  		//detaching from shared memory segment
  		shmdt(ingr);
  		
  		//close and unlink named semaphores
	    sem_close(chef);
	    sem_unlink("/main_salad_semaphore");
	    sem_close(sm1);
	    sem_unlink("/sm1_semaphore");
	    sem_close(sm2);
	    sem_unlink("/sm2_semaphore");
	    sem_close(sm3);
	    sem_unlink("/sm3_semaphore");
  		exit(1);
  	}

  	//after saladmaker is done making salad, post chef semaphore for chef process to resume execution
  	sem_post(chef);
		
  }

   return 0;

}