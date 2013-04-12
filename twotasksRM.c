#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>

#include <native/task.h>
#include <native/timer.h>
#include <rtdk.h>

RT_TASK high_task, low_task;

#define MEM_SIZE 1024
char mem[MEM_SIZE+1];

void put_mem(char ch)
{
  static int index = 0;
  
  mem[index] = ch;
  index = (index + 1) % MEM_SIZE;
}

void print_mem(void)
{
  rt_printf("%s\n", mem);
}



long long fibonacci(int n)
{
  long long retValue=1;
  int i;
  
  for(i=1;i<=n;i++)
    retValue *=(long long)i;
  
  return retValue;
}

#define NO_OF_HIGH_ITERATIONS 70
#define NO_OF_LOW_ITERATIONS 3 

void highTask(void *arg){

  RTIME releaseTime, finishTime, delta, total, max, min;
  int char_index = 0;
  int count,i;
	
  rt_task_set_periodic(NULL, TM_NOW, 300000000); //0.3ms
  
  total = 0;
  max = 0;	

  for (count = 0; count < NO_OF_HIGH_ITERATIONS; count++) {
    rt_task_wait_period(NULL);
    releaseTime = rt_timer_read();
    
    for(i=0; i<5; i++){
      put_mem('0'+char_index);
      print_mem();
    }	
    char_index = (char_index+1)%10;
    
    
    finishTime = rt_timer_read();
    delta = finishTime - releaseTime;
    if(delta > max) max = delta;
    total += delta; 
  }	

  rt_printf("highTask: avgExe = %ld.%06ld ms, maxExe = %ld.%06ld ms\n",
	    (long)((total/NO_OF_HIGH_ITERATIONS)/1000000),
	    (long)((total/NO_OF_HIGH_ITERATIONS)%1000000),
	    (long)(max/1000000),
	    (long)(max%1000000));
  print_mem();
}


void lowTask(void *arg){

  RTIME releaseTime, finishTime, delta, total, max;
  int char_index = 0;
  int count,i;

  rt_task_set_periodic(NULL, TM_NOW, 5000000000); //5ms	
	
  total = 0;
  max = 0;
 
  for (count = 0; count < NO_OF_LOW_ITERATIONS; count++) {
    rt_task_wait_period(NULL);
    releaseTime = rt_timer_read();	
    
    for(i=0; i<10; i++){
      put_mem('A'+char_index);
      print_mem();
      fibonacci(3000000);
      print_mem();
    }
    char_index = (char_index+1)%26;
    
    finishTime = rt_timer_read();	
    delta = finishTime - releaseTime;
    if(delta > max) max = delta;
    total += delta;
  }
  
  rt_printf("lowTask: avgExe = %ld.%06ld ms, maxExe = %ld.%06ld ms, minExe = %ld.%06ld ms\n",
	    (long)((total/NO_OF_LOW_ITERATIONS)/1000000),
	    (long)((total/NO_OF_LOW_ITERATIONS)%1000000),
	    (long)(max/1000000),
	    (long)(max%1000000));
}

void Separate_module(void *arg){
  
  RTIME releaseTime, finishTime, delta, total, max;
  int char_index = 0;
  int count,i;

  rt_task_set_periodic(NULL, TM_NOW, 143000000000); //5ms	
	
  total = 0;
  max = 0;
  
  for (count = 0; count < 100; count++) {
    rt_task_wait_period(NULL);
    releaseTime = rt_timer_read();	
    
    for(i=0; i<10; i++){
      put_mem('a'+char_index);
      print_mem();
    }
    char_index = (char_index+1)%26;
    
    finishTime = rt_timer_read();	
    delta = finishTime - releaseTime;
    if(delta > max) max = delta;
    total += delta;
  }
 
  rt_printf("lowTask: avgExe = %ld.%06ld ms, maxExe = %ld.%06ld ms, minExe = %ld.%06ld ms\n",
	    (long)((total/NO_OF_LOW_ITERATIONS)/1000000),
	    (long)((total/NO_OF_LOW_ITERATIONS)%1000000),
	    (long)(max/1000000),
	    (long)(max%1000000));
}

void catch_signal(int sig)
{
	rt_task_delete(&high_task);
	rt_task_delete(&low_task);
	rt_printf("exiting...\n");
	exit(1);
}

int main(void) {
	signal(SIGTERM, catch_signal);
	signal(SIGINT, catch_signal);

	// avoid memory swapping for this program
	mlockall(MCL_CURRENT|MCL_FUTURE);

	/*
	 * This is a real-time compatible printf() package from
	 * Xenomai's RT Development Kit (RTDK), that does NOT cause
	 * any transition to secondary mode.
	 */
	rt_print_auto_init(1);

	rt_task_create(&low_task, "low", 0, XNSCHED_HIGH_PRIO-2, 0);
	rt_task_create(&high_task, "high", 0, XNSCHED_HIGH_PRIO, 0);
	rt_task_create(&Separate_module, "middle", 0, XNSCHED_HIGH_PRIO-1, 0);

	rt_task_start(&low_task, &lowTask, NULL);
	rt_task_start(&high_task, &highTask, NULL);
	rt_task_start(&Separate_module, &lowTask, NULL);

	pause();
	
	return 0;
}

