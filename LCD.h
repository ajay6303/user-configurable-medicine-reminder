void delay_us(int dlyus);
void delay_ms(int dlyms);
void WriteLCD(unsigned char cmd);
void CmdLCD(unsigned char cmd);
void CharLCD(unsigned char ascii);
void InitLCD(void);
void StrLCD(signed char *ptr);
void intLCD(int num);
void FloatLCD(float dec,unsigned char  DP);
