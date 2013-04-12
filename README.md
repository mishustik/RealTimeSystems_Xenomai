RealTimeSystems_Xenomai
=======================

RealTimeSystems

This is my hand trying on Xenomai and Real-Time Systems programming.
This is an implementation of the system with four tasks:
  - A task with middle priority periodically executing fibonacci function
  - Two tasks of lowest priority calculating the summation of values from 1 to 1000 and from 1 to 2000 consequently
  - The highest priority task periodically checking the number of times of calculations of the lowest priority tasks

Here are two variants of implementation: 
  - by using 'Sleep' method
  - by using 'rt_task_set_periodic'
