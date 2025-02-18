/*
 * File:   main.c
 * Author: DELL
 *
 * Created on 22 September, 2023, 9:02 AM
 */

#include "main.h"

#pragma config WDTE = OFF // watchdog timer disabled

unsigned char sec=0, min=0, flag=0 ;
int operation_flag = POWER_ON_SCREEN;



static void init_config(void) 
{
    //initialsing lcd
    init_clcd();
    
    //initialising the keypad
    init_matrix_keypad();
    
    //fan
    FAN = OFF;
    FAN_DDR = 0;
    
    BUZZER = OFF;
    BUZZER_DDR = 0;
    
    //initialize the timer 2
    init_timer2();
    GIE = ON;
    PEIE = ON;
    
    
}

void main(void) 
{
    init_config();
    // variable
    unsigned char key;
    int reset_flag;
    while (1) 
    {
        unsigned char key =read_matrix_keypad(STATE);
        // Check the status of the door

        if (DOOR == OPEN)
        {

            // The door is open, take appropriate action

            door_status();

        }
        if(operation_flag == MENU_DISPLAY_SCREEN)
        {
              if(key==1)
        {
          operation_flag = MICRO_MODE; 
          reset_flag = MODE_RESET;
          clear_screen();
          clcd_print(" Power = 900W   ",LINE2(0));
          __delay_ms(3000);
          clear_screen();
         
        }
        else if (key==2)//grill
        {
            operation_flag = GRILL_MODE;
            clear_screen();
            reset_flag = MODE_RESET;
        }
        else if(key==3)//convection
        {
           operation_flag = CONVECTION_MODE;
            clear_screen();
            reset_flag = MODE_RESET; 
        }
        else if(key==4)//start
        {
            clear_screen();
            sec = 30;
            min = 0;
            FAN = ON;
            TMR2ON = ON;
     
            operation_flag = TIME_DISPLAY;
        }
        }
         //SW1 is pressed
        else if(operation_flag == TIME_DISPLAY)
        {
            if(key == 4)//start
            {
                sec= sec + 30;
                if(sec > 59)
                {
                    min++;
                    sec = sec - 60;
                }
            }
            else if(key == 5)
            {
                operation_flag = PAUSE;
            }
            else if(key == 6)
            {
              operation_flag= STOP;  
              clear_screen();
            }
        }
        else if(operation_flag == PAUSE)
        {
            if(key == 4)//resume
            {
                FAN = ON;
                TMR2ON = ON;
                operation_flag = TIME_DISPLAY;
            }
        }
    
        switch(operation_flag)
        {
            case POWER_ON_SCREEN:
                power_on_screen();
                operation_flag=MENU_DISPLAY_SCREEN;
                clear_screen();
                break;
            case MENU_DISPLAY_SCREEN:
                menu_display_screen();
                break;
            case GRILL_MODE:
                set_time(key, reset_flag);
                break;
            case MICRO_MODE:
                set_time(key, reset_flag);
                break;
            case CONVECTION_MODE:
                if(flag == 0)
                {
                set_temp(key,reset_flag);
                if(flag == 1)
                  {
                    clear_screen();
                    reset_flag = MODE_RESET;
                    continue;
                  }
                }
                else if(flag == 1)
                {
                    set_time(key,reset_flag);
                }
                 
                break;
            case TIME_DISPLAY:
                time_display_screen();
                break;
            case PAUSE:
                FAN = OFF;
                TMR2ON = OFF;
                break;
            case STOP:
                FAN = OFF;
                TMR2ON = OFF;
                operation_flag= MENU_DISPLAY_SCREEN;
                break;
            
        }
        
        reset_flag = RESET_NOTHING;
    }
}

void power_on_screen(void)
{
    unsigned char i;
    //LINE 1
    for (i=0;i<16;i++)
    {
        //clcd_write(BAR,DATA_MODE);
        clcd_putch(BAR,LINE1(i));
        __delay_us(100);
    }
    
    clcd_print("  Powering On  ",LINE2(0));
    clcd_print(" Microwave Oven ",LINE3(0));
    
    
    for (i=0;i<16;i++)
    {
        //clcd_write(BAR,DATA_MODE);
        clcd_putch(BAR,LINE4(i));
        __delay_us(100);
        
    }
    __delay_ms(3000);
}

void menu_display_screen(void)
{
    clcd_print("1.Micro", LINE1(0));
    clcd_print("2.Grill", LINE2(0));
    clcd_print("3.Convection", LINE3(0));
    clcd_print("4.Start", LINE4(0));
    
}



void clear_screen(void)
{
    clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
    __delay_us(500);
}

void set_temp(unsigned char key, int reset_flag)
{
   static unsigned char key_count, blink, temp;
    static int wait; 
    if(reset_flag == MODE_RESET)
    {
        key_count=0;
        wait=0;
        blink=0;
        temp = 0;
        key = ALL_RELEASED;
        flag = 0;
    clcd_print(" SET TEMP(oC)",LINE1(0));
    clcd_print(" TEMP =",LINE2(0));
    clcd_print("*:CLEAR  #:ENTER",LINE4(0));
        
    }
    if((key != '*') && (key != '#') && (key != ALL_RELEASED))
    {
        key_count++;
                if(key_count <= 3)
                {
                    temp = temp*10 + key;
                }   
               
    }            
    else if(key == '*')
    {
          temp = 0; 
          key_count=0;
    }
      
    else if(key == '#')
    {
        clear_screen();
        clcd_print("  Pre-Heating ", LINE1(0));
        clcd_print(" Time Rem = ",LINE3(0));
        TMR2ON = ON;
        sec = 180;
        while(sec != 0)
        {
        clcd_putch((sec/100) + '0',LINE3(11));
        clcd_putch((sec/10)%10 + '0',LINE3(12));
        clcd_putch((sec%10) + '0',LINE3(13)); 
        }
        //secs is zero
        if( sec == 0)
        {
            //set time is similar to grill mode
            //operation_flag = GRILL_MODE;
            TMR2ON = OFF;
            reset_flag = MODE_RESET;
            flag = 1;
            clear_screen();
            set_time(key,reset_flag);
           
        }
    }
    if(wait++ == 15)
    {
        wait = 0;
        blink = !blink;
    //printing temperature
        clcd_putch((temp/100) + '0',LINE2(8));
        clcd_putch((temp/10)%10 + '0',LINE2(9));
        clcd_putch((temp%10) + '0',LINE2(10));
    }
    if(blink)
    {
       clcd_print("   ",LINE2(8));
    }
           
 }

void set_time(unsigned char key, int reset_flag)
{
    static unsigned char key_count, blink_pos, blink;
    static int wait;
  if(reset_flag == MODE_RESET)
  {
      key_count=0;
      sec = 0;
      min = 0;
      blink_pos=0;
      key = ALL_RELEASED;
      wait = 0;
      blink = 0;
    clcd_print("SET TIME (MM:SS)",LINE1(0));
    clcd_print("TIME-",LINE2(0));
    clcd_print("*:CLEAR  #:ENTER",LINE4(0));
  }
    if((key != '*') && (key != '#') && (key != ALL_RELEASED))
    {
        key_count++;
                if(key_count <= 2)
                {
                    sec = sec*10 + key;
                    blink_pos=0;
                }
                else if((key_count > 2) && (key_count <=4))
                {
                    min = min*10 + key;
                    blink_pos=1;
                }
                      
    }
    else if(key == '*')
    {
      if(blink_pos == 0)
      {
          sec = 0; 
          key_count=0;
      }
      else if(blink_pos == 1)
      {
          min = 0;
          key_count=2;
      }
    }
    else if(key == '#')
    {
        clear_screen();
        operation_flag = TIME_DISPLAY; 
        FAN = ON; 
        
        TMR2ON = ON;
    }
    
    if(wait++ == 15)
    {
        wait = 0;
        blink = !blink;
    //printing secs and mins
    //mins
    clcd_putch(min/10 + '0',LINE2(6));
    clcd_putch(min%10 + '0',LINE2(7));
    clcd_putch(':',LINE2(8));
    //secs
    clcd_putch(sec/10 + '0',LINE2(9));
    clcd_putch(sec%10 + '0',LINE2(10));
    }
    if(blink)
    {
        switch(blink_pos)
        {
            case 0:
                clcd_print("  ",LINE2(9));
                break;
            case 1:
                clcd_print("  ",LINE2(6));
                break;
        }
        
        
    }
    
    
}

void door_status(void)
{
    if(DOOR == OPEN)
    {
        TMR2ON = OFF;
        FAN = OFF;
        BUZZER = ON;
        clear_screen();
        clcd_print("DOOR is Open",LINE2(0));
        clcd_print("Please Close",LINE3(0));
        //Wait till Door Closed
        while(DOOR == OPEN) //RB3 =1
        {
            ;
        }
        BUZZER = OFF;
        TMR2ON = OFF;
        FAN = OFF;
        clear_screen();
    }
}

void time_display_screen(void)
{
    door_status();
    /*Line1 display*/
    clcd_print("TIME = ",LINE1(0));
    //print min and sec
    //min
    clcd_putch(min/10 + '0', LINE1(9));
    clcd_putch(min%10 + '0', LINE1(10));
    clcd_putch(':', LINE1(11));
    //sec
    clcd_putch(sec/10 + '0', LINE1(12));
    clcd_putch(sec%10 + '0', LINE1(13));
    //print options
    clcd_print(" 4.Start/Resume",LINE2(0));
    clcd_print(" 5.Pause",LINE3(0));
    clcd_print(" 6.Stop",LINE4(0));   

    if(sec == 0 && min == 0)

    {
        clear_screen();
        clcd_print(" Time Up! ", LINE2(0));
        BUZZER = ON;
        __delay_ms(3000); // 3 sec
        clear_screen();
        BUZZER = OFF;
        FAN = OFF;
        TMR2ON = OFF;
        operation_flag = MENU_DISPLAY_SCREEN;
    }
}
        
     
    
    
    
   
