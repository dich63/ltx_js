
// ----->>>> logger.h --------------

#ifndef LOGGER_PPP_H
#define LOGGER_PPP_H

extern  int process_logger_open();
extern  int process_logger_printf(const char* fmt,...);
/*
#include "ll\logger.h"
#include "windows.h"
int main()
{
double fps,xren;
xren=1.11;
fps=212;
process_logger_open();
//.....
while(1)
{
xren=xren+1;     
process_logger_printf("{fps:%g,xren:%g}",fps,xren);
Sleep(1000);
}
//........
return 0;
}
*/

#endif