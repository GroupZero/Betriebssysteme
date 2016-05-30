#include <stdio.h> 
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
  int ret = 0;
  int time_sleep;
  
  srand ( time(NULL) );
  
  if ( argc == 1 ) {
    time_sleep = rand() % 10 + 1;
    ret = rand() % 10;
    if ( ret > 6 ) ret = 1;
    else ret = 0;
  } else if ( argc == 2 ) {
    time_sleep = atoi(argv[1]);
    ret = rand() % 10;
    if ( ret != 1 ) ret = 0;
  } else {
    time_sleep = atoi(argv[1]);
    ret = atoi(argv[2]);
  } 
 
  //printf("Time %d Exit: %d\n",time_sleep, ret);
  
  sleep(time_sleep * 1);
   
  return ret;
}
