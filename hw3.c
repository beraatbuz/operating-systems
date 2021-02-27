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

//Semaforlarý mainde tanýmladým
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
	//Semaforlar için key tanýmý	
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

	pid_t children[toplam]; //toplam proses sayýsý
	int artirici=0,azaltici=0;
    //Paylaþýlan bellek
	int *globalcp = NULL; 
	shmid = shmget (KEYSHM, sizeof (int) , 0700 | IPC_CREAT );
    globalcp = (int*) shmat(shmid,0,0);
    
    *globalcp=0; // þu anki para miktarý
    
    globalcp[1]=ti/2; //azaltýcý için bir turdaki olmasý gereken tek sayýyý tutuyor
    globalcp[2]=ti/2; // azaltýcý için bir turdaki olmasý gereken çift sayýyý tutuyor
    globalcp[3]=1;//azaltici proses için tek sayýlarýn kaçýncý fibonacci de olduðunu tutuyor
    globalcp[4]=1;//azaltici proses için çift sayýlarýn kaçýncý fibonacci de olduðunu tutuyor
    globalcp[5]=0;//artýrýcý prosesin çalýþma sayýsýný tutuyor
    globalcp[6]=0;//artýrýcý prosesin tur sayýsýný tutuyor
    globalcp[7]=0;//azaltýcý prosesin tur sayýsýný tutuyor
    globalcp[8]=0;//azaltýcý prosesin çalýþma sayýsýný tutuyor
    globalcp[9]=1;//artýrýcý proses için toplam tur sayýsý
    globalcp[10]=1;//azaltýcý proses için toplam tur sayýsý
    
    mysigset(12);
    fflush(NULL);
    printf("Ana Proses: Mevcut para %d\n",*globalcp);
    //ni ve nd nin toplamý kadar proses oluþtur
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
    // Semaforlarý oluþtur
	  sem_artirici=semget(SEMKEY,1,0700|IPC_CREAT);
	  sem_azaltici=semget(SEMKEY1,1,0700|IPC_CREAT);
	  sem_art_tek=semget(SEMKEY2,1,0700|IPC_CREAT);
	  sem_azal_tek=semget(SEMKEY3,1,0700|IPC_CREAT);
	  sem_main=semget(SEMKEY4,1,0700|IPC_CREAT);
	  sem_tur_art=semget(SEMKEY5,1,0700|IPC_CREAT);
	  sem_tur_azal=semget(SEMKEY6,1,0700|IPC_CREAT);
	// Ýlk deðerlerini ata  
	  semctl(sem_artirici,0,SETVAL,ni);
	  semctl(sem_azaltici,0,SETVAL,0);
	  semctl(sem_art_tek,0,SETVAL,1);
	  semctl(sem_azal_tek,0,SETVAL,0);
	  semctl(sem_tur_art,0,SETVAL,0);
	  semctl(sem_tur_azal,0,SETVAL,0);
	  semctl(sem_main,0,SETVAL,0);
	  
	  sleep(1);
	  // çocuk proseslere sinyal gönder
	  for(i=0;i<toplam;i++)
		kill(children[i],12);
		
	  sem_wait(sem_main,1); // programý bitirmek için sinyal bekle
	  //bütün çocuk prosesleri öldür
	  for(i=0;i<toplam;i++)
		if(kill(children[i],SIGTERM)==-1 && errno!=ESRCH){
				exit(EXIT_FAILURE);
		}
	  
	  printf("Ana proses: Killing all children and terminating the program\n");
	  //Semaforlarý kaldýr
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
		//semaforlarý belirle
		sem_artirici = semget(SEMKEY, 1, 0);
        sem_azaltici = semget(SEMKEY1, 1, 0);
        sem_art_tek = semget(SEMKEY2, 1, 0);
        sem_azal_tek = semget(SEMKEY3, 1, 0);
        sem_main = semget(SEMKEY4, 1, 0);
        sem_tur_art = semget(SEMKEY5, 1, 0);
        sem_tur_azal = semget(SEMKEY6, 1, 0);
        
      while(1) // main e sinyal gönderilene kadar devam et
       {
		    
			if(i <= ni){ //Eðer artýrýcý proses ise
				sem_wait(sem_artirici,1); // ni tanesini girebilir
				while(globalcp[6]<ti){ // globalcp[6]=tur sayýsý
					sem_wait(sem_art_tek,1); // ni tanesinden bir tanesi baþlar
					if(i<=ni/2) //proseslerin ilk yarýsýndaysa 10 artýrýr
						*globalcp+=10;
					else //proseslerin diðer yarýsýndaysa 15 artýrýr
						*globalcp+=15;
					
					globalcp[5]++;// bu turdaki toplam proses sayýsý
					if(globalcp[5]==ni){ //eðer bu turdaki toplam proses sayýsý==artýrýcý proses sayýsý
						printf("Artirici proses %d: Mevcut para %d, artirici prosesin bitirdigi tur %d\n\n",i-1,*globalcp,globalcp[9]++);
						globalcp[5]=0;//bu turdaki toplam proses sayýsýný sýfýrla
						globalcp[6]++; //tur sayýsýný arttýr
						if(globalcp[6]==ti){ //eðer tur sayýsý == toplam tur sayýsý
							globalcp[6]=0; // tur sayýsýný sýfýrla 
							if(*globalcp<para_miktari){ //eðer anlýk para miktarý < belirlenen para miktarý
								sem_signal(sem_art_tek, 1); //diðer tur için aktif et
								sem_signal(sem_tur_art, ni-1);//bu turdaki prosesleri serbest býrak
								continue;
							}
							sem_signal(sem_azaltici, nd); // azaltýcý prosesi baþlat
							sem_signal(sem_azal_tek, 1); // azaltýcý proseste tek bir prosesin çalýþmasýna izin ver 
							sem_signal(sem_tur_art, ni-1); //bu turdaki prosesleri serbest býrak
							break;
						}
						sem_signal(sem_art_tek, 1);//bu turdaki diðerlerinin çalýþmasýna izin ver
						sem_signal(sem_tur_art, ni-1);//bu turdaki prosesleri serbest býrak
					}
					else{ 
						printf("Artirici proses %d: Mevcut para %d\n",i-1,*globalcp);
						sem_signal(sem_art_tek, 1);//bu turdaki diðerlerinin çalýþmasýna izin ver
						sem_wait(sem_tur_art,1);//bu tur bitene kadar bekle
					}
				}
				sem_signal(sem_artirici, 1);
				
			}
			else{
				/*Ayný iþlemler azaltýcý prosesler  */
				sem_wait(sem_azaltici,1);
				while(globalcp[7]<td){
					sem_wait(sem_azal_tek,1);
					int fib,sayi,azaltici;
					globalcp[8]++; // bu turdaki toplam proses sayýsý
					if(*globalcp%2==0&&globalcp[2]>0){ //	Eðer para miktarý çift sayý ise ve çift sayý prosesi varsa
						fib=fibonacci(globalcp[4]);
						sayi=globalcp[4];
						globalcp[4]++; //çift sayý için fibonacci tutucusunu arttýr
						globalcp[2]--; //çift sayý proses sayýsýný azalt
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
					else if(*globalcp%2!=0&&globalcp[1]>0){ //	Eðer para miktarý tek sayý ise ve tek sayý prosesi varsa
						fib=fibonacci(globalcp[3]);
						sayi=globalcp[3];
						azaltici=*globalcp%2;
						globalcp[3]++; //tek sayý için fibonacci tutucusunu arttýr
						globalcp[1]--; //tek sayý proses sayýsýný azalt
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
					if(*globalcp<=0){ //Para miktarý sýfýrdan küçük veya eþitse
						printf("Azaltici Proses %d: Mevcut para %d' ten kucuk, ana prosese bitirmesi için bir sinyal yollaniyor (%d. fibonacci sayisi azaltici %d )\n",i-ni-1,fib,sayi,azaltici);
						sem_signal(sem_main, 1);// ana prosese sinyal gönder
						break;
					}
					if(globalcp[8]==nd){ // bu turdaki toplam proses sayýsý == toplam tur sayýsý 
						globalcp[1]=ti/2; //tek proses sayýsýný günceller
						globalcp[2]=ti/2; //çift proses sayýsýný günceller
						globalcp[8]=0; // bu turdaki toplam proses sayýsýný sýfýrla
						globalcp[7]++; // tur sayýsýný arttýr 
						if(globalcp[7]==td){ // Tur sayýsý == toplam tur sayýsý
							printf("\n");
							globalcp[7]=0;  //tur sayýsýný güncelle
							sem_signal(sem_artirici, ni); // artýrýcý prosese sinyal gönder
							sem_signal(sem_art_tek, 1); 
							sem_signal(sem_tur_azal, nd-1); // bu turdaki prosesleri serbest býrak
							break;
						}
						sem_signal(sem_azal_tek, 1);//bu turdaki diðerlerinin çalýþmasýna izin ver
						sem_signal(sem_tur_azal, nd-1);// bu turdaki prosesleri serbest býrak
					}
					else{
						printf("\n");
						sem_signal(sem_azal_tek, 1);//bu turdaki diðerlerinin çalýþmasýna izin ver
						sem_wait(sem_tur_azal,1); // bu turdaki prosesleri serbest býrak
					}
				}
				sem_signal(sem_azaltici,nd);
				
			}
	   }
	}
	
	return 0;
}
