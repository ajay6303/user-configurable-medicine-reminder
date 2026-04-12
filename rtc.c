#include "rtc.h"
#include <lpc214x.h>

typedef unsigned int u32;
typedef int s32;

#define _LPC2148

/*
Initialize the Real-Time Clock (RTC)
Prepares the RTC hardware by resetting and then enabling the clock source.
*/
void RTC_Init(void)
{
    // Clock Control Register (CCR): Reset the clock to clear any previous state
    CCR = RTC_RESET;

    #ifndef _LPC2148
    // If not using LPC2148, configure the internal prescaler for timing accuracy
    PREINT = PREINT_VAL;
    PREFRAC = PREFRAC_VAL;

    // Start the RTC
    CCR = RTC_ENABLE;
    #else
    // For LPC2148: Enable the RTC and specify an external 32.768kHz crystal as the source
    CCR = RTC_ENABLE | RTC_CLKSRC;
    #endif
}

/*
Get the current RTC time as a formatted string
Converts internal integer registers into ASCII characters for LCD display.
Format: HH:MM:SS
*/
void GetRTCTimeInfo(char *rtc)
{
    // Extract Hours and convert to ASCII
    rtc[0] = (HOUR / 10) + '0';
    rtc[1] = (HOUR % 10) + '0';
    // Positions rtc[2] and rtc[5] are usually ':' (assumes pre-formatted buffer)
    // Extract Minutes and convert to ASCII
    rtc[3] = (MIN / 10) + '0';
    rtc[4] = (MIN % 10) + '0';
    // Extract Seconds and convert to ASCII
    rtc[6] = (SEC / 10) + '0';
    rtc[7] = (SEC % 10) + '0';
}

/*
Get the current RTC date
Extracts Day of Month, Month, and Year into a string buffer for the LCD.
*/
void GetRTCDateInfo(char *p)
{
    int i, y;
    // Extract Day of Month (DOM)
    p[0] = (DOM / 10) + '0';
    p[1] = (DOM % 10) + '0';
    // Extract Month
    p[3] = (MONTH / 10) + '0';
    p[4] = (MONTH % 10) + '0';

    // Loop to extract the four digits of the Year into the end of the string
    y = YEAR;
    for(i = 9; y; y /= 10)
    {
        p[i] = y % 10 + '0';
        i--;
    }
}

/*
Set the RTC time from a string
Used during "Edit RTC Time" mode to update the hardware registers from keypad input.
*/
void SetRTCTimeInfo(char *p)
{
    int h = 0, m = 0, s = 0;

    // Convert string characters back to integers for the hardware registers
    h = (p[0] - '0') * 10 + (p[1] - '0');
    m = (p[3] - '0') * 10 + (p[4] - '0');
    s = (p[6] - '0') * 10 + (p[7] - '0');

    // Update the RTC hardware registers
    HOUR = h;
    MIN = m;
    SEC = s;
}

/*
Set the RTC date from a string
Updates Day, Month, and Year based on user input.
*/
void SetRTCDateInfo(char* p)
{
    int d = 0, m = 0, y = 0;

    d = (p[0] - '0') * 10 + (p[1] - '0');
    m = (p[3] - '0') * 10 + (p[4] - '0');
    
    // Accumulate digits to form the 4-digit year (e.g., 2026)
    y = (p[6] - '0') + (y * 10);
    y = (p[7] - '0') + (y * 10);
    y = (p[8] - '0') + (y * 10);
    y = (p[9] - '0') + (y * 10);

    DOM = d;
    MONTH = m;
    YEAR = y;
}

/*
Read the Day of the Week
Returns values 0-6 (0 = Sunday, 1 = Monday, etc.)
*/
void GetRTCDay(s32 *dow)
{
    *dow = DOW;
}

/*
Write the Day of the Week
Allows the user to manually set which day it is during setup.
*/
void SetRTCDay(u32 dow)
{
    DOW = dow;
}
