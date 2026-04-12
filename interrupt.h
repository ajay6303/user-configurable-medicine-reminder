#include<lpc21xx.h>

#include "rtc.h"

//EINT0
#define EINT0_INPUT_PIN 0x0000000C
#define EINT0_VIC_CHN0 14
//EINT1
#define EINT1_INPUT_PIN 0x000000C0
#define EINT1_VIC_CHN0 15

#define BUZZER 23

void Init_intrrupt(void);
void eint0_isr(void)__irq;
void eint1_isr(void)__irq;
int check(char*p,char*q);
void menu2(void);
void menu3(void);
void configure(void);


typedef unsigned int u32;
