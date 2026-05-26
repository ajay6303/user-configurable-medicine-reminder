#ifndef KPM_H_
#define KPM_H_

/**
 * @brief Initializes the Keypad Matrix.
 * Configures the row and column GPIO pins (e.g., setting rows as outputs 
 * and columns as inputs with internal pull-up resistors).
 */
void InitKPM(void);

/**
 * @brief Identifies which specific column has a pressed key.
 * Iterates through the columns or reads the input register to find 
 * the active low/high state caused by a button press.
 * @return The index or bitmask of the detected column, or -1/0 if none.
 */
int ColScan(void);

/**
 * @brief Validates the state of the rows to confirm a key press.
 * Often used to check if any key is currently pressed in the matrix 
 * before proceeding with full scanning.
 * @return Returns a non-zero value if a row activity is detected, 0 otherwise.
 */
int RowCheck(void);

/**
 * @brief Double-checks the column state to eliminate noise or debouncing.
 * Ensures that the column detection is stable and valid.
 * @return Returns a non-zero value if a valid column activity is confirmed.
 */
int ColCheck(void);

/**
 * @brief Performs the full keypad scanning state machine.
 * Coordinates driving the rows, reading the columns, debouncing, 
 * and decoding the matrix intersection into a physical character.
 * @return The ASCII character of the pressed key (e.g., '0'-'9', '*', '#', 'A'-'D'), 
 * or 0 (null character) if no key is pressed.
 */
char KeyScan(void);

#endif /* KPM_H_ */
