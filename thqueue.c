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
