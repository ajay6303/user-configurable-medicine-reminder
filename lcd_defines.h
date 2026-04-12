// Hardware Pin Connections for the LCD (LPC2148 Port 0)
#define PIN 8    // Data Bus: D0-D7 connected to P0.8 through P0.15
#define RS 18    // Register Select (P0.18): 0 = Command, 1 = Data
#define RW 17    // Read/Write (P0.17): 0 = Write, 1 = Read
#define EN 19    // Enable (P0.19): Pulsed to latch data/commands into the LCD

// Standard LCD Command Set
#define CLR 0x01           // Clear the entire display screen
#define RET 0x02           // Move cursor back to the first position of the first line
#define D_OFF 0x08         // Turn off display, but keep content in memory
#define D_ON 0x0c          // Turn on display without a visible cursor
#define D_ON_C_ON 0x0e     // Turn on display with a static underline cursor
#define D_ON_C_BLK 0x0f    // Turn on display with a blinking block cursor

// Function Set / Operating Modes
#define M_8BIT_1L 0x30     // Configure for 8-bit data mode, 1-line display
#define M_8BIT_2L 0x38     // Configure for 8-bit data mode, 2-line display (Standard for 16x2)

// Cursor Positioning (Memory addresses)
#define GOTO_L1_POSN0 0x80 // Set cursor to the start (column 0) of Line 1
#define GOTO_L2_POSN0 0xc0 // Set cursor to the start (column 0) of Line 2

// Entry Mode and Shifting Commands
#define SHIFT_C_R 0x06     // Entry Mode: Move cursor right after each character is written
#define SHIFT_D_L 0X10     // Command: Move cursor one position to the left
#define SHIFT_D_R 0x14     // Command: Move cursor one position to the right
