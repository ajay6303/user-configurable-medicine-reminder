// Include standard LCD driver headers for displaying time/date
#include "lcd.h"
#include "lcd_defines.h"

// System clock (FOSC = Crystal Frequency) and Internal Clock Macros
#define FOSC 12000000      // Typical crystal frequency is 12MHz
#define CCLK (5*FOSC)      // CPU Clock (CCLK) = 60MHz using PLL
#define PCLK (CCLK/4)      // Peripheral Clock (PCLK) = 15MHz

// RTC Prescaler Calculation Macros
// The RTC requires a precise 32.768 kHz signal to track time accurately.
// These formulas calculate how much to divide the PCLK to reach that frequency.

// PREINT: Integer part of the prescaler (Calculated as: int(PCLK / 32768) - 1)
#define PREINT_VAL (int) ((PCLK / 32768) - 1)

// PREFRAC: Fractional part of the prescaler (The remainder of the division)
#define PREFRAC_VAL (PCLK -((PREINT_VAL + 1) * 32768))

// RTC Control Register (CCR) Bit Definitions
// Bit 0: Used to start or stop the clock counters
#define RTC_ENABLE (1<<0)

// Bit 1: Used to reset all time/date registers to zero
#define RTC_RESET (1<<1)

// Specific to LPC2148 Architecture
// Bit 4: Selects the clock source (External crystal vs Internal PCLK)
#define RTC_CLKSRC (1<<4)

// Human-readable aliases for the Day of Week (DOW) register
#define SUN 0
#define MON 1
#define TUE 2
#define WED 3
#define THU 4
#define FRI 5
#define SAT 6

typedef unsigned int u32;
typedef int s32;

/**
 * @brief Sets up the RTC hardware registers (PREINT, PREFRAC, CCR).
 */
void RTC_Init(void);

/**
 * @brief Reads the current time and formats it into an ASCII string for the LCD.
 */
void GetRTCTimeInfo(char*);

/**
 * @brief Reads the current date and formats it into an ASCII string for the LCD.
 */
void GetRTCDateInfo(char*);

/**
 * @brief Updates the RTC time registers with values provided by user input.
 */
void SetRTCTimeInfo(char*);

/**
 * @brief Updates the RTC date registers with values provided by user input.
 */
void SetRTCDateInfo(char*);

/**
 * @brief Retrieves the current day of the week (0-6).
 */
void GetRTCDay(s32 *dow);

/**
 * @brief Sets the current day of the week (0-6).
 */
void SetRTCDay(u32);
