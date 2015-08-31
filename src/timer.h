
#include <sys/time.h>


typedef struct _time_struc {
  struct timeval start, end;
} TIME_STRUCT;

//typedef struct _time_sturc TIME_STRUCT;

TIME_STRUCT t_start(void);
void t_end(TIME_STRUCT *t);
double t_get(TIME_STRUCT ts);
void t_dump(TIME_STRUCT ts);
