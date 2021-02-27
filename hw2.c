/*
Tarih: 30.04.2020
Beraat BUZ 
150160002
"gcc -pthread  hw2.c -o out" ile SSH üzerinden derledim.
"./out 101 200 2 2" ile çalýþtýrdým

*/
#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include <string.h>
#include <stdbool.h> 
#include<sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#define _GNU_SOURCE
//KEYSEM main de tanýmladým
int sem_key;
int * globalcp;

//Signal fonksiyonlarý
void sem_signal (int semid ,int val) {
	struct sembuf semaphore ;
	semaphore.sem_num=0;
	semaphore.sem_op=val ;
	semaphore.sem_flg=1; 
	semop( semid , &semaphore , 1 ) ;
}

void sem_wait(int semid , int val ) {
	struct sembuf semaphore ;
	semaphore.sem_num=0;
	semaphore.sem_op=(-1*val ) ;
	semaphore.sem_flg =1; 
	semop( semid , &semaphore , 1 ) ;
}
void mysignal(int signum){

}
void mysigset(int num){
	struct sigaction mysigaction;
	mysigaction.sa_handler=(void*)mysignal;
	mysigaction.sa_flags=0;
	sigaction(num,&mysigaction,NULL);
}

//Threadlara gönderilen argümanlarýn listesi
struct args {
    int i;
    int j;
    int min;
    int max;
    int *arr;
    int lock;
};
//Asal sayý olup olmadýðýný kontrol eden fonksiyon
bool is_prime(int n){
	int flag=0,i=0;
	for ( i = 2; i <= n / 2; ++i) {

        if (n % i == 0) {
            flag = 1;
            break;
        }
    }

    if (n < 2) {
        return false;
    }
    else {
        if (flag == 0)
            return true;
        else
            return false;
    }
}
//Thread Fonksiyobu
void* ThreadFunc(void*input) {
	int i;
	double result = 0.0;

	int lock=((struct args*)input)->lock;

	printf( "Iplik  %d.%d araniyor: %d-%d ...\n",((struct args*)input)->i,((struct args*)input)->j,((struct args*)input)->min,((struct args*)input)->max) ;
	for(i=((struct args*)input)->min;i<=((struct args*)input)->max;i++){

		if(is_prime(i)){
		    sem_wait(lock,1); 
			globalcp[++globalcp[0]]=i;	//Asal sayý ise global deðiþkene yazýlýyor, bu süreçte diðerleri yazamasýn diye kilitleniyor.
			sleep(1);	
			sem_signal(lock,1);
		}
	}

	pthread_exit ( (void *) input ) ;
}

int main(int argc,char *argv[]){
	mysigset(12);
	if(argc<5)
		return 1;
    //KeySem yaratma
	globalcp = NULL ;
	key_t KEYSHM = ftok( argv[0] , 1 );
	key_t KEYSEM = ftok(argv[0] , 2 );
	//Arg Array ini deðiþkenlere aktarma
	int shmid = 0 ;
	int interval_min=atoi(argv[1]);
	int interval_max=atoi(argv[2]);
	
	int np=atoi(argv[3]);
	int nt=atoi(argv[4]);
	
	int min[np];
	int max[np];
	int init=0;
	int artis_degeri=(interval_max-interval_min+1)/np;
	int i=0;
	int lock=0;
	int f;
	int child[np];
	int localint,index=0;
	//Prosesler için asal sayý bulma aralýðýný belirleme
	for(i=0;i<np;i++){
		min[i]=interval_min+(i*artis_degeri);
		if(i+1==np)
			max[i]=interval_max;
		else
			max[i]=min[i]+(artis_degeri-1);
	}
	//Çocuk prosesler yaratma
	for(i=1;i<=np;i++)  
    {
    	f=fork();
    	if(f==-1)
    		exit(1);
        if(f == 0) 
        	break;
        child[i]=f;
    }
    if(f!=0){
    	printf("Ana Proses: basladi.\n");
    	
    	sem_key=semget(KEYSEM,1,0700|IPC_CREAT);
    	semctl(sem_key,0,SETVAL,0);
    	lock=semget(KEYSEM,1,0700|IPC_CREAT);
    	semctl(lock,0,SETVAL,1);
    	shmid = shmget (KEYSHM, sizeof (int) , 0700 | IPC_CREAT );
		globalcp = (int*) shmat(shmid,0,0);
		
		*globalcp=0; // arrayin indeksini belirleyecek

		sleep(2);
		sem_wait(sem_key,np); // çocuklar bitene kadar bekle.
		sleep(1);
		for(i=0;i<np;i++)
			wait(NULL);
		
		int j;
		for( i=1;i<=globalcp[0];i++){
			for(j=i+1;j<=globalcp[0];j++)
				if(globalcp[j]<globalcp[i]){
					int temp=globalcp[j];
					globalcp[j]=globalcp[i];
					globalcp[i]=temp;
				}
		}
		printf("Ana Proses: sonlandi. Bulunan asal sayilar: ");
		for( i=1;i<=globalcp[0];i++)
			printf("%d, ",globalcp[i]);
		printf("\n");	
		
		shmdt(globalcp);
		semctl(sem_key,0,IPC_RMID,0);
		semctl(shmid,IPC_RMID,0);
		
		exit(0);
		
	}
    else { // Çocuk Proses

    	index=i;
    	printf("Proses %d: basladi. Aralik: %d-%d\n",index,min[index-1],max[index-1]);
    	sem_key=semget(KEYSEM,1,0);
    	shmid = shmget (KEYSHM, sizeof (int) ,0);
    	globalcp = (int*) shmat(shmid,0,0);
    	int min_process[nt];
		int max_process[nt];
		long j;
		int rc;
		void* status;
		int artis_degeri_thread=(max[index-1]-min[index-1]+1)/nt;
		//Ýplik aralýðý belirleme
		for(j=0;j<nt;j++){
			min_process[j]=min[index-1]+(j*artis_degeri_thread);
			if(j+1==nt)
				max_process[j]=max[index-1];
			else
				max_process[j]=min_process[j]+(artis_degeri_thread-1);
			
		}
    	pthread_t thread[nt];
    	pthread_attr_t attr;
        pthread_attr_init (&attr) ;
        pthread_attr_setdetachstate (& attr , PTHREAD_CREATE_JOINABLE);
        // Ýplik yaratma baþlýyor
        for(j=0;j<nt;j++){
        	struct args *Yeni = (struct args *)malloc(sizeof(struct args));
        	Yeni->i=index;
        	Yeni->j=j+1;
        	Yeni->min=min_process[j];
        	Yeni->max=max_process[j];
			Yeni->lock=lock;
        	rc = pthread_create(&thread[j],&attr,ThreadFunc,(void *)Yeni);	
			if(rc){
        		
				free(Yeni);
        		exit(-1);
        	}
        	
        	sleep(1);
		}
		pthread_attr_destroy(&attr) ;
		for(j=0;j<nt;j++){
			rc = pthread_join(thread[j],&status);
			if(rc)
        		exit(-1);
		}
		
		printf("Proses %d: sonlandi.\n",i); //Çocuk proses sonlandý
		shmdt(globalcp);
		sleep(1);
		sem_signal(sem_key,1); // Ýzin ver
		
		
    } 
    
    pthread_exit(NULL);
	return 0;
	
	
}

