#include<stdio.h>
#include<stdlib.h<
#include<dirent.h<
#include<string.h<
#include<sys/types.h<
#include<unistd.h<


int main(int argc, char *argv[])
{
 DIR *d;
 char *t;
 int N=0,pid;
 int cnt=0;
 int curr[10], curr_cnt=0;
 struct dirent *fl;
 d=opendir("/proc");
 int M=getppid();
 printf("Process ID of the terminal : %d\n", M);
 printf("Format: PID  /  PNAME  /  STATE  /  PPID \n");
 while((fl=readdir(d))!=NULL)
 {
  int num=atoi(fl->d_name);
  N=0;
  if(num!=0)
  {
   cnt++;
   t= (char*)malloc(15*sizeof(char));
   strcpy(t,"/proc/");
   strncat(t,fl->d_name,10);
   strncat(t,"/stat",6);  
   FILE *ptr=fopen(t,"r");
   if(ptr==NULL)
   printf("error\n");
   char tt[20];
   char g;
   /**process id**/
   while((g=getc(ptr))!=' ')
   N=N*10 + (g-48);
   pid=N;
   printf("%d  /  ", N);
   g=getc(ptr);
   /**process name**/
   while((g=getc(ptr))!=')' )
   {
    printf("%c", g);
   }
   printf("  /  ");
   g=getc(ptr);
   g=getc(ptr);
   if(g=='S')
    printf("Interruptible sleep  /  ");
   else if(g=='D')
    printf("Uninterruptible sleep  /  ");
   else if(g=='R')
    printf("Runnable  /  ");
   else if(g=='T')
    printf("Stopped  /  ");
   else if(g=='X')
    printf("Dead  /  ");
   else if(g=='Z')
    printf("Zombie  /  ");
   else if(g=='W')
    printf("Paging  /  ");
   g=getc(ptr);
   N=0;
   while((g=getc(ptr))!=' ')
    N = N*10 + (g-48);
   printf("%d" , N);
   if(N==M)
   {
    printf("**\n");
    curr[curr_cnt++]=pid;
   }
   fclose(ptr);
   printf("\n");
  }
 }
 printf("**\nTOTAL PROCESSES:%d\n**\n", cnt);
 printf("**********\n");
 printf("Processes inside the terminal:%d\n",curr_cnt );
 for(int i=0; i<curr_cnt; i++)
  printf("%d ", curr[i]);
 printf("\n");
exit(0);
}