//interrupt.c

#include "interrupt.h"
#include <lpc21xx.h>
#include "lcd_defines.h"
#include "LCD.h"
#include "rtc.h"
#include "KPM.h"

/* Global and External Variables */
unsigned int year_val;
char dayLUT[][4] = {"SUN","MON","TUE","WED","THU","FRI","SAT"}; // Look-up table for days of the week
unsigned int cnt;
extern int day, last_triggered_minute;
int i, temp;
extern int flag, flag1, flag3, flag4;
extern char key, m1[], m2[], m3[], time[], date[];
extern int alert_timer;

/**
 * @brief Helper function to compare two 5-character arrays (e.g., matching "HH:MM")
 * @param p Pointer to first string/array
 * @param q Pointer to second string/array
 * @return 1 if matching, 0 if a mismatch occurs
 */
int check(char* p, char* q)
{
	int i = 0;
	while(i < 5)
	{
		if(p[i] != q[i])
			return 0; // Return mismatch
		i++;
	}
	return 1; // Strings match
}

/**
 * @brief Initializes External Interrupts EINT0 and EINT1 on the LPC21xx
 * Configures pins, Vectored Interrupt Controller (VIC) channels, and trigger modes.
 */
void Init_intrrupt(void)
{
	/* Configure P0.1 and P0.3 as EINT0 and EINT1 respectively */
	// Clear bit pairs 2&3 (P0.1) and 4&5 (P0.3) safely using a mask
	PINSEL0 &= ((u32)~3<<2) | ((u32)~3<<4);
	// Set the respective pin functions to EINT0 and EINT1
	PINSEL0 |= EINT0_INPUT_PIN | EINT1_INPUT_PIN;
	
	/* Configure the Vectored Interrupt Controller (VIC) */
	// Set EINT0 and EINT1 as IRQ type (VICIntSelect bit = 0 by default)
	// Enable interrupt channels for EINT0 and EINT1
	VICIntEnable = (1 << EINT0_VIC_CHN0) | (1 << EINT1_VIC_CHN0);
	
	/* Configure EINT0 Slot in VIC (Highest Priority: Slot 1 used here) */
	VICVectCntl1 = (1 << 5) | EINT0_VIC_CHN0; // Bit 5 enables the vectored slot
	VICVectAddr1 = (u32) eint0_isr;           // Load ISR address into slot vector register

	/* Configure EINT1 Slot in VIC (Next Highest Priority: Slot 0 used here) */
	VICVectCntl0 = (1 << 5) | EINT1_VIC_CHN0; // Bit 5 enables the vectored slot
	VICVectAddr0 = (u32) eint1_isr;           // Load ISR address into slot vector register

	/* Configure Triggering Mode in External Interrupt Peripheral */
	EXTMODE = ((1 << 1) | (1 << 0));          // Set both EINT0 and EINT1 to Edge-Triggered mode
	// EXTPOLAR = 0;                          // Default: Configured as Falling Edge triggered
}

/**
 * @brief Interrupt Service Routine for EINT0
 * Typically used to detect system state adjustments or external configuration signals.
 */
void eint0_isr(void) __irq
{
	EXTINT = 1 << 0;     // Clear EINT0 interrupt flag in the Ext Int block
	VICVectAddr = 0;     // Acknowledge interrupt execution to the VIC dummy register
	
	// Check if the buzzer pin state is low
	if(((IOPIN0 >> BUZZER) & 1) == 0)
	{
		flag = 1;        // Raise application flag
		IOCLR0 = 1 << 1; // Clear pin P0.1 / Status indication
	}
}

/**
 * @brief Interrupt Service Routine for EINT1
 * Acts as the physical "Acknowledge Button" to log medicine intake and shut off reminders.
 */
void eint1_isr(void) __irq 
{
	EXTINT = 1 << 1;        // Clear EINT1 interrupt flag in the Ext Int block
	VICVectAddr = 0;        // Acknowledge interrupt execution to the VIC dummy register
	
	// Check if the buzzer is actively sounding (High state)
	if(((IOPIN0 >> BUZZER) & 1) == 1)
	{
		IOCLR0 = 1 << BUZZER;   // Turn off the buzzer immediately
		alert_timer = 0;        // Clear out the software running timer duration
		
		CmdLCD(CLR);
		StrLCD((signed char*)"Medicine Taken"); // Display visual validation to user
		delay_ms(1000);
		CmdLCD(CLR);
		
		/* Update tracking states to prevent immediate re-triggering */
		flag4 = 1;
		flag3 = 0;
		last_triggered_minute = -1;
	}
	else if(flag1 == 1)
	{
		// Context block reserved for alternative tracking states
	}
	else
	{
		// Fallback execution if button is pressed with no alert active
		CmdLCD(CLR);
		StrLCD((signed char*)"No Remindiers"); 
		delay_ms(1000);
		CmdLCD(CLR);
	}
}

/**
 * @brief Menu Interface for configuring RTC variables (Time, Day, Date)
 */
void menu2(void)
{
	while(1)
	{
		CmdLCD(CLR);
		CmdLCD(GOTO_L1_POSN0);
		StrLCD((signed char*)"1.RTC Time 2.Dy");
		CmdLCD(GOTO_L2_POSN0);
		StrLCD((signed char*)"3.RTC Date 4.Q");
		key = KeyScan(); // Fetch matrix keypad input
		
		switch(key)
		{
			/* Submenu 1: Adjust RTC Time (Hours, Minutes, Seconds) */
			case '1':
				while(1)
				{
					CmdLCD(CLR);
					CmdLCD(GOTO_L1_POSN0);
					StrLCD((signed char*)time);//00:00:00
					CmdLCD(GOTO_L1_POSN0 + 11);
					StrLCD((signed char*)"4.exit");
					CmdLCD(GOTO_L2_POSN0);
					StrLCD((signed char*)"1.hr 2.min 3.sec");
					key = KeyScan();
					
					switch(key)
					{
						// Configure Hours
						case '1':
							flag3 = 1;
							g: 
							CmdLCD(CLR);
							CmdLCD(GOTO_L1_POSN0);
							StrLCD((signed char*)time);
							CmdLCD(GOTO_L1_POSN0);
							time[0] = KeyScan(); // Scan tens digit for hour
							CharLCD(time[0]);
							
							if(time[0] == 'C') // Clear/Escape condition
							{
								time[0] = '0';
								goto g;
							}
							else if(time[0] >= '0' && time[0] <= '2')
							{
								h2:
								CmdLCD(CLR);
								CmdLCD(GOTO_L1_POSN0);
								StrLCD((signed char*)time);
								CmdLCD(GOTO_L1_POSN0 + 1);
								time[1] = KeyScan(); // Scan units digit for hour
								
								if(time[1] == 'C')
								{
									time[0] = '0'; time[1] = '0';
									goto g;
								}
								// Bound check validation for 24-Hour format (00 to 23)
								else if(((time[0] >= '0' && time[0] <= '1') && (time[1] >= '0' && time[1] <= '9'))
									|| (time[0] == '2' && (time[1] >= '0') && (time[1] <= '3')))
								{
									CharLCD(time[1]);
									SetRTCTimeInfo(time); // Commit to internal RTC registers
								}
								else
								{
									time[1] = '0';
									CmdLCD(CLR);
									CmdLCD(GOTO_L1_POSN0);
									StrLCD((signed char*)"Invalid Hour");
									CmdLCD(GOTO_L2_POSN0);
									StrLCD((signed char*)"Try Again");
									delay_ms(100);
									goto h2;
								}
							}
							else
							{
								time[0] = '0';
								CmdLCD(CLR);
								CmdLCD(GOTO_L1_POSN0);
								StrLCD((signed char*)"Invalid Hour");
								CmdLCD(GOTO_L2_POSN0);
								StrLCD((signed char*)"Try Again");
								delay_ms(100);
								goto g;
							}	
							break;
						
						// Configure Minutes
						case '2':
							flag3 = 1;
							h: 
							CmdLCD(CLR);
							CmdLCD(GOTO_L1_POSN0);
							StrLCD((signed char*)time);
							CmdLCD(GOTO_L1_POSN0 + 3);
							time[3] = KeyScan(); // Scan tens digit for minute
							CharLCD(time[3]);
							
							if(time[3] == 'C')
							{
								time[3] = '0';
								goto h;
							}
							else if(time[3] >= '0' && time[3] <= '5') // Max bound tens is 5
							{
								min: 
								CmdLCD(CLR);
								CmdLCD(GOTO_L1_POSN0);
								StrLCD((signed char*)time);
								CmdLCD(GOTO_L1_POSN0 + 4);
								time[4] = KeyScan(); // Scan units digit for minute
								
								if(time[4] == 'C')
								{
									time[3] = '0'; time[4] = '0';
									goto h;
								}
								else if(time[4] >= '0' && time[4] <= '9')
								{
									CharLCD(time[4]);
									SetRTCTimeInfo(time); // Commit update
								}
								else
								{
									time[4] = '0';
									CmdLCD(CLR);
									CmdLCD(GOTO_L1_POSN0);
									StrLCD((signed char*)"Invalid Minute");
									CmdLCD(GOTO_L2_POSN0);
									StrLCD((signed char*)"Try Again");
									delay_ms(100);
									goto min;
								}
							}
							else
							{
								time[3] = '0';
								CmdLCD(CLR);
								CmdLCD(GOTO_L1_POSN0);
								StrLCD((signed char*)"Invalid Minute");
								CmdLCD(GOTO_L2_POSN0);
								StrLCD((signed char*)"Try Again");
								delay_ms(100);
								goto h;
							}
							break;
						
						// Configure Seconds
						case '3':
							flag3 = 1;
							n: 
							CmdLCD(CLR);
							CmdLCD(GOTO_L1_POSN0);
							StrLCD((signed char*)time);
							CmdLCD(GOTO_L1_POSN0 + 6);
							time[6] = KeyScan(); // Scan tens digit for seconds
							CharLCD(time[6]);
							
							if(time[6] == 'C')
							{
								time[6] = '0';
								goto n;
							}
							else if(time[6] >= '0' && time[6] <= '5') // Max bound tens is 5
							{
								sec: 
								CmdLCD(CLR);
								CmdLCD(GOTO_L1_POSN0);
								StrLCD((signed char*)time);
								CmdLCD(GOTO_L1_POSN0 + 7);
								time[7] = KeyScan(); // Scan units digit for seconds
								
								if(time[7] == 'C')
								{
									time[6] = '0'; time[7] = '0';
									goto n;
								}
								else if(time[7] >= '0' && time[7] <= '9')
								{
									CharLCD(time[7]);
									SetRTCTimeInfo(time); // Commit update
								}
								else
								{
									time[7] = '0';
									CmdLCD(CLR);
									CmdLCD(GOTO_L1_POSN0);
									StrLCD((signed char*)"Invalid Second");
									CmdLCD(GOTO_L2_POSN0);
									StrLCD((signed char*)"Try Again");
									delay_ms(100);
									goto sec;
								}
							}
							else
							{
								time[6] = '0';
								CmdLCD(CLR);
								CmdLCD(GOTO_L1_POSN0);
								StrLCD((signed char*)"Invalid Second");
								CmdLCD(GOTO_L2_POSN0);
								StrLCD((signed char*)"Try Again");
								delay_ms(100);
								goto n;
							}
							break;
							
						case '4': break; // Escape to main configuration layout
						
						default:
							CmdLCD(CLR);
							CmdLCD(GOTO_L1_POSN0);
							StrLCD((signed char*)"Invalid Input");
							CmdLCD(GOTO_L2_POSN0);
							StrLCD((signed char*)"Try Again");
							delay_ms(1000);
							break;
					}
					if(key == '4')
						break;
				}
				break;
						
			/* Submenu 2: Configure RTC Day of the week */
			case '2':
				while(1)
				{
					CmdLCD(CLR);
					CmdLCD(GOTO_L1_POSN0);
					StrLCD((signed char*)"0.S 1.M 2.TU 3.W");
					CmdLCD(GOTO_L2_POSN0);
					StrLCD((signed char*)"4.T 5.F 6.SA 7.Q");
					key = KeyScan();
					
					if(!(key >= '0' && key <= '7'))
					{
						CmdLCD(CLR);
						CmdLCD(GOTO_L1_POSN0);
						StrLCD((signed char*)"Invalid Day");
						CmdLCD(GOTO_L2_POSN0);
						StrLCD((signed char*)"Try Again");
						delay_ms(100);
					}
					else if(key == '7') // Quit option
					{
						break;
					}
					else
					{
						SetRTCDay((int)key - '0'); // Convert ASCII to integer index and save
						CmdLCD(CLR);
						CmdLCD(GOTO_L1_POSN0);
						StrLCD((signed char*)"Day saved!");
						delay_ms(500);
					}
				}
				break;

			/* Submenu 3: Configure RTC Date Details (Date, Month, Year) */
			case '3':
				while(1)
				{
					CmdLCD(CLR);
					CmdLCD(GOTO_L1_POSN0);
					StrLCD((signed char*)date);
					CmdLCD(GOTO_L1_POSN0 + 11);
					StrLCD((signed char*)"4.exit");
					CmdLCD(GOTO_L2_POSN0);
					StrLCD((signed char*)"1.dt 2.mon 3.yr");
					key = KeyScan();
					
					switch(key)
					{
						// Configure Numeric Day/Date (01 to 31)
						case '1':
							i: 
							CmdLCD(CLR);
							CmdLCD(GOTO_L1_POSN0);
							StrLCD((signed char*)date);
							CmdLCD(GOTO_L1_POSN0);
							date[0] = KeyScan(); // Tens digit
							CharLCD(date[0]);
							
							if(date[0] == 'C')
							{
								date[0] = '0';
								goto i;
							}
							else if(date[0] >= '0' && date[0] <= '3')
							{
								date: 
								CmdLCD(CLR);
								CmdLCD(GOTO_L1_POSN0);
								StrLCD((signed char*)date);
								CmdLCD(GOTO_L1_POSN0 + 1);
								date[1] = KeyScan(); // Units digit
								
								if(date[1] == 'C')
								{
									date[0] = '0'; date[1] = '0';
									goto i;
								}
								// Explicit boundary rules logic validation for parsing days
								else if((date[0] == '0' && (date[1] >= '1' && date[1] <= '9'))
									|| ((date[0] >= '1' && date[0] <= '2') && (date[1] >= '0' && date[1] <= '9'))
									|| (date[0] == '3' && (date[1] >= '0') && date[1] <= '1'))
								{
									CharLCD(date[1]);
									SetRTCDateInfo(date); // Save layout
								}
								else
								{
									date[1] = '0';
									CmdLCD(CLR);
									CmdLCD(GOTO_L1_POSN0);
									StrLCD((signed char*)"Invalid Date");
									CmdLCD(GOTO_L2_POSN0);
									StrLCD((signed char*)"Try Again");
									delay_ms(100);
									goto date;
								}
							}
							else
							{
								date[0] = '0';
								CmdLCD(CLR);
								CmdLCD(GOTO_L1_POSN0);
								StrLCD((signed char*)"Invalid Date");
								CmdLCD(GOTO_L2_POSN0);
								StrLCD((signed char*)"Try Again");
								delay_ms(100);
								goto i;
							}
							break;
						
						// Configure Month (01 to 12)
						case '2':
							j: 
							CmdLCD(CLR);
							CmdLCD(GOTO_L1_POSN0);
							StrLCD((signed char*)date);
							CmdLCD(GOTO_L1_POSN0 + 3);
							date[3] = KeyScan(); // Month tens digit
							CharLCD(date[3]);
							
							if(date[3] == 'C')
							{
								date[3] = '0';
								goto i;
							}
							else if(date[3] >= '0' && date[3] <= '1')
							{
								month: 
								CmdLCD(CLR);
								CmdLCD(GOTO_L1_POSN0);
								StrLCD((signed char*)date);
								CmdLCD(GOTO_L1_POSN0 + 4);
								date[4] = KeyScan(); // Month units digit
								
								if(date[4] == 'C')
								{
									date[3] = '0'; date[4] = '0';
									goto j;
								}
								// Confirm month stays inside 01-09 or 10-12 parameters
								else if((date[3] == '0' && (date[4] >= '1' && date[4] <= '9')) ||
									(date[3] == '1' && (date[4] >= '0' && date[4] <= '2')))
								{
									CharLCD(date[1]); // Note: Potential intentional variable behavior checking string character indexes
									SetRTCDateInfo(date);
								}
								else
								{
									date[4] = '0';
									CmdLCD(CLR);
									CmdLCD(GOTO_L1_POSN0);
									StrLCD((signed char*)"Invalid month");
									CmdLCD(GOTO_L2_POSN0);
									StrLCD((signed char*)"Try Again");
									delay_ms(100);
									goto month;
								}
							}
							else
							{
								date[3] = '0';
								CmdLCD(CLR);
								CmdLCD(GOTO_L1_POSN0);
								StrLCD((signed char*)"Invalid month");
								CmdLCD(GOTO_L2_POSN0);
								StrLCD((signed char*)"Try Again");
								delay_ms(100);
								goto j;
							}
							break;						
						
						// Configure Year (Input range up to 4095 bounds dictated by hardware constraints)
						case '3':
							k: 
							CmdLCD(CLR);
							StrLCD((signed char*)"Year(0-4095):");
							CmdLCD(GOTO_L2_POSN0);

							// Loop execution to collect 4 linear alphanumeric characters sequentially
							for(i = 0; i < 4; i++) {
								temp = KeyScan();
								
								// 1. Validate numerical ranges
								if(temp >= '0' && temp <= '9') {
									date[6+i] = temp;  // Maps explicitly to locations inside date tracking strings
									CharLCD(temp);      
								}
								// 2. Clear handling
								else if(temp == 'C') {
									goto k; 
								}
								// 3. Fallback tracking logic for system drops or erroneous layout inputs
								else {
									CmdLCD(CLR);
									StrLCD((signed char*)"Invalid Key!");
									delay_ms(1000);
									goto k; 
								}
							}

							// Translate characters to numerical equivalent variables
							year_val = ((date[6]-'0')*1000) + 
									   ((date[7]-'0')*100) + 
									   ((date[8]-'0')*10) + 
									   (date[9]-'0');

							// Ensure value satisfies structural hardware bounds of LPC integrated RTC architecture
							if (year_val > 4095) {
								CmdLCD(CLR);
								StrLCD((signed char*)"Max is 4095!");
								delay_ms(1000);
								goto k;
							} else {
								SetRTCDateInfo(date);
								StrLCD((signed char*)" - Saved!");
								delay_ms(1000);
							}
							break;
							
						case '4': break;
						
						default:
							CmdLCD(CLR);
							CmdLCD(GOTO_L1_POSN0);
							StrLCD((signed char*)"Invalid Input");
							CmdLCD(GOTO_L2_POSN0);
							StrLCD((signed char*)"Try Again");
							delay_ms(1000);
							break;
					}
					if(key == '4')
						break;
				}
				break;
						
			case '4': break;
			
			default:
				CmdLCD(CLR);
				CmdLCD(GOTO_L1_POSN0);
				StrLCD((signed char*)"Invalid Input");
				CmdLCD(GOTO_L2_POSN0);
				StrLCD((signed char*)"Try Again");
				delay_ms(1000);
				break;
		}
		if(key == '4')
			break;
	}
}

/**
 * @brief Menu layout configuration module to establish medication reminder intervals
 * Configures scheduling records across three individual reminder blocks (Med1, Med2, Med3).
 */
void menu3(void)
{
	while(1)
	{
		CmdLCD(CLR);
		CmdLCD(GOTO_L1_POSN0);
		StrLCD((signed char*)"1.Med1 3.Med3");
		CmdLCD(GOTO_L2_POSN0);
		StrLCD((signed char*)"2.Med2 4.Exit");
		key = KeyScan(); 
		
		switch(key)
		{
			/* Submenu Block: Medication Slot 1 */
			case '1':
				while(1)
				{
					CmdLCD(CLR);
					CmdLCD(GOTO_L1_POSN0);
					StrLCD((signed char*)m1);
					CmdLCD(GOTO_L2_POSN0);
					StrLCD((signed char*)"1.hr 2.min 3.Q");
					key = KeyScan();
					
					switch(key)
					{
						// Set Hour Profile for Med1
						case '1':
							a: 
							CmdLCD(CLR);
							CmdLCD(GOTO_L1_POSN0);
							StrLCD((signed char*)m1);
							CmdLCD(GOTO_L1_POSN0);
							m1[0] = KeyScan();
							CharLCD(m1[0]);
							
							if(m1[0] == 'C')
							{
								m1[0] = '0';
								goto a;
							}
							else if(m1[0] >= '0' && m1[0] <= '2')
							{
								m1h: 
								CmdLCD(CLR);
								CmdLCD(GOTO_L1_POSN0);
								StrLCD((signed char*)m1);
								CmdLCD(GOTO_L1_POSN0 + 1);
								m1[1] = KeyScan();
								
								if(m1[1] == 'C')
								{
									m1[0] = '0'; m1[1] = '0';
									goto a;
								}
								else if(((m1[0] >= '0' && m1[0] <= '1') && (m1[1] >= '0' && m1[1] <= '9'))
									|| (m1[0] == '2' && (m1[1] >= '0') && (m1[1] <= '3')))
								{
									CharLCD(m1[1]);
								}
								else
								{
									m1[1] = '0';
									CmdLCD(CLR);
									CmdLCD(GOTO_L1_POSN0);
									StrLCD((signed char*)"Invalid Hour");
									CmdLCD(GOTO_L2_POSN0);
									StrLCD((signed char*)"Try Again");
									delay_ms(100);
									goto m1h;
								}
							}
							else
							{
								m1[0] = '0';
								CmdLCD(CLR);
								CmdLCD(GOTO_L1_POSN0);
								StrLCD((signed char*)"Invalid Hour");
								CmdLCD(GOTO_L2_POSN0);
								StrLCD((signed char*)"Try Again");
								delay_ms(100);
								goto a;
							}	
							break;
					
						// Set Minute Profile for Med1
						case '2':
							b: 
							CmdLCD(CLR);
							CmdLCD(GOTO_L1_POSN0);
							StrLCD((signed char*)m1);
							CmdLCD(GOTO_L1_POSN0 + 3);
							m1[3] = KeyScan();
							CharLCD(m1[3]);
							
							if(m1[3] == 'C')
							{
								m1[3] = '0';
								goto b;
							}
							else if(m1[3] >= '0' && m1[3] <= '5')
							{
								m1m: 
								CmdLCD(CLR);
								CmdLCD(GOTO_L1_POSN0);
								StrLCD((signed char*)m1);
								CmdLCD(GOTO_L1_POSN0 + 4);
								m1[4] = KeyScan();
								
								if(m1[4] == 'C')
								{
									m1[3] = '0'; m1[4] = '0';
									goto b;
								}
								else if(m1[4] >= '0' && m1[4] <= '9')
								{
									CharLCD(m1[4]);
								}
								else
								{
									m1[4] = '0';
									CmdLCD(CLR);
									CmdLCD(GOTO_L1_POSN0);
									StrLCD((signed char*)"Invalid Minute");
									CmdLCD(GOTO_L2_POSN0);
									StrLCD((signed char*)"Try Again");
									delay_ms(100);
									goto m1m;
								}
							}
							else
							{
								m1[3] = '0';
								CmdLCD(CLR);
								CmdLCD(GOTO_L1_POSN0);
								StrLCD((signed char*)"Invalid Minute");
								CmdLCD(GOTO_L2_POSN0);
								StrLCD((signed char*)"Try Again");
								delay_ms(100);
								goto b;
							}
							break;
					
						case '3': break; // Return to parent level selection
					
						default:
							CmdLCD(CLR);
							CmdLCD(GOTO_L1_POSN0);
							StrLCD((signed char*)"Invalid Input");
							CmdLCD(GOTO_L2_POSN0);
							StrLCD((signed char*)"Try Again");
							delay_ms(1000);
							break;
					}
					if(key == '3')
						break;
				}
				break;
				
			/* Submenu Block: Medication Slot 2 */
			case '2':
				while(1)
				{
					CmdLCD(CLR);
					CmdLCD(GOTO_L1_POSN0);
					StrLCD((signed char*)m2);
					CmdLCD(GOTO_L2_POSN0);
					StrLCD((signed char*)"1.hr 2.min 3.Q");
					key = KeyScan();
					
					switch(key)
					{
						// Set Hour Profile for Med2
						case '1':
							c: 
							CmdLCD(CLR);
							CmdLCD(GOTO_L1_POSN0);
							StrLCD((signed char*)m2);
							CmdLCD(GOTO_L1_POSN0);
							m2[0] = KeyScan();
							CharLCD(m2[0]);
							
							if(m2[0] == 'C')
							{
								m2[0] = '0';
								goto c;
							}
							else if(m2[0] >= '0' && m2[0] <= '2')
							{
								m2h: 
								CmdLCD(CLR);
								CmdLCD(GOTO_L1_POSN0);
								StrLCD((signed char*)m2);
								CmdLCD(GOTO_L1_POSN0 + 1);
								m2[1] = KeyScan();
								
								if(m2[1] == 'C')
								{
									m2[0] = '0'; m2[1] = '0';
									goto c;
								}
								else if(((m2[0] >= '0' && m2[0] <= '1') && (m2[1] >= '0' && m2[1] <= '9'))
									|| (m2[0] == '2' && (m2[1] >= '0') && (m2[1] <= '3')))
								{
									CharLCD(m2[1]);
								}
								else
								{
									m2[1] = '0';
									CmdLCD(CLR);
									CmdLCD(GOTO_L1_POSN0);
									StrLCD((signed char*)"Invalid Hour");
									CmdLCD(GOTO_L2_POSN0);
									StrLCD((signed char*)"Try Again");
									delay_ms(100);
									goto m2h;
								}
							}
							else
							{
								m2[0] = '0';
								CmdLCD(CLR);
								CmdLCD(GOTO_L1_POSN0);
								StrLCD((signed char*)"Invalid Hour");
								CmdLCD(GOTO_L2_POSN0);
								StrLCD((signed char*)"Try Again");
								delay_ms(100);
								goto c;
							}		
							break;
					
						// Set Minute Profile for Med2
						case '2':
							d: 
							CmdLCD(CLR);
							CmdLCD(GOTO_L1_POSN0);
							StrLCD((signed char*)m2);
							CmdLCD(GOTO_L1_POSN0 + 3);
							m2[3] = KeyScan();
							CharLCD(m2[3]);
							
							if(m2[3] == 'C')
							{
								m2[3] = '0';
								goto d;
							}
							else if(m2[3] >= '0' && m2[3] <= '5')
							{
								m2m: 
								CmdLCD(CLR);
								CmdLCD(GOTO_L1_POSN0);
								StrLCD((signed char*)m2);
								CmdLCD(GOTO_L1_POSN0 + 4);
								m2[4] = KeyScan();
								
								if(m2[4] == 'C')
								{
									m2[3] = '0'; m2[4] = '0';
									goto d;
								}
								else if(m2[4] >= '0' && m2[4] <= '9')
								{
									CharLCD(m2[4]);
								}
								else
								{
									m2[4] = '0';
									CmdLCD(CLR);
									CmdLCD(GOTO_L1_POSN0);
									StrLCD((signed char*)"Invalid Minute");
									CmdLCD(GOTO_L2_POSN0);
									StrLCD((signed char*)"Try Again");
									delay_ms(100);
									goto m2m;
								}
							}
							else
							{
								m2[3] = '0';
								CmdLCD(CLR);
								CmdLCD(GOTO_L1_POSN0);
								StrLCD((signed char*)"Invalid Minute");
								CmdLCD(GOTO_L2_POSN0);
								StrLCD((signed char*)"Try Again");
								delay_ms(100);
								goto d;
							}
							break;
					
						case '3': break;
					
						default:
							CmdLCD(CLR);
							CmdLCD(GOTO_L1_POSN0);
							StrLCD((signed char*)"Invalid Input");
							CmdLCD(GOTO_L2_POSN0);
							StrLCD((signed char*)"Try Again");
							delay_ms(1000);
							break;
					}
					if(key == '3')
						break;
				}
				break;
				
			/* Submenu Block: Medication Slot 3 */
			case '3':
				while(1)
				{
					CmdLCD(CLR);
					CmdLCD(GOTO_L1_POSN0);
					StrLCD((signed char*)m3);
					CmdLCD(GOTO_L2_POSN0);
					StrLCD((signed char*)"1.hr 2.min 3.Q");
					key = KeyScan();
					
					switch(key)
					{
						// Set Hour Profile for Med3
						case '1':
							e: 
							CmdLCD(CLR);
							CmdLCD(GOTO_L1_POSN0);
							StrLCD((signed char*)m3);
							CmdLCD(GOTO_L1_POSN0);
							m3[0] = KeyScan();
							CharLCD(m3[0]);
							
							if(m3[0] == 'C')
							{
								m3[0] = '0';
								goto e;
							}
							else if(m3[0] >= '0' && m3[0] <= '2')
							{
								m3h: 
								CmdLCD(CLR);
								CmdLCD(GOTO_L1_POSN0);
								StrLCD((signed char*)m3);
								CmdLCD(GOTO_L1_POSN0 + 1);
								m3[1] = KeyScan();
								
								if(m3[1] == 'C')
								{
									m3[0] = '0'; m3[1] = '0';
									goto e;
								}
								else if(((m3[0] >= '0' && m3[0] <= '1') && (m3[1] >= '0' && m3[1] <= '9'))
									|| (m3[0] == '2' && (m3[1] >= '0') && (m3[1] <= '3')))
								{
									CharLCD(m3[1]);
								}
								else
								{
									m3[1] = '0';
									CmdLCD(CLR);
									CmdLCD(GOTO_L1_POSN0);
									StrLCD((signed char*)"Invalid Hour");
									CmdLCD(GOTO_L2_POSN0);
									StrLCD((signed char*)"Try again");
									delay_ms(100);
									goto m3h;
								}
							}
							else
							{
								m3[0] = '0';
								CmdLCD(CLR);
								CmdLCD(GOTO_L1_POSN0);
								StrLCD((signed char*)"Invalid Hour");
								CmdLCD(GOTO_L2_POSN0);
								StrLCD((signed char*)"Try again");
								delay_ms(100);
								goto e;
							}	
							break;
					
						// Set Minute Profile for Med3
						case '2':
							f: 
							CmdLCD(CLR);
							CmdLCD(GOTO_L1_POSN0);
							StrLCD((signed char*)m3);
							CmdLCD(GOTO_L1_POSN0 + 3);
							m3[3] = KeyScan();
							CharLCD(m3[3]);
							
							if(m3[3] == 'C')
							{
								m3[3] = '0';
								goto f;
							}
							else if(m3[3] >= '0' && m3[3] <= '5')
							{
								m3m: 
								CmdLCD(CLR);
								CmdLCD(GOTO_L1_POSN0);
								StrLCD((signed char*)m3);
								CmdLCD(GOTO_L1_POSN0 + 4);
								m3[4] = KeyScan();
								
								if(m3[4] == 'C')
								{
									m3[3] = '0'; m3[4] = '0';
									goto f;
								}
								else if(m3[4] >= '0' && m3[4] <= '9')
								{
									CharLCD(m3[4]);
								}
								else
								{
									m3[4] = '0';
									CmdLCD(CLR);
									CmdLCD(GOTO_L1_POSN0);
									StrLCD((signed char*)"Invalid Minute");
									CmdLCD(GOTO_L2_POSN0);
									StrLCD((signed char*)"Try again");
									delay_ms(100);
									goto m3m;
								}
							}
							else
							{
								m3[3] = '0';
								CmdLCD(CLR);
								CmdLCD(GOTO_L1_POSN0);
								StrLCD((signed char*)"Invalid Minute");
								CmdLCD(GOTO_L2_POSN0);
								StrLCD((signed char*)"Try again");
								delay_ms(100);
								goto f;
							}
							break;
					
						case '3': break;
					
						default:
							CmdLCD(CLR);
							CmdLCD(GOTO_L1_POSN0);
							StrLCD((signed char*)"Invalid Input");
							CmdLCD(GOTO_L2_POSN0);
							StrLCD((signed char*)"Try Again");
							delay_ms(1000);
							break;
					}
					if(key == '3')
						break;
				}
				break;
				
			case '4': break; // Exit medication configuration structure
				
			default:
				CmdLCD(CLR);
				CmdLCD(GOTO_L1_POSN0);
				StrLCD((signed char*)"Invalid Input");
				CmdLCD(GOTO_L2_POSN0);
				StrLCD((signed char*)"Try Again");
				delay_ms(1000);
				break;
		}
		if(key == '4')
			break;
	}
}

/**
 * @brief Main Entry Control Configuration Loop
 * Branches off into RTC modification menus or Medication edit trees based on key entry.
 */
void configure(void)
{
	flag = 0; // Clear dynamic context flag upon entry
	while(1)
	{
		/* Top-Level Menu Presentation
		   1. Real Time Clock Variables Configuration
		   2. Custom Alarm / Medicine Timing Records Management
		   3. Terminate back into persistent system clock readouts */
		CmdLCD(CLR);
		CmdLCD(GOTO_L1_POSN0);
		StrLCD((signed char*)"1.RTC Edit 3.Q");
		CmdLCD(GOTO_L2_POSN0);
		StrLCD((signed char*)"2.Medicine Edit");
		key = KeyScan(); 
		
		switch(key)
		{
			case '1':
				menu2(); // Call RTC Edit Routine
				break;
			
			case '2':
				menu3(); // Call Medicine Entry Edit Routine
				break;
			
			case '3': 
				break;   // Request Breakout
			
			default:
				CmdLCD(CLR);
				CmdLCD(GOTO_L1_POSN0);
				StrLCD((signed char*)"Invalid Input");
				CmdLCD(GOTO_L2_POSN0);
				StrLCD((signed char*)"Try Again");
				delay_ms(1000);
				break;
		}
		
		if(key == '3')
			break; // Force breakout of runtime configuration wrapper loop
	}
}
