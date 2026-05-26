#ifndef INTERRUPT_H_
#define INTERRUPT_H_

#include <lpc21xx.h>
#include "rtc.h"

/*******************************************************************************
 * Hardware and VIC Configuration Macros
 ******************************************************************************/

/* External Interrupt 0 (EINT0) Configuration */
#define EINT0_INPUT_PIN 0x0000000C  /**< PINSEL bitmask to configure P0.1 as EINT0 */
#define EINT0_VIC_CHN0  14          /**< VIC Channel slot number assigned to EINT0 */

/* External Interrupt 1 (EINT1) Configuration */
#define EINT1_INPUT_PIN 0x000000C0  /**< PINSEL bitmask to configure P0.3 as EINT1 */
#define EINT1_VIC_CHN0  15          /**< VIC Channel slot number assigned to EINT1 */

/* Application Output Pins */
#define BUZZER          23          /**< GPIO pin number (P0.23) mapped to the Buzzer */

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/
typedef unsigned int u32;           /**< Standard shorthand for 32-bit unsigned integer */

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/**
 * @brief Initializes external interrupts EINT0 and EINT1.
 * Configures the associated GPIO pins via PINSEL, sets the interrupt type 
 * (edge/level triggered) in EXTMODE/EXTPOLAR, and registers the ISRs into the VIC.
 */
void Init_intrrupt(void);

/**
 * @brief Interrupt Service Routine (ISR) for External Interrupt 0.
 * Marked with "__irq" to let the compiler handle context saving/restoring automatically.
 * Must clear the EXTINT flag and update the VICVectAddr before exiting.
 */
void eint0_isr(void) __irq;

/**
 * @brief Interrupt Service Routine (ISR) for External Interrupt 1.
 * Handles the async event triggered on EINT1. Clears the EXTINT flag and 
 * clears the VICVectAddr on completion.
 */
void eint1_isr(void) __irq;

/**
 * @brief Compares two character strings.
 * Typically used to validate inputs, passwords, or configuration codes.
 * @param p Pointer to the first string.
 * @param q Pointer to the second string to compare against.
 * @return 0 if strings match, non-zero value if they differ.
 */
int check(char *p, char *q);

/**
 * @brief Displays or processes Secondary Menu Navigation (Menu Level 2).
 * Often triggered by an interrupt event to change modes or view application states.
 */
void menu2(void);

/**
 * @brief Displays or processes Tertiary Menu Navigation (Menu Level 3).
 * Provides deep settings adjustment options to the user interface.
 */
void menu3(void);

/**
 * @brief Runs runtime configuration routines.
 * Sets up basic operation thresholds, parameters, or internal software states 
 * before or during main system loops.
 */
void configure(void);

#endif /* INTERRUPT_H_ */
