/* "getkey.c": unbuffered console input
Copyright January 3 1992
Philips Research Labs Eindhoven
M.E. Groenewegen assisted by P. van Hooft

Refurbished MEG august 2011


          --- Notes for usage: ---

Export functions:

The function getkey() waits for a keypress and returns the ASCII value
  of the key that was pressed.

The function keypressed() returns TRUE if a key has been pressed prior
  to the calling of the function
*/


#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <termio.h>


struct termio old_termio;
fd_set readfds; // create an FD set


int init_keypress()
{
struct termio new_termio;

  /* get information about terminal connected to stdin */
  if( -1 == ioctl(0, TCGETA, &old_termio) ){
    std::cout << "init_keypress" << std::endl;
    exit(1);
  }
  new_termio = old_termio;

  /*
  If the ICANON flag is not set, read requests are satisfied directly from
  the input queue. In this case, a read request is not satisfied until one of
  the following conditions is met: The minimum number of characters specified
  by MIN are received. The time-out value specified by TIME has expired
  since the last character was received. This allows bursts of input to be
  read, while still allowing single-character input.
  */
  new_termio.c_lflag &= ~ICANON;
  new_termio.c_cc[VMIN] = 1;
  new_termio.c_cc[VTIME] = 0;
  /* reconfigure terminal connected to stdin */
  if ( -1 == ioctl(0, TCSETA, &new_termio) ){
    std::cout << "init_keypress" << std::endl;
    exit(1);
  }

  return 0;
} /* init_keypress */


void end_keypress()
{
  if( -1 == ioctl(0, TCSETA, &old_termio)) /* restore old settings */
    std::cout << "end_keypress" << std::endl;
} /* end_keypress */



int keypressed()
{
struct timeval timeout;
int retval;

  timeout.tv_sec=0;
  timeout.tv_usec=0;

  FD_ZERO(&readfds); // clear the set
  FD_SET(0,&readfds); // add stdin (fd 0) to set

  retval = select(1,&readfds,NULL,NULL,&timeout);
  if(retval == -1) {
    std::cout << "keypressed" << std::endl;
  }
  else if(retval) return 1;
  return 0;

  // make sure stdin is actually in the set
  //if(FD_ISSET(0,&readfds)) return 1;
  //return 0;
} /* keypressed */



/*
 * Test routine

int main()
{
char nextchar='@';

  init_keypress();

  while(nextchar != 'q')
  {
    printf(".\n");
    if(keypressed())
    {
      nextchar = getchar(); // clear input buffer
      printf("KEY '%c'\n",nextchar);
    }
    usleep(100000);
  }

  return 0;
}
 */

