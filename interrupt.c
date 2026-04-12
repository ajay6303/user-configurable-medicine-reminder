//interrupt.c

#include "interrupt.h"
#include<lpc21xx.h>
#include "lcd_defines.h"
#include "LCD.h"
#include "rtc.h"
#include "KPM.h"

unsigned int year_val;
char dayLUT[][4] = {"SUN","MON","TUE","WED","THU","FRI","SAT"};
unsigned int cnt;
extern int day,last_triggered_minute;
int i,temp;
extern int flag,flag1,flag3,flag4;
extern char key,m1[],m2[],m3[],time[],date[];

extern int alert_timer;

int check(char*p,char*q)
{
	int i=0;
	while(i<5)
	{
		if(p[i]!=q[i])
			return 0;
		i++;
	}
	return 1;
}

void Init_intrrupt(void)
{
	//cfg p0.1,p0.3 pin as EINT0,EINT1
  //input pins
	//clr bit pair 2&3 & bit pair 4&5,
	//w/o affecting other bits
	PINSEL0&=((u32)~3<<2)|((u32)~3<<4);
	//update bit2&3,bit4&5 for EINT0,EINT1
	//pin function
	PINSEL0|=EINT0_INPUT_PIN|EINT1_INPUT_PIN;
	
	//cfg VIC peripheral/block
	//allow EINT0,EINT1 as irq type
	//VICIntSelect=0; //default
	//enable EINT0,EINT1 through channel
	VICIntEnable=1<<EINT0_VIC_CHN0|
	             1<<EINT1_VIC_CHN0;
	
	//Cfg EINT0 as v.irq with highest priority(0)
	VICVectCntl1=(1<<5)|EINT0_VIC_CHN0;
	//load eint0_isr address into LUT sfr
  VICVectAddr1=(u32 ) eint0_isr;

  //Cfg EINT1 as v.irq with next
	//highest priority(1)
	VICVectCntl0=(1<<5)|EINT1_VIC_CHN0;
	//load eint1_isr address into LUT sfr
  VICVectAddr0=(u32 ) eint1_isr;

  //Cfg EINT0,EINT1 via 
	//External Interrupts Peripheral	
	//Enable EINT0,EINT1
	//EXTINT=0;//default
	//Cfg EINT0,EINT1 as edge triggered interrupt
	EXTMODE=((1<<1)|(1<<0));
	//cfg EINT0,EINT1 as falling edge triggered
	//EXTPOLAR=0;//default
	
	//cfg status led pin for EINT0,EINT1
 	//as gpio out

}
void eint0_isr(void) __irq
{
	
	//clear EINT0 Status in Ext Int Peripheral
	EXTINT=1<<0;
	//clear EINT0 status in VIC peripheral
	VICVectAddr=0;
	if(((IOPIN0>>BUZZER)&1)==0)
	{
	flag=1;
	IOCLR0=1<<1;
	}
}

void eint1_isr(void) __irq 
{
	
    EXTINT = 1 << 1;        // Clear EINT1 interrupt flag
    VICVectAddr = 0;        // Acknowledge VIC
    if(((IOPIN0>>BUZZER)&1)==1)
	{
    IOCLR0 = 1 << BUZZER;   // Immediately stop the buzzer
    alert_timer = 0;        // Reset the alert duration
    
    CmdLCD(CLR);
    StrLCD((signed char*)"Medicine Taken"); // Visual confirmation
    delay_ms(1000);
		CmdLCD(CLR);
		flag4=1;
		flag3=0;
		last_triggered_minute=-1;
	}
	else if(flag1==1)
	{
	}
	else
	{
		CmdLCD(CLR);
    StrLCD((signed char*)"No Remindiers"); // Visual confirmation
    delay_ms(1000);
		CmdLCD(CLR);
		
	}

}

//for RTC data
void menu2(void)
{
			while(1)
			{
			CmdLCD(CLR);
			CmdLCD(GOTO_L1_POSN0);
			StrLCD((signed char*)"1.RTC Time 2.Dy");
			CmdLCD(GOTO_L2_POSN0);
			StrLCD((signed char*)"3.RTC Date 4.Q");
			key=KeyScan(); //get input from keypad
			
				
			switch(key)
			{
				case'1':
					while(1)
					{
				CmdLCD(CLR);
			CmdLCD(GOTO_L1_POSN0);
			StrLCD((signed char*)time);
			CmdLCD(GOTO_L1_POSN0+11);
			StrLCD((signed char*)"4.exit");
			CmdLCD(GOTO_L2_POSN0);
			StrLCD((signed char*)"1.hr 2.min 3.sec");
				key=KeyScan();
				switch(key)
				{
				//hour
				case '1':flag3=1;
						g:CmdLCD(CLR);
							CmdLCD(GOTO_L1_POSN0);
							StrLCD((signed char*)time);
							CmdLCD(GOTO_L1_POSN0);
							time[0]=KeyScan();
							CharLCD(time[0]);
							if(time[0]=='C')
							{
								time[0]='0';
							goto g;
							}
							else if(time[0]>='0' && time[0]<='2')
							{
								h2:
									CmdLCD(CLR);
									CmdLCD(GOTO_L1_POSN0);
									StrLCD((signed char*)time);
									CmdLCD(GOTO_L1_POSN0+1);
									time[1]=KeyScan();
									if(time[1]=='C')
									{
										time[0]='0';
										time[1]='0';
										goto g;
									}
									else if(((time[0]>='0' && time[0]<='1') &&(time[1]>='0' && time[1]<='9'))
										|| (time[0]=='2' && (time[1]>='0')&&(time[1]<='3')))
									{
										
										CharLCD(time[1]);
										// Set the initial time (hours, minutes, seconds)
										SetRTCTimeInfo(time);
									}
									else
									{
											time[1]='0';
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
								time[0]='0';
								CmdLCD(CLR);
								CmdLCD(GOTO_L1_POSN0);
								StrLCD((signed char*)"Invalid Hour");
								CmdLCD(GOTO_L2_POSN0);
								StrLCD((signed char*)"Try Again");
								delay_ms(100);
								goto g;
							}	
						break;
						
						//minute
				case '2':flag3=1;
						h:CmdLCD(CLR);
							CmdLCD(GOTO_L1_POSN0);
							StrLCD((signed char*)time);
							CmdLCD(GOTO_L1_POSN0+3);
							time[3]=KeyScan();
							CharLCD(time[3]);
							if(time[3]=='C')
							{
								time[3]='0';
								goto h;
							}
							else if(time[3]>='0' && time[3]<='5')
							{
											min:CmdLCD(CLR);
										CmdLCD(GOTO_L1_POSN0);
										StrLCD((signed char*)time);
										CmdLCD(GOTO_L1_POSN0+4);
											time[4]=KeyScan();
										if(time[4]=='C')
										{
											time[3]='0';
											time[4]='0';
											goto h;
										}
										else if(time[4]>='0' && time[4]<='9')
										{
											CharLCD(time[4]);
											SetRTCTimeInfo(time);
										}
										else
										{
											time[4]='0';
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
							time[3]='0';
							CmdLCD(CLR);
							CmdLCD(GOTO_L1_POSN0);
							StrLCD((signed char*)"Invalid Minute");
							CmdLCD(GOTO_L2_POSN0);
							StrLCD((signed char*)"Try Again");
							delay_ms(100);
							goto h;
						}
						break;
						
						//seconds
				case '3':flag3=1;
						n:CmdLCD(CLR);
							CmdLCD(GOTO_L1_POSN0);
							StrLCD((signed char*)time);
							CmdLCD(GOTO_L1_POSN0+6);
							time[6]=KeyScan();
							CharLCD(time[6]);
							if(time[6]=='C')
							{
								time[6]='0';
								goto n;
							}
							else if(time[6]>='0' && time[6]<='5')
							{
								sec:CmdLCD(CLR);
								CmdLCD(GOTO_L1_POSN0);
								StrLCD((signed char*)time);
								CmdLCD(GOTO_L1_POSN0+7);
								time[7]=KeyScan();
								if(time[7]=='C')
								{
									time[6]='0';
									time[7]='0';
									goto n;
								}
								else if(time[7]>='0' && time[7]<='9')
								{
									CharLCD(time[7]);
								SetRTCTimeInfo(time);
								}
								else
								{
									time[7]='0';
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
								time[6]='0';
								CmdLCD(CLR);
								CmdLCD(GOTO_L1_POSN0);
								StrLCD((signed char*)"Invalid Second");
								CmdLCD(GOTO_L2_POSN0);
								StrLCD((signed char*)"Try Again");
								delay_ms(100);
								goto n;
							}
						break;
				case '4':break;
				default:CmdLCD(CLR);
							CmdLCD(GOTO_L1_POSN0);
							StrLCD((signed char*)"Invalid Input");
							CmdLCD(GOTO_L2_POSN0);
							StrLCD((signed char*)"Try Again");
							delay_ms(1000);
							break;
				}
					if(key=='4')
						break;
				}
			break;
						
						// RTC day
				case '2':
					while(1)
					{
							CmdLCD(CLR);
							CmdLCD(GOTO_L1_POSN0);
							StrLCD((signed char*)"0.S 1.M 2.TU 3.W");
							CmdLCD(GOTO_L2_POSN0);
							StrLCD((signed char*)"4.T 5.F 6.SA 7.Q");
							key=KeyScan();
							if(!(key>='0' && key<='7'))
							{
								CmdLCD(CLR);
								CmdLCD(GOTO_L1_POSN0);
								StrLCD((signed char*)"Invalid Day");
								CmdLCD(GOTO_L2_POSN0);
								StrLCD((signed char*)"Try Again");
								delay_ms(100);
								
							}
							else if(key=='7')
							{
								break;
							}
							{
								SetRTCDay((int)key-'0');
								CmdLCD(CLR);
							CmdLCD(GOTO_L1_POSN0);
							StrLCD((signed char*)"Day saved!");
								delay_ms(500);
							}
					}
					break;

						// RTC date
				case '3':
					while(1)
					{
				CmdLCD(CLR);
			CmdLCD(GOTO_L1_POSN0);
			StrLCD((signed char*)date);
			CmdLCD(GOTO_L1_POSN0+11);
			StrLCD((signed char*)"4.exit");
			CmdLCD(GOTO_L2_POSN0);
			StrLCD((signed char*)"1.dt 2.mon 3.yr");
				key=KeyScan();
				switch(key)
				{
					//date
					case '1':
					i:CmdLCD(CLR);
						CmdLCD(GOTO_L1_POSN0);
						StrLCD((signed char*)date);
						CmdLCD(GOTO_L1_POSN0);
						date[0]=KeyScan();
						CharLCD(date[0]);
						if(date[0]=='C')
						{
							date[0]='0';
							goto i;
						}
						else if(date[0]>='0' && date[0]<='3')
						{
									date:CmdLCD(CLR);
								CmdLCD(GOTO_L1_POSN0);
								StrLCD((signed char*)date);
								CmdLCD(GOTO_L1_POSN0+1);
								date[1]=KeyScan();
								if(date[1]=='C')
								{
									date[0]='0';
									date[1]='0';
									goto i;
								}
								else if((date[0]=='0'&&(date[1]>='1' && date[1]<='9'))
									||((date[0]>='1' && date[0]<='2')&&(date[1]>='0' && date[1]<='9'))
									|| (date[0]=='3' && (date[1]>='0')&&date[1]<='1'))
								{
									CharLCD(date[1]);
									SetRTCDateInfo(date);
								}
								else
								{
											date[1]='0';
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
									date[0]='0';
									CmdLCD(CLR);
									CmdLCD(GOTO_L1_POSN0);
									StrLCD((signed char*)"Invalid Date");
									CmdLCD(GOTO_L2_POSN0);
									StrLCD((signed char*)"Try Again");
									delay_ms(100);
									goto i;
							
						}
						break;
						
						//month
				case '2':
					j:CmdLCD(CLR);
						CmdLCD(GOTO_L1_POSN0);
						StrLCD((signed char*)date);
						CmdLCD(GOTO_L1_POSN0+3);
						date[3]=KeyScan();
						CharLCD(date[3]);
						if(date[3]=='C')
						{
							date[3]='0';
							goto i;
						}
						else if(date[3]>='0' && date[3]<='1')
						{
									month:CmdLCD(CLR);
								CmdLCD(GOTO_L1_POSN0);
								StrLCD((signed char*)date);
								CmdLCD(GOTO_L1_POSN0+4);
								date[4]=KeyScan();
								if(date[4]=='C')
								{
									date[3]='0';
									date[4]='0';
									goto j;
								}
								else if((date[3]=='0' &&( date[4]>='1'&&date[4]<='9')) ||
									(date[3]=='1' && ( date[4]>='0'&&date[4]<='2')))
								{
									CharLCD(date[1]);
								SetRTCDateInfo(date);
								}
								else
								{
											date[4]='0';
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
									date[3]='0';
									CmdLCD(CLR);
									CmdLCD(GOTO_L1_POSN0);
									StrLCD((signed char*)"Invalid month");
									CmdLCD(GOTO_L2_POSN0);
									StrLCD((signed char*)"Try Again");
									delay_ms(100);
									goto j;
							
						}
						break;						
						//year
				case '3':
    k: 
    CmdLCD(CLR);
    StrLCD((signed char*)"Year(0-4095):");
    CmdLCD(GOTO_L2_POSN0);

    // Collect 4 digits and check each one for "numeric only"
    for(i = 0; i < 4; i++) {
						temp = KeyScan();
        
        // 1. Check if the key is a digit (0-9)
        if(temp >= '0' && temp <= '9') {
            date[6+i] = temp;  // Store in date[6], date[7], date[8], or date[9]
            CharLCD(temp);      // Show the digit
        }
        // 2. If 'C' is pressed, restart
        else if(temp == 'C') {
            goto k; 
        }
        // 3. If anything else is pressed (like +, A, B, #)
        else {
            CmdLCD(CLR);
            StrLCD((signed char*)"Invalid Key!");
            delay_ms(1000);
            goto k; // Force restart because a non-digit was entered
        }
    }

    // --- After the loop, we have exactly 4 digits ---
								year_val = ((date[6]-'0')*1000) + 
                            ((date[7]-'0')*100) + 
                            ((date[8]-'0')*10) + 
                            (date[9]-'0');

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
				case '4':break;
				default:CmdLCD(CLR);
							CmdLCD(GOTO_L1_POSN0);
							StrLCD((signed char*)"Invalid Input");
							CmdLCD(GOTO_L2_POSN0);
							StrLCD((signed char*)"Try Again");
							delay_ms(1000);
							break;
	}
				if(key=='4')
					break;
			}
					break;
						
				case '4':break;
				default:
							CmdLCD(CLR);
							CmdLCD(GOTO_L1_POSN0);
							StrLCD((signed char*)"Invalid Input");
							CmdLCD(GOTO_L2_POSN0);
							StrLCD((signed char*)"Try Again");
							delay_ms(1000);
							break;
			}
		if(key=='4')
			break;
	}
}

//for medicine data
void menu3(void)
{
	while(1)
			{
			CmdLCD(CLR);
			CmdLCD(GOTO_L1_POSN0);
			StrLCD((signed char*)"1.Med1 3.Med3");
			CmdLCD(GOTO_L2_POSN0);
			StrLCD((signed char*)"2.Med2 4.Exit");
			key=KeyScan(); //get input from keypad
			switch(key)
			{
				//for medicine 1
				case '1':
				while(1)
				{
					CmdLCD(CLR);
			CmdLCD(GOTO_L1_POSN0);
			StrLCD((signed char*)m1);
			CmdLCD(GOTO_L2_POSN0);
			StrLCD((signed char*)"1.hr 2.min 3.Q");
				key=KeyScan();
				switch(key)
				{
					//for hour set
					case '1':
						a:CmdLCD(CLR);
						CmdLCD(GOTO_L1_POSN0);
						StrLCD((signed char*)m1);
						CmdLCD(GOTO_L1_POSN0);
						m1[0]=KeyScan();
						CharLCD(m1[0]);
					if(m1[0]=='C')
							{
								m1[0]='0';
							goto a;
							}
					else if(m1[0]>='0' && m1[0]<='2')
					{
							m1h:CmdLCD(CLR);
					CmdLCD(GOTO_L1_POSN0);
					StrLCD((signed char*)m1);
					CmdLCD(GOTO_L1_POSN0+1);
						m1[1]=KeyScan();
					if(m1[1]=='C')
					{
						m1[0]='0';
						m1[1]='0';
						goto a;
					}
					else if(((m1[0]>='0' && m1[0]<='1') &&(m1[1]>='0' && m1[1]<='9'))
										|| (m1[0]=='2' && (m1[1]>='0')&&(m1[1]<='3')))
									{
										
										CharLCD(m1[1]);
									}
									else
									{
											m1[1]='0';
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
								m1[0]='0';
								CmdLCD(CLR);
								CmdLCD(GOTO_L1_POSN0);
								StrLCD((signed char*)"Invalid Hour");
								CmdLCD(GOTO_L2_POSN0);
								StrLCD((signed char*)"Try Again");
								delay_ms(100);
								goto a;
							}	
			
					break;
					
					//for min set
					case '2':
						b:CmdLCD(CLR);
						CmdLCD(GOTO_L1_POSN0);
						StrLCD((signed char*)m1);
						CmdLCD(GOTO_L1_POSN0+3);
						m1[3]=KeyScan();
						CharLCD(m1[3]);
						if(m1[3]=='C')
							{
								m1[3]='0';
								goto b;
							}
							else if(m1[3]>='0' && m1[3]<='5')
							{
									m1m:CmdLCD(CLR);
									CmdLCD(GOTO_L1_POSN0);
									StrLCD((signed char*)m1);
									CmdLCD(GOTO_L1_POSN0+4);
										m1[4]=KeyScan();
									if(m1[4]=='C')
									{
										m1[3]='0';
										m1[4]='0';
										goto b;
									}
										else if(m1[4]>='0' && m1[4]<='9')
										{
											CharLCD(m1[4]);
										}
										else
										{
											m1[4]='0';
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
							m1[3]='0';
							CmdLCD(CLR);
							CmdLCD(GOTO_L1_POSN0);
							StrLCD((signed char*)"Invalid Minute");
							CmdLCD(GOTO_L2_POSN0);
							StrLCD((signed char*)"Try Again");
							delay_ms(100);
							goto b;
						}

					break;
					
					case '3':break; //exit
					
					default:CmdLCD(CLR);
						CmdLCD(GOTO_L1_POSN0);
						StrLCD((signed char*)"Invalid Input");
						CmdLCD(GOTO_L2_POSN0);
						StrLCD((signed char*)"Try Again");
						delay_ms(1000);
						break;
				}
				if(key=='3')
					break;
				}
				break;
				
				//for medicine 2
				case '2':
					while(1)
				{
				CmdLCD(CLR);
			CmdLCD(GOTO_L1_POSN0);
			StrLCD((signed char*)m2);
			CmdLCD(GOTO_L2_POSN0);
			StrLCD((signed char*)"1.hr 2.min 3.Q");
				key=KeyScan();
				switch(key)
				{
					//for hour set
					case '1':
					c:CmdLCD(CLR);
						CmdLCD(GOTO_L1_POSN0);
						StrLCD((signed char*)m2);
						CmdLCD(GOTO_L1_POSN0);
						m2[0]=KeyScan();
						CharLCD(m2[0]);
					if(m2[0]=='C')
							{
								m2[0]='0';
							goto c;
							}
					else if(m2[0]>='0' && m2[0]<='2')
					{
							m2h:CmdLCD(CLR);
					CmdLCD(GOTO_L1_POSN0);
					StrLCD((signed char*)m2);
					CmdLCD(GOTO_L1_POSN0+1);
						m2[1]=KeyScan();
					if(m2[1]=='C')
					{
						m2[0]='0';
						m2[1]='0';
						goto c;
					}
					else if(((m2[0]>='0' && m2[0]<='1') &&(m2[1]>='0' && m2[1]<='9'))
										|| (m2[0]=='2' && (m2[1]>='0')&&(m2[1]<='3')))
									{
										
										CharLCD(m2[1]);
									}
									else
									{
											m2[1]='0';
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
								m2[0]='0';
								CmdLCD(CLR);
								CmdLCD(GOTO_L1_POSN0);
								StrLCD((signed char*)"Invalid Hour");
								CmdLCD(GOTO_L2_POSN0);
								StrLCD((signed char*)"Try Again");
								delay_ms(100);
								goto c;
							}		
			
					break;
					
					//for min set
					case '2':
						d:CmdLCD(CLR);
						CmdLCD(GOTO_L1_POSN0);
						StrLCD((signed char*)m2);
						CmdLCD(GOTO_L1_POSN0+3);
						m2[3]=KeyScan();
						CharLCD(m2[3]);
						if(m2[3]=='C')
							{
								m2[3]='0';
								goto d;
							}
							else if(m2[3]>='0' && m2[3]<='5')
							{
									m2m:CmdLCD(CLR);
									CmdLCD(GOTO_L1_POSN0);
									StrLCD((signed char*)m2);
									CmdLCD(GOTO_L1_POSN0+4);
										m2[4]=KeyScan();
									if(m2[4]=='C')
									{
										m2[3]='0';
										m2[4]='0';
										goto d;
									}
										else if(m2[4]>='0' && m2[4]<='9')
										{
											CharLCD(m2[4]);
										}
										else
										{
											m2[4]='0';
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
							m2[3]='0';
							CmdLCD(CLR);
							CmdLCD(GOTO_L1_POSN0);
							StrLCD((signed char*)"Invalid Minute");
							CmdLCD(GOTO_L2_POSN0);
							StrLCD((signed char*)"Try Again");
							delay_ms(100);
							goto d;
						}

					break;
					
					case '3':break; //exit
					
					default:CmdLCD(CLR);
						CmdLCD(GOTO_L1_POSN0);
						StrLCD((signed char*)"Invalid Input");
						CmdLCD(GOTO_L2_POSN0);
						StrLCD((signed char*)"Try Again");
						delay_ms(1000);
						break;
				}
				if(key=='3')
					break;
				}
				break;
				
			//for medicine 3
				case '3':
					while(1)
				{
					CmdLCD(CLR);
			CmdLCD(GOTO_L1_POSN0);
			StrLCD((signed char*)m3);
			CmdLCD(GOTO_L2_POSN0);
			StrLCD((signed char*)"1.hr 2.min 3.Q");
				key=KeyScan();
				switch(key)
				{
					//for hour set
					case '1':
						e:CmdLCD(CLR);
						CmdLCD(GOTO_L1_POSN0);
						StrLCD((signed char*)m3);
						CmdLCD(GOTO_L1_POSN0);
						m3[0]=KeyScan();
						CharLCD(m3[0]);
					if(m3[0]=='C')
							{
								m3[0]='0';
							goto e;
							}
					else if(m3[0]>='0' && m3[0]<='2')
					{
							m3h:CmdLCD(CLR);
					CmdLCD(GOTO_L1_POSN0);
					StrLCD((signed char*)m3);
					CmdLCD(GOTO_L1_POSN0+1);
						m3[1]=KeyScan();
					if(m3[1]=='C')
					{
						m3[0]='0';
						m3[1]='0';
						goto e;
					}
					else if(((m3[0]>='0' && m3[0]<='1') &&(m3[1]>='0' && m3[1]<='9'))
										|| (m3[0]=='2' && (m3[1]>='0')&&(m3[1]<='3')))
									{
										
										CharLCD(m3[1]);
									}
									else
									{
											m3[1]='0';
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
								m3[0]='0';
								CmdLCD(CLR);
								CmdLCD(GOTO_L1_POSN0);
								StrLCD((signed char*)"Invalid Hour");
								CmdLCD(GOTO_L2_POSN0);
								StrLCD((signed char*)"Try again");
								delay_ms(100);
								goto e;
							}	
			
					break;
					
					//for min set
					case '2':
						f:CmdLCD(CLR);
						CmdLCD(GOTO_L1_POSN0);
						StrLCD((signed char*)m3);
						CmdLCD(GOTO_L1_POSN0+3);
						m3[3]=KeyScan();
						CharLCD(m3[3]);
						if(m3[3]=='C')
							{
								m3[3]='0';
								goto f;
							}
							else if(m3[3]>='0' && m3[3]<='5')
							{
									m3m:CmdLCD(CLR);
									CmdLCD(GOTO_L1_POSN0);
									StrLCD((signed char*)m3);
									CmdLCD(GOTO_L1_POSN0+4);
										m3[4]=KeyScan();
									if(m3[4]=='C')
									{
										m3[3]='0';
										m3[4]='0';
										goto f;
									}
										else if(m3[4]>='0' && m3[4]<='9')
										{
											CharLCD(m3[4]);
										}
										else
										{
											m3[4]='0';
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
							m3[3]='0';
							CmdLCD(CLR);
							CmdLCD(GOTO_L1_POSN0);
							StrLCD((signed char*)"Invalid Minute");
							CmdLCD(GOTO_L2_POSN0);
							StrLCD((signed char*)"Try again");
							delay_ms(100);
							goto f;
						}
						break;
					
					case '3':break; //exit
					
					default:CmdLCD(CLR);
						CmdLCD(GOTO_L1_POSN0);
						StrLCD((signed char*)"Invalid Input");
						CmdLCD(GOTO_L2_POSN0);
						StrLCD((signed char*)"Try Again");
						delay_ms(1000);
						break;
				}
				if(key=='3')
					break;
				}
				break;
				
				case '4':break; //exit
				
				default:CmdLCD(CLR);
						CmdLCD(GOTO_L1_POSN0);
						StrLCD((signed char*)"Invalid Input");
						CmdLCD(GOTO_L2_POSN0);
						StrLCD((signed char*)"Try Again");
						delay_ms(1000);
						break;
			}
			if(key=='4')
				break;
		}
}

void configure(void)
{
	flag=0;
	while(1)
	{
		/*menu-1
		1.time,date,day editing
		2.New Entry or update medicine details and Remainders
		3.exit menu and display real time clock*/
	CmdLCD(CLR);
	CmdLCD(GOTO_L1_POSN0);
	StrLCD((signed char*)"1.RTC Edit 3.Q");
	CmdLCD(GOTO_L2_POSN0);
	StrLCD((signed char*)"2.Medicine Edit");
	key=KeyScan(); //get input from keypad
	//Based on key value run the process
	
	switch(key)
	{
		/*menu 2
		1.hour,2.min,3.sec,4.day,5.date,6.month,7.year,8.quit*/
		case '1':
			menu2();
			break;
		
		/*menu 3
		1.1st medicine details 2.2nd medicine details 3.3rd medicine details*/
		case '2':
			menu3();
			break;
		
		case '3':break; //exit
		
		default:CmdLCD(CLR);
						CmdLCD(GOTO_L1_POSN0);
						StrLCD((signed char*)"Invalid Input");
						CmdLCD(GOTO_L2_POSN0);
						StrLCD((signed char*)"Try Again");
						delay_ms(1000);
						break;
	}
	
	if(key=='3')
		break;
	}
}

