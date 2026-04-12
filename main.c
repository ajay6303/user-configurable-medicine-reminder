/* * Project: User-Configurable Medicine Reminder System
 * File: main.c
 * Target: LPC2148 (ARM7)
 * Description: Main loop handling RTC display, medicine time comparison, 
 * and buzzer alert management.
 */

#include <lpc21xx.h>
#include "LCD.h"
#include "KPM.h"
#include "lcd_defines.h"
#include "kpm_defines.h"
#include "rtc.h"
#include "interrupt.h"

// Global Flags and Variables
int flag=0, flag1=0, flag3=0, flag4=0, s=0;
char key, m1[]="00:00:00", m2[]="00:00:00", m3[]="00:00:00", time[]="00:00:00", date[]="00/00/0000";
extern char dayLUT[][4];
int day;

// Alert Management Variables
int alert_timer = 0;           // Countdown for how long the buzzer stays on (seconds)
int last_second = 0;           // Tracks RTC second changes to decrement alert_timer
int last_triggered_minute = -1; // Prevents the alarm from re-triggering multiple times in the same minute

int main()
{
    // Peripheral Initialization
    IODIR0 |= 1 << BUZZER;      // Configure Buzzer pin as Output
    InitKPM();                  // Initialize 4x4 Keypad
    InitLCD();                  // Initialize 16x2 LCD
    RTC_Init();                 // Start Internal Real-Time Clock
    Init_intrrupt();            // Configure EINT0 (Menu) and EINT1 (Ack)
    
    CmdLCD(D_ON);               // Display ON, Cursor OFF

    while(1)
    {
        // Fetch current time from RTC registers
        GetRTCTimeInfo(time);

        /* --- CONFIGURATION MENU LOGIC --- */
        // Triggered by EINT0 ISR setting 'flag'
        if(flag)
        {
            flag1 = 1;          // Indicate system is in "Edit Mode"
            CmdLCD(D_ON_C_BLK); // Show blinking cursor for user input
            configure();        // Enter menu system (interrupt.c)
            flag1 = 0;          // Exit Edit Mode
            CmdLCD(CLR);
            CmdLCD(D_ON);       // Restore normal display
        }

        /* --- SECOND-BASED TIMER DECREMENT --- */
        // Calculates current second to create a non-blocking 1-second delay pulse
        s = (time[6] - '0') * 10;
        s += (time[7] - '0');

        if (s != last_second)   // Executes once every time the RTC second increments
        {
            last_second = s;
            if (alert_timer > 0)
            {
                alert_timer--; 
                if(alert_timer == 0)
                    last_triggered_minute = -1; // Reset trigger so alarm can fire later
                
                flag4 = 1;      // Request LCD refresh
            }
        }

        /* --- MEDICINE TIME COMPARISON LOGIC --- */
        if (flag3) // flag3 is set once the user has configured at least one reminder
        {
            // Only check if we haven't already fired an alarm this minute
            if (MIN != last_triggered_minute) 
            {
                // Compare current RTC time against the 3 stored medicine slots
                if ((check(m1, time)) || (check(m2, time)) || (check(m3, time)))
                {
                    if(alert_timer == 0)
                    {
                        alert_timer = 60;          // Set alert duration for 60 seconds
                        last_triggered_minute = MIN; // Lock trigger for this minute
                    }
                }
            }
        }

        /* --- ALARM EXECUTION vs. CLOCK DISPLAY --- */
        if (last_triggered_minute == MIN)
        {
            // State: ACTIVE ALERT
            CmdLCD(GOTO_L1_POSN0);
            StrLCD((signed char*)"TAKE MEDICINE!  ");
            CmdLCD(GOTO_L2_POSN0);
            StrLCD((signed char*)"Rem: ");
            intLCD(alert_timer); // Show remaining seconds
            StrLCD((signed char*)" sec left    ");

            IOSET0 = 1 << BUZZER; // Activate audible alert
        }
        else
        {
            // State: IDLE / CLOCK DISPLAY
            IOCLR0 = 1 << BUZZER; // Ensure buzzer is off

            // Clear the "TAKE MEDICINE" text once when returning to idle
            if (flag4) {
                CmdLCD(CLR);
                flag4 = 0;
            }

            // Display Digital Clock (Line 1)
            GetRTCTimeInfo(time);
            CmdLCD(GOTO_L1_POSN0);
            StrLCD((signed char*)time);

            // Display Day of Week
            GetRTCDay(&day);
            CmdLCD(GOTO_L1_POSN0 + 10);
            StrLCD((signed char*)dayLUT[day]);

            // Display Date (Line 2)
            GetRTCDateInfo(date);
            CmdLCD(GOTO_L2_POSN0);
            StrLCD((signed char*)date);
        }
    }
}
