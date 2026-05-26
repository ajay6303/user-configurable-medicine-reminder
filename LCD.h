#ifndef LCD_H_
#define LCD_H_

/**
 * @brief Generates a delay in microseconds.
 * @param dlyus Number of microseconds to delay.
 */
void delay_us(int dlyus);

/**
 * @brief Generates a delay in milliseconds.
 * @param dlyms Number of milliseconds to delay.
 */
void delay_ms(int dlyms);

/**
 * @brief Sends a raw byte (command or data) to the LCD data lines.
 * Typically handles the low-level latching (EN pin pulsing).
 * @param cmd The 8-bit value to write to the LCD.
 */
void WriteLCD(unsigned char cmd);

/**
 * @brief Sends a command instruction to the LCD (e.g., clear screen, move cursor).
 * Sets RS pin low before writing.
 * @param cmd The command hex code to be executed.
 */
void CmdLCD(unsigned char cmd);

/**
 * @brief Displays a single character on the LCD at the current cursor position.
 * Sets RS pin high before writing.
 * @param ascii The ASCII value of the character to display.
 */
void CharLCD(unsigned char ascii);

/**
 * @brief Initializes the LCD module.
 * Configures GPIO pins, sets the operating mode (4-bit or 8-bit), 
 * turns on the display, and clears the screen.
 */
void InitLCD(void);

/**
 * @brief Displays a null-terminated string on the LCD.
 * @param ptr Pointer to the character array (string) to be displayed.
 */
void StrLCD(signed char *ptr);

/**
 * @brief Converts an integer to a string and displays it on the LCD.
 * @param num The integer value to display.
 */
void intLCD(int num);

/**
 * @brief Displays a floating-point number on the LCD with a specified decimal precision.
 * @param dec The float value to display.
 * @param DP  Number of digits to display after the decimal point (Decimal Places).
 */
void FloatLCD(float dec, unsigned char DP);

#endif /* LCD_H_ */
