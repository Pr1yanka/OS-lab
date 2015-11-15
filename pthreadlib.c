#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/time.h>
#include<string.h>
#include<errno.h>
#include<ucontext.h>
#define MAX_THREADS 10
#define STACK_SIZE 64000
#define TIME_SLOT 500000   //time slot given to each thread in round robin scheduling (0.25sec here..)
typedef struct Task* Tptr;


//======================================================
//======================================================

struct  Task
{
 int user_priority;
 ucontext_t threads_context;
 struct Task *next;
 int thread_id;
};
struct Tque
{
 struct Task *head;
 struct Task *tail;
 int count;
 
};

struct Task_Semaphore
{
 struct Tque wait_queue;
 int count;
};

void display(struct Tque *u)
{
 struct Task *tmp=u->head;
 while(tmp!=NULL)
 {
  printf("%p ", tmp);
  tmp=tmp->next;
 }
 printf("\n");
}
void insert(struct Tque *q, struct Task *t)
{
 if(q->count==0)
 {
  

  t->next=NULL;
  q->head=q->tail=t;
  
 }
 else
 {
  t->next=NULL;
  (q->tail)->next=t;
  q->tail=(q->tail)->next;
 }
 q->count++;
//c printf("insert:");
// display(q);
}

void remov(struct Tque *q, int id)
{
 struct Task *tmp=q->head, *prev;
 while(tmp!=NULL && tmp->thread_id!=id)
 {
  prev=tmp;
  tmp=tmp->next;
 }
 if(tmp!=NULL)
 {
//c  printf("getting deleted\n");
  if(tmp==q->head)
  {
//c   printf("first one!\n");
   q->head=(q->head)->next;
   //free(tmp);
  }
  else
  {
   prev->next=(prev->next)->next;
   //free(tmp);
  }
  q->count--;
 }
 else
 {
  printf("node with id=%d not found!\n",id);
 }
//c printf("delete:");
// display(q);
}

int isempty(struct Tque *q)
{
//c printf("count:%d\n",q->count );
 return q->count==0;
}

struct Task* top(struct Tque* q)
{
 if(q!=NULL)
 {
//c  printf("correct!\n");
  return q->head;
 }
 else
 {
  return NULL;
 }
}

struct Task* next(struct Tque* q, struct Task* cur)
{
 return cur->next;
}

struct Task* rem_and_poi(struct Tque* q, struct Task* to)
{
 struct Task *tmp=q->head, *tmp1=NULL;
 while(tmp!=to)
 {
  tmp1=tmp;
  tmp=tmp->next;
 }
 if(tmp1==NULL) //node is at head...
 {
  q->head=tmp->next;
  return tmp;
 }
 else   //node is in middle or may be end...
 {
  tmp1->next=tmp->next;
  return tmp;
 }
}


















//=======================================================
//=======================================================

int CURRENT_THREAD_COUNT=0;
int TERMINATED=0;
int Global_id;


 ucontext_t Main;  //context for saving main...

 ucontext_t Delete[MAX_THREADS];  //context for thread recycling...
 int alloc[MAX_THREADS];
struct itimerval alarm;

struct sigaction sched_signal;


struct Tque    ready_queue;
struct Task     *Current_Thread=NULL;

void schedule()
{
//cs printf("activated\n");
 if(Current_Thread==NULL) // Main Thread was executing previously..
 {
  TERMINATED=0;
  if(!isempty(&ready_queue))       // If there are more threads...
  {
   Current_Thread=top(&ready_queue);
//c   printf("yes!! %p\n", Current_Thread);
   Global_id=Current_Thread->thread_id;

   swapcontext(&Main, &(Current_Thread->threads_context));
  }
  else                // If all threads are dead...
  {
//c   printf("main is getting set!\n");
   setcontext(&Main);
  }
 }
 else     //if someother thread was executing...
 {
  struct Task* tmp=next(&ready_queue, Current_Thread);
//c  printf("other way around and %p and current:%p\n", tmp, Current_Thread);
  if(tmp==NULL)  //if this was the last thread in the queue....
  {     //execute main fn.
//c   printf("it was null\n");
   if(TERMINATED==1)
   {
//c    printf("its gone!!\n");
    TERMINATED=0;
    remov(&ready_queue, Global_id);
    Current_Thread=NULL;
    setcontext(&Main);
   }
   else
   {
    struct Task* tmp1=Current_Thread;
    Current_Thread=NULL;
    swapcontext(&(tmp1->threads_context), &Main); 
   }
   
  }
  else
  {
   struct Task* tmp2=Current_Thread;
   Current_Thread=tmp;
   if(TERMINATED==1)
   {
    TERMINATED=0;
    remov(&ready_queue, Global_id);
    Global_id=tmp->thread_id;
//c    printf("context set for %p\n", tmp);

    setcontext(&(tmp->threads_context));
   }
   else
   {
    Global_id=tmp->thread_id;
//c    printf("running:%p\n", tmp);
    swapcontext(&(tmp2->threads_context), &(tmp->threads_context)); 
   }
   
  }
 }
}

void Kill_Task(void *arg)
{
//c printf("its coming!!\n");
 int t_id=*((int*)arg);
//c printf("terminator for %d started\n", Global_id);
 TERMINATED=1;
 schedule();
}


void Init_Task()
{
 sched_signal.sa_handler=schedule;
 sigemptyset(&sched_signal.sa_mask);
 sched_signal.sa_flags=0;
 sigaction(SIGPROF, &sched_signal, NULL);
 ready_queue.count=0;
 int i=-1;
 while(++i<MAX_THREADS)
  alloc[i]=0;
 //for the first time alarm after 0 sec....
 //for every TIME_SLOT secs...
 alarm.it_value.tv_sec=0;
 alarm.it_value.tv_usec=TIME_SLOT;
 //next time alarm after every TIME_SLOT secs...
 alarm.it_interval.tv_sec=0;
 alarm.it_interval.tv_usec=TIME_SLOT;
//c printf("Initialized\n");
}

void Init_Semaphore(struct Task_Semaphore* mut, int value)
{
 mut->count=value;
 
}

void change_Semaphore(struct Task_Semaphore* mut, int value)
{
 sigset_t old, new;
 sigemptyset(&new);
 sigaddset(&new, SIGPROF);
 sigprocmask(SIG_BLOCK, &new, &old);
 if(mut->count > value)
 {
  mut->count-=value;
  sigprocmask(SIG_SETMASK, &old, NULL);
  return;
 }
 else
 {
  insert(&(mut->wait_queue), rem_and_poi(&ready_queue, Current_Thread));
  sigprocmask(SIG_SETMASK, &old,NULL);
  schedule();
 }
}
void create_task(struct Task* target, void (*fn)(void*), void * param)
{


  if(getcontext(&(target->threads_context))==-1)
  {
   printf("getcontext error!\n");
   exit(1);
  }
  
  (target->threads_context).uc_stack.ss_sp=malloc(STACK_SIZE);
  (target->threads_context).uc_stack.ss_size=STACK_SIZE;
  (target->threads_context).uc_stack.ss_flags=0;

  

  int id=-1;
  while(++id<10 && alloc[id]!=0);

  if(id==10)
  {
   printf("allocation error\n");
   exit(1);
  }
  target->thread_id=id;
  alloc[id]=1;
  //Delete[id]=malloc(sizeof(ucontext_t));
//c  printf("yyyy\n");
  if(getcontext(&Delete[id])==-1)
  {
   printf("delete context error!\n");
   exit(1);
  }
  
  Delete[id].uc_link=0;
  Delete[id].uc_stack.ss_sp=malloc(STACK_SIZE);
  Delete[id].uc_stack.ss_size=STACK_SIZE;
  Delete[id].uc_stack.ss_flags=0;
  (target->threads_context).uc_link=&Delete[id];
  makecontext(&Delete[id], (void*)&Kill_Task, 0);
  makecontext(&(target->threads_context), fn, 0);

  target->next=NULL;
  //if this is the first thread, then set the timer...
  if(CURRENT_THREAD_COUNT++==0)
  {
    if(setitimer(ITIMER_PROF, &alarm, NULL)==-1)
    {
     //some shit is happening with setitimer
     printf("Itimer error:%s", strerror(errno));
     exit(1);
    }
//c    printf("timer set!\n");
  }
  //thread should be added to queue...
  //so block all the SIGPROF signal until this is added....
  sigset_t new,old;
  sigemptyset(&new);
  sigaddset(&new, SIGPROF);
  sigprocmask(SIG_BLOCK, &new, &old);
  if(isempty(&ready_queue)!=0)
  {
//c   printf("first added to queue %p\n", target);
   insert(&ready_queue, target);
  }
  else
  {
//c   printf("next added to queue %p\n", target);
   insert(&ready_queue, target);
  }
  //sleep(5);
  //restore back the old mask...
  sigprocmask(SIG_SETMASK, &old, NULL);
}  


int Sigh=0;
void sample()
{
 printf("success!!\n");
 while(Sigh==0);
 printf("completed!!\n");
 //setcontext(&Main);
}

void samps()
{
 
 printf("success2\n");
 Sigh=1;
}
void sam3()
{
 printf("started3\n");
 //while(1);
 printf("success3\n");
}

int main(int argc, char *argv[])
{
 Init_Task();
 struct Task t,p,q;
 int a=5;
 create_task(&t, (void*)&sample, (void*)&a);
 create_task(&p, (void*)&samps, (void*)&a);
 create_task(&q, (void*)&sam3, (void*)&a);
 //getcontext(&Main);
 printf("yeasss\n");
 while(1);
 //exit(0);
}


