#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#define KEYSHM ftok("SHMKEY", 3)

//Semaforlar� mainde tan�mlad�m
//Signal fonksiyonlar�
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

void sig_usr(int signo){
	if(signo == SIGINT)
		printf("Signal caught!");
	return;
}
//Fibonacci fonksiyonu
int fibonacci(int n)
{
   if (0 == n) {
        return 0;
    } else if (1 == n) {
        return 1;
    } else {
        return fibonacci(n - 2) + fibonacci(n - 1);
    }
}
int main(int argc,char *argv[]){
	if(argc<6)
		return 1;
	//Semaforlar i�in key tan�m�	
	key_t SEMKEY = ftok(argv[0] , 2 );
	key_t SEMKEY1 = ftok(argv[0] , 3 );
	key_t SEMKEY2 = ftok(argv[0] , 4 );
	key_t SEMKEY3 = ftok(argv[0] , 5 );
	key_t SEMKEY4 = ftok(argv[0] , 6 );
	key_t SEMKEY5 = ftok(argv[0] , 7 );
	key_t SEMKEY6 = ftok(argv[0] , 7 );
	//Terminali okuma
	int para_miktari=atoi(argv[1]);
	int ni=atoi(argv[2]);
	int nd=atoi(argv[3]);
	int ti=atoi(argv[4]);
	int td=atoi(argv[5]);
	
	int shmid = 0,i,f ;
	int sem_artirici,sem_azaltici,sem_art_tek,sem_azal_tek,sem_tur_art,sem_main,sem_tur_azal; //Semaforlar
	int toplam=nd+ni;

	pid_t children[toplam]; //toplam proses say�s�
	int artirici=0,azaltici=0;
    //Payla��lan bellek
	int *globalcp = NULL; 
	shmid = shmget (KEYSHM, sizeof (int) , 0700 | IPC_CREAT );
    globalcp = (int*) shmat(shmid,0,0);
    
    *globalcp=0; // �u anki para miktar�
    
    globalcp[1]=ti/2; //azalt�c� i�in bir turdaki olmas� gereken tek say�y� tutuyor
    globalcp[2]=ti/2; // azalt�c� i�in bir turdaki olmas� gereken �ift say�y� tutuyor
    globalcp[3]=1;//azaltici proses i�in tek say�lar�n ka��nc� fibonacci de oldu�unu tutuyor
    globalcp[4]=1;//azaltici proses i�in �ift say�lar�n ka��nc� fibonacci de oldu�unu tutuyor
    globalcp[5]=0;//art�r�c� prosesin �al��ma say�s�n� tutuyor
    globalcp[6]=0;//art�r�c� prosesin tur say�s�n� tutuyor
    globalcp[7]=0;//azalt�c� prosesin tur say�s�n� tutuyor
    globalcp[8]=0;//azalt�c� prosesin �al��ma say�s�n� tutuyor
    globalcp[9]=1;//art�r�c� proses i�in toplam tur say�s�
    globalcp[10]=1;//azalt�c� proses i�in toplam tur say�s�
    
    mysigset(12);
    fflush(NULL);
    printf("Ana Proses: Mevcut para %d\n",*globalcp);
    //ni ve nd nin toplam� kadar proses olu�tur
    for(i=1;i<=toplam;i++)  
	{
			f=fork();
			if(f==-1)
				exit(1);
		    if(f == 0) 
		   		break;
			else
				children[i-1]=f;
			
	}
	if(f!=0)//anne
	{
    // Semaforlar� olu�tur
	  sem_artirici=semget(SEMKEY,1,0700|IPC_CREAT);
	  sem_azaltici=semget(SEMKEY1,1,0700|IPC_CREAT);
	  sem_art_tek=semget(SEMKEY2,1,0700|IPC_CREAT);
	  sem_azal_tek=semget(SEMKEY3,1,0700|IPC_CREAT);
	  sem_main=semget(SEMKEY4,1,0700|IPC_CREAT);
	  sem_tur_art=semget(SEMKEY5,1,0700|IPC_CREAT);
	  sem_tur_azal=semget(SEMKEY6,1,0700|IPC_CREAT);
	// �lk de�erlerini ata  
	  semctl(sem_artirici,0,SETVAL,ni);
	  semctl(sem_azaltici,0,SETVAL,0);
	  semctl(sem_art_tek,0,SETVAL,1);
	  semctl(sem_azal_tek,0,SETVAL,0);
	  semctl(sem_tur_art,0,SETVAL,0);
	  semctl(sem_tur_azal,0,SETVAL,0);
	  semctl(sem_main,0,SETVAL,0);
	  
	  sleep(1);
	  // �ocuk proseslere sinyal g�nder
	  for(i=0;i<toplam;i++)
		kill(children[i],12);
		
	  sem_wait(sem_main,1); // program� bitirmek i�in sinyal bekle
	  //b�t�n �ocuk prosesleri �ld�r
	  for(i=0;i<toplam;i++)
		if(kill(children[i],SIGTERM)==-1 && errno!=ESRCH){
				exit(EXIT_FAILURE);
		}
	  
	  printf("Ana proses: Killing all children and terminating the program\n");
	  //Semaforlar� kald�r
	  semctl(sem_artirici, 0, IPC_RMID, 0);
      semctl(sem_azaltici, 0, IPC_RMID, 0);
      semctl(sem_art_tek, 0, IPC_RMID, 0);
      semctl(sem_azal_tek, 0, IPC_RMID, 0);
      semctl(sem_tur_art, 0, IPC_RMID, 0);
      semctl(sem_tur_azal, 0, IPC_RMID, 0);
      semctl(sem_main, 0, IPC_RMID, 0);
      exit(0);

	}
	else
	{
		
		pause();
		//semaforlar� belirle
		sem_artirici = semget(SEMKEY, 1, 0);
        sem_azaltici = semget(SEMKEY1, 1, 0);
        sem_art_tek = semget(SEMKEY2, 1, 0);
        sem_azal_tek = semget(SEMKEY3, 1, 0);
        sem_main = semget(SEMKEY4, 1, 0);
        sem_tur_art = semget(SEMKEY5, 1, 0);
        sem_tur_azal = semget(SEMKEY6, 1, 0);
        
      while(1) // main e sinyal g�nderilene kadar devam et
       {
		    
			if(i <= ni){ //E�er art�r�c� proses ise
				sem_wait(sem_artirici,1); // ni tanesini girebilir
				while(globalcp[6]<ti){ // globalcp[6]=tur say�s�
					sem_wait(sem_art_tek,1); // ni tanesinden bir tanesi ba�lar
					if(i<=ni/2) //proseslerin ilk yar�s�ndaysa 10 art�r�r
						*globalcp+=10;
					else //proseslerin di�er yar�s�ndaysa 15 art�r�r
						*globalcp+=15;
					
					globalcp[5]++;// bu turdaki toplam proses say�s�
					if(globalcp[5]==ni){ //e�er bu turdaki toplam proses say�s�==art�r�c� proses say�s�
						printf("Artirici proses %d: Mevcut para %d, artirici prosesin bitirdigi tur %d\n\n",i-1,*globalcp,globalcp[9]++);
						globalcp[5]=0;//bu turdaki toplam proses say�s�n� s�f�rla
						globalcp[6]++; //tur say�s�n� artt�r
						if(globalcp[6]==ti){ //e�er tur say�s� == toplam tur say�s�
							globalcp[6]=0; // tur say�s�n� s�f�rla 
							if(*globalcp<para_miktari){ //e�er anl�k para miktar� < belirlenen para miktar�
								sem_signal(sem_art_tek, 1); //di�er tur i�in aktif et
								sem_signal(sem_tur_art, ni-1);//bu turdaki prosesleri serbest b�rak
								continue;
							}
							sem_signal(sem_azaltici, nd); // azalt�c� prosesi ba�lat
							sem_signal(sem_azal_tek, 1); // azalt�c� proseste tek bir prosesin �al��mas�na izin ver 
							sem_signal(sem_tur_art, ni-1); //bu turdaki prosesleri serbest b�rak
							break;
						}
						sem_signal(sem_art_tek, 1);//bu turdaki di�erlerinin �al��mas�na izin ver
						sem_signal(sem_tur_art, ni-1);//bu turdaki prosesleri serbest b�rak
					}
					else{ 
						printf("Artirici proses %d: Mevcut para %d\n",i-1,*globalcp);
						sem_signal(sem_art_tek, 1);//bu turdaki di�erlerinin �al��mas�na izin ver
						sem_wait(sem_tur_art,1);//bu tur bitene kadar bekle
					}
				}
				sem_signal(sem_artirici, 1);
				
			}
			else{
				/*Ayn� i�lemler azalt�c� prosesler  */
				sem_wait(sem_azaltici,1);
				while(globalcp[7]<td){
					sem_wait(sem_azal_tek,1);
					int fib,sayi,azaltici;
					globalcp[8]++; // bu turdaki toplam proses say�s�
					if(*globalcp%2==0&&globalcp[2]>0){ //	E�er para miktar� �ift say� ise ve �ift say� prosesi varsa
						fib=fibonacci(globalcp[4]);
						sayi=globalcp[4];
						globalcp[4]++; //�ift say� i�in fibonacci tutucusunu artt�r
						globalcp[2]--; //�ift say� proses say�s�n� azalt
						*globalcp-=fib;
						
						if(*globalcp%2==0)
							azaltici=1;
						else
							azaltici=0;
						if(*globalcp>0)
							printf("Azaltici Proses %d: Mevcut para %d (%d. fibonacci sayisi azaltici %d )",i-ni-1,*globalcp,sayi,azaltici);
						if(globalcp[8]==nd &&*globalcp>0)
							printf(", azaltici proseslerin bitirdigi tur %d \n",globalcp[10]++);
						else if(*globalcp%2==0&&globalcp[2]==0 &&*globalcp>0)
							printf(", azaltici proseslerin bitirdigi tur %d",globalcp[10]++);
						
					}
					else if(*globalcp%2!=0&&globalcp[1]>0){ //	E�er para miktar� tek say� ise ve tek say� prosesi varsa
						fib=fibonacci(globalcp[3]);
						sayi=globalcp[3];
						azaltici=*globalcp%2;
						globalcp[3]++; //tek say� i�in fibonacci tutucusunu artt�r
						globalcp[1]--; //tek say� proses say�s�n� azalt
						*globalcp-=fib;
						if(*globalcp%2==0)
							azaltici=1;
						else
							azaltici=0;
						if(*globalcp>0)
							printf("Azaltici Proses %d: Mevcut para %d (%d. fibonacci sayisi azaltici %d )",i-ni-1,*globalcp,sayi,azaltici);
						if(globalcp[8]==nd &&*globalcp>0)
							printf(", azaltici proseslerin bitirdigi tur %d \n",globalcp[10]++);
						else if(*globalcp%2!=0&&globalcp[1]==0 &&*globalcp>0)
							printf(", azaltici proseslerin bitirdigi tur %d",globalcp[10]++);
						
					}
					if(*globalcp<=0){ //Para miktar� s�f�rdan k���k veya e�itse
						printf("Azaltici Proses %d: Mevcut para %d' ten kucuk, ana prosese bitirmesi i�in bir sinyal yollaniyor (%d. fibonacci sayisi azaltici %d )\n",i-ni-1,fib,sayi,azaltici);
						sem_signal(sem_main, 1);// ana prosese sinyal g�nder
						break;
					}
					if(globalcp[8]==nd){ // bu turdaki toplam proses say�s� == toplam tur say�s� 
						globalcp[1]=ti/2; //tek proses say�s�n� g�nceller
						globalcp[2]=ti/2; //�ift proses say�s�n� g�nceller
						globalcp[8]=0; // bu turdaki toplam proses say�s�n� s�f�rla
						globalcp[7]++; // tur say�s�n� artt�r 
						if(globalcp[7]==td){ // Tur say�s� == toplam tur say�s�
							printf("\n");
							globalcp[7]=0;  //tur say�s�n� g�ncelle
							sem_signal(sem_artirici, ni); // art�r�c� prosese sinyal g�nder
							sem_signal(sem_art_tek, 1); 
							sem_signal(sem_tur_azal, nd-1); // bu turdaki prosesleri serbest b�rak
							break;
						}
						sem_signal(sem_azal_tek, 1);//bu turdaki di�erlerinin �al��mas�na izin ver
						sem_signal(sem_tur_azal, nd-1);// bu turdaki prosesleri serbest b�rak
					}
					else{
						printf("\n");
						sem_signal(sem_azal_tek, 1);//bu turdaki di�erlerinin �al��mas�na izin ver
						sem_wait(sem_tur_azal,1); // bu turdaki prosesleri serbest b�rak
					}
				}
				sem_signal(sem_azaltici,nd);
				
			}
	   }
	}
	
	return 0;
}
