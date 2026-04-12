#include <lpc21xx.h>
#include "lcd_defines.h"

// Generate a delay in microseconds based on the processor clock speed
void delay_us(int dlyus)
{
    // Approximation for microsecond delay (adjust multiplier based on CCLK)
    for(dlyus *= 12; dlyus > 0; dlyus--);
}

// Generate a delay in milliseconds
void delay_ms(int dlyms)
{
    // Approximation for millisecond delay
    for(dlyms *= 12000; dlyms > 0; dlyms--);
}

// Low-level function to perform the Write operation to the LCD
void WriteLCD(unsigned char cmd)
{
    IOCLR0 = 1 << RW; // Set RW to 0 for Write operation
    // Clear data pins and shift current command/data into the designated PIN range
    IOPIN0 = (IOPIN0 & ~(0XFF << PIN)) | (cmd << PIN);
    IOSET0 = 1 << EN; // Pulse Enable HIGH to start latching
    delay_us(1);      // Hold for minimum setup time
    IOCLR0 = 1 << EN; // Set Enable LOW to finish latching
    delay_ms(2);      // Wait for LCD's internal processing to complete
}

// Send an instruction command (like Clear Screen or Move Cursor)
void CmdLCD(unsigned char cmd)
{
    IOCLR0 = 1 << RS; // Set RS to 0 to select the Command Register
    WriteLCD(cmd);
}

// Send a single ASCII character to be displayed
void CharLCD(unsigned char ascii)
{
    IOSET0 = 1 << RS; // Set RS to 1 to select the Data Register
    WriteLCD(ascii);
}

// Comprehensive LCD Hardware Initialization
void InitLCD(void)
{
    // Configure Data pins, RS, RW, and EN as Output pins in IODIR0
    IODIR0 |= ((0XFF << PIN) | (1 << RS) | (1 << RW) | (1 << EN));
    delay_ms(15);         // Initial power-on delay for LCD controller
    CmdLCD(M_8BIT_1L);    // Mandatory sequence to wake up the controller
    delay_ms(5);
    CmdLCD(M_8BIT_1L);
    delay_us(100);
    CmdLCD(M_8BIT_1L);
    CmdLCD(M_8BIT_2L);    // Configure for 2 lines, 5x7 matrix (Standard 16x2)
    CmdLCD(D_ON);         // Turn display ON
    CmdLCD(CLR);          // Clear initial garbage from screen
    CmdLCD(SHIFT_C_R);    // Set cursor to auto-increment to the right
}

// Display a full string (character array) until the null terminator is reached
void StrLCD(signed char *ptr)
{
    while(*ptr)
    {
        CharLCD(*ptr++); // Send character and move pointer to next address
    }
}

// Convert and display integer values on the LCD
void intLCD(int num)
{
    char a[10];
    int i = 0;
    if(num == 0)
        CharLCD('0');
    if(num < 0)
    {
        CharLCD('-'); // Handle negative signs
        num = -(num);
    }
    while(num)
    {
        a[i++] = (num % 10) + '0'; // Extract digits and convert to ASCII
        num /= 10;
    }
    // Print digits in correct order (reverse from the extraction order)
    for(i -= 1; i >= 0; i--)
        CharLCD(a[i]);
}

// Convert and display floating point numbers with a set number of decimal places (DP)
void FloatLCD(float dec, unsigned char DP)
{
    int n, i;
    if(dec < 0)
    {
        CharLCD('-');
        dec = -(dec);
    }
    n = dec; // Extract integer part
    intLCD(n);
    CharLCD('.'); // Print decimal point
    for(i = 0; i < DP; i++)
    {
        dec = (dec - n) * 10; // Extract fractional digits one by one
        n = dec;
        CharLCD(n + 48);      // Print converted ASCII digit
    }
}
