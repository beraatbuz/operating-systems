#include <stdio.h> // printf
#include<unistd.h> // fork
#include<stdlib.h> // exit
#include<sys/wait.h> // wait
int main()
{
	int proses,proses2,proses3,proses4;
	int level=0;
	printf("\nparent_pid %d  at level %d has 3 children:\n", getpid(),++level );
	for ( int i = 0; i < 3; i++ ){
		proses=fork();
        if ( proses == 0 )
    	{
    		
    		printf("	child_pid = %d\n",getpid());
            ++level;
            proses2=fork();
            if(proses2>0){
            	wait(NULL);
            	proses3=fork();
            	if(proses3==0){//sol çocuk
            		printf("\nparent_pid = %d at level %d has  NO CHILD\n", getpid(),++level) ;	
					exit( 0 );	
				}
				else{
				wait( NULL );// önce child bitsin
				printf("\nparent_pid = %d at level %d has 2 children:\n", getpid(),level);
				printf("	child_pid=%d\n",proses2);
            	printf("	child_pid=%d\n",proses3);
				exit( 0 );
				}
				
            	exit( 0 );
			}
			else{ // sað çocuk
				level++;
				proses4=fork();
				if(proses4==0){
					printf("\nparent_pid = %d at level %d has  NO CHILD\n", getpid(),++level) ;
					exit( 0 );
				}
				else{
					wait( NULL ); //önce child bitsin
					printf("\nparent_pid = %d at level %d has 1 children:\n", getpid(),level);
					printf("	child_pid=%d\n",proses4);
            		exit( 0 );
				}
            	exit( 0 );
			}
		}
    }

   
   for ( int i = 0; i < 3; i++ ) // prosesler bitene kadar bekle
        wait( NULL );
	return 0;
}
