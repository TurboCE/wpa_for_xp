#include <stdio.h>
#include <sys/time.h>
#include "timer.h"



TIME_STRUCT t_start(void)
{
  TIME_STRUCT t;
  gettimeofday(&t.start, NULL);
  return t;
}

void t_end(TIME_STRUCT *t)
{
  gettimeofday(&t->end,NULL);
}

double t_get(TIME_STRUCT ts)
{
  return (ts.end.tv_sec * 1000000 + ts.end.tv_usec)
	  - (ts.start.tv_sec * 1000000 + ts.start.tv_usec);
}

void t_dump(TIME_STRUCT ts)
{
  printf("%ld ns\n", ((ts.end.tv_sec * 1000000 + ts.end.tv_usec)
		   - (ts.start.tv_sec * 1000000 + ts.start.tv_usec)));
}

