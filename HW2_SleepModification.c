#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>
#include <time.h>
#include <native/task.h>
#include <native/timer.h>

#include <rtdk.h>

const float UtilBound[10] = {1,0.828,0.779,0.756,0.743,0.734,0.728,0.724,0.720,0.690};
const float TasksPeriods[4] = {100000000.0,1000000000.0,1000000000.0,10000000000.0};

int NumbTimes = 0;
RTIME Time_Shift[4] = {0,0,0,0};

float WorstET[4] = {0.0,0.0,0.0,0.0};
float Average[4] = {0.0,0.0,0.0,0.0};

float TotalUtil = 0.0;

long long fibonacci(int n){

  long long retValue=1;
  int i;

  for(i=1;i<=n;i++)
    retValue *=(long long)i;
  
  return retValue;

}

int sum(int n){

  int result = 0;
  int i;
  
  for(i=1;i<=n;i++)
    result = result + i;
  NumbTimes++;
  
  return result;

}

int check_sched_Ubound(){

  int i;

  for(i=0;i<=3;i++)
    TotalUtil += WorstET[i]/TasksPeriods[i];

  if (TotalUtil < UtilBound[3])
    return 1;
  else
    return 0;

}

int check_sched_TDAnalysis(){

  int i;
  float TimeDemand;

  for(i=1;i<=100;i+=1){

    TimeDemand = ((int)(i / (TasksPeriods[0] / 100000000))) * WorstET[0] + 
      ((int)(i / (TasksPeriods[1]  / 100000000))) * WorstET[1] +
      ((int)(i / (TasksPeriods[2]  / 100000000))) * WorstET[2] +
      ((int)(i / (TasksPeriods[3]  / 100000000))) * WorstET[3];
    
    if(i < (float)TimeDemand / 100000000.0)
      return 0;
  }
  
  return 1;

}

pthread_t thread_id, thread_id1, thread_id2, thread_id3;

void heavyThread(void *arg){

  struct sched_param p;
  struct timespec sleep_time,rem;
  RTIME releaseTime, finishTime;
  int counter = 0;
  float Sum1 = 0;
  float ET = 0;

  sleep_time.tv_sec = 0;
  sleep_time.tv_nsec = 100000000/*95000000*/;
  
  p.sched_priority = 5;
  pthread_setschedparam(pthread_self(),SCHED_FIFO,&p);
    
  while(1){

    nanosleep(&sleep_time,&rem);
    rt_printf("task \n");
    releaseTime = rt_timer_read();
    
    fibonacci(1000000);
        
    finishTime = rt_timer_read();
    counter++;
    if (counter == 1)
      Time_Shift[0] = releaseTime;
    ET = finishTime - releaseTime;
    Sum1 += ET;
    Average[0] = Sum1/counter;
    if(ET > WorstET[0])
      WorstET[0] = ET;
    
    rt_printf("Release Time: %ld.%06ld ms\n",((long)(releaseTime - Time_Shift[0])) / 1000000,
	      ((long)(releaseTime - Time_Shift[0])) % 1000000);
    rt_printf("Difference from expected RT: %ld.%06ld ms\n",((long)(releaseTime - Time_Shift[0])) / 1000000 - (long)((counter - 1) * 100),
	      ((long)(releaseTime - Time_Shift[0])) % 1000000);
    rt_printf("Finish Time: %ld.%06ld ms\n",((long)(finishTime - Time_Shift[0])) / 1000000,
	      ((long)(finishTime - Time_Shift[0])) % 1000000);
    rt_printf("Difference from Deadline: %ld.%06ld ms\n",((long)(finishTime - Time_Shift[0])) / 1000000 - (long)(counter * 100) + 1,
	      abs(((long)(finishTime - Time_Shift[0])) % 1000000 - 1000000));
    rt_printf("execution time: %ld.%06ld ms\n",
                       (long)(finishTime - releaseTime) / 1000000,
                       (long)(finishTime - releaseTime) % 1000000);
  }
  
  return 0;
  
}

void thread1(void *arg){

  struct sched_param p;
  struct timespec sleep_time1,rem1;
  RTIME releaseTime, finishTime;
  int counter = 0;
  float Sum1 = 0;
  float ET = 0;

  sleep_time1.tv_sec = 1;
  sleep_time1.tv_nsec = 0;
  
  p.sched_priority = 1;
  pthread_setschedparam(pthread_self(),SCHED_FIFO,&p);
        
  while(1){

    nanosleep(&sleep_time1,&rem1);
    rt_printf("task1 \n");
    releaseTime = rt_timer_read();
    
    sum(1000);
        
    finishTime = rt_timer_read();
    counter++;
    if (counter == 1)
      Time_Shift[1] = releaseTime;
    ET = finishTime - releaseTime;
    Sum1 += ET;
    Average[1] = Sum1/counter;
    if(ET > WorstET[1])
      WorstET[1] = ET;
   
    rt_printf("Release Time: %ld.%06ld ms\n",((long)(releaseTime - Time_Shift[0])) / 1000000,
	      ((long)(releaseTime - Time_Shift[0])) % 1000000);
    rt_printf("Difference from expected RT: %ld.%06ld ms\n",((long)(releaseTime - Time_Shift[1])) / 1000000 - (long)((counter - 1) * 1000),
	      ((long)(releaseTime - Time_Shift[1])) % 1000000);
    rt_printf("Finish Time: %ld.%06ld ms\n",((long)(finishTime - Time_Shift[0])) / 1000000,
	      ((long)(finishTime - Time_Shift[0])) % 1000000);
    rt_printf("Difference from Deadline: %ld.%06ld ms\n",((long)(finishTime - Time_Shift[1])) / 1000000 - (long)(counter * 1000) + 1,
	      abs(((long)(finishTime - Time_Shift[1])) % 1000000 - 1000000));
    rt_printf("execution time: %ld.%06ld ms\n",
                       (long)(finishTime - releaseTime) / 1000000,
                       (long)(finishTime - releaseTime) % 1000000);
  }

}

void thread2(void *arg){

  struct sched_param p;
  struct timespec sleep_time2,rem2;
  RTIME releaseTime, finishTime;
  int counter = 0;
  float Sum1 = 0;
  float ET = 0;

  sleep_time2.tv_sec = 1;
  sleep_time2.tv_nsec = 0;
  
  p.sched_priority = 1;
  pthread_setschedparam(pthread_self(),SCHED_FIFO,&p);
        
  while(1){

    nanosleep(&sleep_time2,&rem2);
    rt_printf("task2 \n");
    releaseTime = rt_timer_read();
    
    sum(2000);
        
    finishTime = rt_timer_read();
    counter++;
    if (counter == 1)
      Time_Shift[2] = releaseTime;
    ET = finishTime - releaseTime;
    Sum1 += ET;
    Average[2] = Sum1/counter;
    if(ET > WorstET[2])
      WorstET[2] = ET;
    
    rt_printf("Release Time: %ld.%06ld ms\n",((long)(releaseTime - Time_Shift[0])) / 1000000,
	      ((long)(releaseTime - Time_Shift[0])) % 1000000);
    rt_printf("Difference from expected RT: %ld.%06ld ms\n",((long)(releaseTime - Time_Shift[2])) / 1000000 - (long)((counter - 1) * 1000),
	      ((long)(releaseTime - Time_Shift[2])) % 1000000);
    rt_printf("Finish Time: %ld.%06ld ms\n",((long)(finishTime - Time_Shift[0])) / 1000000,
	      ((long)(finishTime - Time_Shift[0])) % 1000000);
    rt_printf("Difference from Deadline: %ld.%06ld ms\n",((long)(finishTime - Time_Shift[2])) / 1000000 - (long)(counter * 1000) + 1,
	      abs(((long)(finishTime - Time_Shift[2])) % 1000000 - 1000000));
    rt_printf("execution time: %ld.%06ld ms\n",
                       (long)(finishTime - releaseTime) / 1000000,
                       (long)(finishTime - releaseTime) % 1000000);
  }

}

void thread3(void *arg){

  struct sched_param p;
  struct timespec sleep_time3,rem3;
  RTIME releaseTime, finishTime;
  int counter = 0, NumbTimesResult = 0;
  float Sum1 = 0;
  float ET = 0;

  sleep_time3.tv_sec = 10;
  sleep_time3.tv_nsec = 0;
  
  p.sched_priority = 10;
  pthread_setschedparam(pthread_self(),SCHED_FIFO,&p);
      
  while(1){

    nanosleep(&sleep_time3,&rem3);
    rt_printf("task3 \n");
    releaseTime = rt_timer_read();
    
    NumbTimesResult = NumbTimes;
    
    finishTime = rt_timer_read();
    counter++;
    if (counter == 1)
      Time_Shift[3] = releaseTime;
    ET = finishTime - releaseTime;
    Sum1 += ET;
    Average[3] = Sum1/counter;
    if(ET > WorstET[3])
      WorstET[3] = ET;
    
    rt_printf("Release Time: %ld.%06ld ms\n",((long)(releaseTime - Time_Shift[0])) / 1000000,
	      ((long)(releaseTime - Time_Shift[0])) % 1000000);
    rt_printf("Difference from expected RT: %ld.%06ld ms\n",((long)(releaseTime - Time_Shift[3])) / 1000000 - (long)((counter - 1) * 10000),
	      ((long)(releaseTime - Time_Shift[3])) % 1000000);
    rt_printf("Finish Time: %ld.%06ld ms\n",((long)(finishTime - Time_Shift[0])) / 1000000,
	      ((long)(finishTime - Time_Shift[0])) % 1000000);
    rt_printf("Difference from Deadline: %ld.%06ld ms\n",((long)(finishTime - Time_Shift[3])) / 1000000 - (long)(counter * 10000) + 1,
	      abs(((long)(finishTime - Time_Shift[3])) % 1000000 - 1000000));
    rt_printf("execution time: %ld.%06ld ms\n",
                       (long)(finishTime - releaseTime) / 1000000,
                       (long)(finishTime - releaseTime) % 1000000);
    
  }

}

void catch_signal(int sig){

  pthread_cancel(thread_id);
  pthread_join(thread_id,NULL);
  pthread_cancel(thread_id1);
  pthread_join(thread_id1,NULL);
  pthread_cancel(thread_id2);
  pthread_join(thread_id2,NULL);
  pthread_cancel(thread_id3);
  pthread_join(thread_id3,NULL);
  

  int sched1 = check_sched_Ubound();
  int sched2 = check_sched_TDAnalysis();
  rt_printf("Utilization: %f \n",TotalUtil);
  rt_printf("Schedulability_Ubound: %d \n",sched1);
  rt_printf("Schedulability_TimeDemand: %d \n",sched2);
  
  rt_printf("average time: %ld.%06ld ms\n",
                       (long)(Average[0]) / 1000000,
                       (long)(Average[0]) % 1000000);
  rt_printf("average time1: %ld.%06ld ms\n",
                       (long)(Average[1]) / 1000000,
                       (long)(Average[1]) % 1000000);
  rt_printf("average time2: %ld.%06ld ms\n",
                       (long)(Average[2]) / 1000000,
                       (long)(Average[2]) % 1000000);
  rt_printf("average time3: %ld.%06ld ms\n",
                       (long)(Average[3]) / 1000000,
                       (long)(Average[3]) % 1000000);
  rt_printf("worst time: %ld.%06ld ms\n",
                       (long)(WorstET[0]) / 1000000,
                       (long)(WorstET[0]) % 1000000);
  rt_printf("worst time1: %ld.%06ld ms\n",
                       (long)(WorstET[1]) / 1000000,
                       (long)(WorstET[1]) % 1000000);
  rt_printf("worst time2: %ld.%06ld ms\n",
                       (long)(WorstET[2]) / 1000000,
                       (long)(WorstET[2]) % 1000000);
  rt_printf("worst time3: %ld.%06ld ms\n",
                       (long)(WorstET[3]) / 1000000,
                       (long)(WorstET[3]) % 1000000);
  
  
  rt_printf("exiting...\n");
  exit(1);

}


int main(){

  signal(SIGTERM, catch_signal);
  signal(SIGINT, catch_signal);

  int arg[2];

  rt_print_auto_init(1);

  mlockall(MCL_CURRENT|MCL_FUTURE);

  
  pthread_create(&thread_id,NULL,heavyThread,NULL);
  pthread_create(&thread_id1,NULL,thread1,NULL);
  pthread_create(&thread_id2,NULL,thread2,NULL);
  pthread_create(&thread_id3,NULL,thread3,NULL);
    
  pthread_join(thread_id,NULL);
  pthread_join(thread_id1,NULL);
  pthread_join(thread_id2,NULL);
  pthread_join(thread_id3,NULL);
    
  return 0;

}
