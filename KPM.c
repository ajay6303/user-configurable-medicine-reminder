#include <lpc21xx.h>
#include "kpm_defines.h"
#include "KPM.h"

// 2D Array mapping row and column intersections to specific characters
unsigned char kpmLUT[4][4] = {
    {'1','2','3','/'},
    {'4','5','6','*'},
    {'7','8','9','-'},
    {'C','0','=','+'}
};

// Initialize Keypad Matrix: Set Row pins as Output
void InitKPM(void)
{
    // Configure 4 bits starting from ROW0 as output in IODIR1
    IODIR1 |= 15 << ROW0; 
}

// Check if any key in the matrix is currently pressed
int ColScan(void)
{
    // Shift column pins to LSB, mask 4 bits, and check if value is less than 15 (0xF)
    // A '0' bit indicates a connection (key press)
    return (((IOPIN1 >> COL0) & 15) < 15) ? 0 : 1;
}

// Identify which specific Row contains the pressed key
int RowCheck(void)
{
    int rno;
    for(rno = 0; rno < 4; rno++)
    {
        // Ground one row at a time (set to 0) while keeping others High
        IOPIN1 = (~(1 << rno)) << ROW0;
        
        // If ColScan detects a '0', the press is on this active row
        if(ColScan() == 0)
        {
            break;
        }
    }
    // Reset all rows to logic 0 (default state for next scan)
    IOPIN1 = 0x0 << ROW0;
    return rno;
}

// Identify which specific Column was pulled Low by the key press
int ColCheck(void)
{
    int cno;
    for(cno = 0; cno < 4; cno++)
    {
        // Shift to current column bit and check if it is pulled to GND (logic 0)
        if((IOPIN1 >> (COL0 + cno) & 1) == 0)
        {
            break;
        }
    }
    return cno;
}

// Main function to detect a full key press and release cycle
char KeyScan(void)
{
    int rno, cno, keyv;
    
    // Step 1: Block execution until a key is physically pressed (ColScan returns 0)
    while(ColScan());
    
    // Step 2: Determine Row and Column indices
    rno = RowCheck();
    cno = ColCheck();
    
    // Step 3: Map indices to character using the Look-Up Table
    keyv = kpmLUT[rno][cno];
    
    // Step 4: Block execution until the key is released (prevents multiple triggers)
    while(!ColScan());
    
    return keyv;
}
