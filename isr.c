/*
 * File:   isr.c
 * Author: DELL
 *
 * Created on 30 September, 2023, 4:14 PM
 */


#include <xc.h>
#include "main.h"

void __interrupt() isr(void)
{
    extern unsigned char sec, min;
    static unsigned int count = 0;
    
    if (TMR2IF == 1)
    {    
        if (++count == 1250) // 1sec
        {
            count = 0;
            if(sec>0)
            {
                sec--;
            }
            else if((sec == 0) && (min > 0))
            {
                min--;
                sec = 59;
            }
           
        }
        
        TMR2IF = 0;
    }
}