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
const int TasksPeriods[4] = {100000000.0,1000000000.0,1000000000.0,10000000000.0};

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

RT_TASK task, task1, task2, task3;

void heavyThread(void *arg){

  struct sched_param p;
  RTIME releaseTime, finishTime;
  int counter = 0;
  float Sum1 = 0;
  float ET = 0;
    
  rt_task_set_periodic(NULL,TM_NOW,100000000);
      
  while(1){

    rt_task_wait_period(NULL);
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
}

void thread1(void *arg){

  struct sched_param p;
  RTIME releaseTime, finishTime;
  int counter = 0;
  float Sum1 = 0;
  float ET = 0;

  rt_task_set_periodic(NULL,TM_NOW,1000000000);
        
  while(1){

    rt_task_wait_period(NULL);
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
  RTIME releaseTime, finishTime;
  int counter = 0;
  float Sum1 = 0;
  float ET = 0;

  rt_task_set_periodic(NULL,TM_NOW,1000000000);
      
  while(1){

    rt_task_wait_period(NULL);
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
  RTIME releaseTime, finishTime;
  int counter = 0, NumbTimesResult = 0;
  float Sum1 = 0;
  float ET = 0;

  rt_task_set_periodic(NULL,TM_NOW,10000000000);
      
  while(1){

    rt_task_wait_period(NULL);
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
    
    rt_printf("Number of calculations: %d  \n",NumbTimes);
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

  rt_task_delete(&task);
  rt_task_delete(&task1);
  rt_task_delete(&task2);
  rt_task_delete(&task3);
  
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

  rt_print_auto_init(1);

  mlockall(MCL_CURRENT|MCL_FUTURE);

  rt_task_create(&task,"heavy",0,XNSCHED_HIGH_PRIO-1,0);
  rt_task_create(&task1,"low_prior1",0,XNSCHED_HIGH_PRIO-20,0);
  rt_task_create(&task2,"low_prior2",0,XNSCHED_HIGH_PRIO-18,0);
  rt_task_create(&task3,"top_prior",0,XNSCHED_HIGH_PRIO,0);
  rt_task_start(&task,&heavyThread,0);
  rt_task_start(&task1,&thread1,0);
  rt_task_start(&task2,&thread2,0);
  rt_task_start(&task3,&thread3,0);
  
  pause();
  
  //pthread_create(&thread_id2,NULL,thread,(void*)arg);
  //arg[0] = 1;
  //arg[1] = 800000000;
  //pthread_create(&thread_id3,NULL,thread,(void*)arg);
  return 0;

}
