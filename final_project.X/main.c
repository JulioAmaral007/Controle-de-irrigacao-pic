
// PIC16F877A Configuration Bit Settings

// 'C' source line config statements

// CONFIG
#pragma config FOSC = XT        // Oscillator Selection bits (XT oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bit (BOR disabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <PIC16F877A.h>
#include "lcd.h"

#define _XTAL_FREQ 8000000
#define LCD_ON LCD_Display(true, true, true);
#define LCD_OFF LCD_Display(false, false, false);

LCD lcd = {&PORTC, 1, 2, 3, 4, 5, 6};

//LCD
/*#define LCD_RS RC1_bit;
#define LCD_EN RC2_bit;
#define LCD_D4 RC3_bit;
#define LCD_D5 RC4_bit;
#define LCD_D6 RC5_bit;
#define LCD_D7 RC6_bit;

#define LCD_RS_Direction TRISC1_bit;
#define LCD_EN_Direction TRISC2_bit;
#define LCD_D4_Direction TRISC3_bit;
#define LCD_D5_Direction TRISC4_bit;
#define LCD_D6_Direction TRISC5_bit;
#define LCD_D7_Direction TRISC6_bit;*/
//End of LCD

int SetMoist=85;
int SetMoistR=0;
int SetMoistL=0;
int cset=0;
int sensingCounter=0;
int screenCounter=0;
int getMoist=0;
char buffer[2] = "00";
int turnON=1;

unsigned int ADC_Read(unsigned char channel)

{

  ADCON0 &= 0x11000101; //Clearing the Channel Selection Bits
  ADCON0 |= channel<<3; //Setting the required Bits
  __delay_ms(2); //Acquisition time to charge hold capacitor
  GO_nDONE = 1; //Initializes A/D Conversion
  while(GO_nDONE); //Wait for A/D Conversion to complete
  return ((ADRESH<<8)+ADRESL); //Returns Result

}

void ADC_Init() 
{
    ADRESL = 0x00;
    ADRESH = 0x00;

    ADCON0bits.ADCS1 = 1;
    ADCON0bits.ADCS0 = 1;
    ADCON0bits.CHS2 = 0;
    ADCON0bits.CHS1 = 0;
    ADCON0bits.CHS0 = 0;
    ADCON0bits.GO_nDONE = 0; //ONLY TURN ON WHEN READING
    ADCON0bits.ADON = 1;

    ADCON1bits.ADFM = 1;
    ADCON1bits.ADCS2 = 0;
    ADCON1bits.PCFG3 = 0;
    ADCON1bits.PCFG2 = 0;
    ADCON1bits.PCFG1 = 0;
    ADCON1bits.PCFG0 = 0;

}

void __interrupt() InterrupT(void)
{
    if(INTCONbits.RBIF ==1)
    {
        turnON=1;
        if(PORTBbits.RB7==1)
        {
            if(SetMoist!=99)
            {
                SetMoist++;
                cset=0;
            }
        }
        if(PORTBbits.RB6==1)
        {
            turnON=1;
            if(SetMoist!=5)
            {
                SetMoist--;
                cset=0;
            }
        }
        if(PORTBbits.RB5==1)
        {
            turnON=1;
            PORTCbits.RC0 = ~PORTCbits.RC0; 
        }
        if(PORTBbits.RB4==1)
        {
            turnON = 1;
        }
        INTCONbits.RBIF=0;
    }
    if(INTCONbits.TMR0IF == 1)
    {
        sensingCounter++;
        screenCounter++;    //2 minutes timer for moisture
        if(sensingCounter==50)
        {
            sensingCounter=0;
            getMoist=ADC_Read(0);
            getMoist=(getMoist/1024.0)*100.0;
            if(getMoist<SetMoist)
            {
                PORTCbits.RC0=1;
            }
            else if(getMoist>SetMoist)
            {
                PORTCbits.RC0=0;
            }
        }
        if(screenCounter==500)
        {
            screenCounter=0;
            turnON=0;
        }
        INTCONbits.TMR0IF=0;
    }
}

void main() 
{ 

    INTCONbits.GIE=1;
    INTCONbits.TMR0IE=1;
    INTCONbits.RBIE=1;
    //ENABLE GIE, TMR0IE, RBIE
    
    OPTION_REG = 0b10000111;
    //1:256, PORTB PULL UPS ARE DISABLED
    
    ADC_Init();
    //CONFIGURES ADC REGISTERS
    
    TRISB= 0xFF;
    TRISD = 0x00;
    TRISC = 0;
    PORTCbits.RC0=0;
    PORTDbits.RD7=1;
    PORTCbits.RC7= 1;
    TRISA = 1;
    
    LCD_Init(lcd);
    LCD_Set_Cursor(0,0);
    LCD_putrs("Calculating");
    LCD_Set_Cursor(1,0);
    LCD_putrs("Moisture...");    
    __delay_ms(1000);
    LCD_Clear();
    LCD_Set_Cursor(0,1);
    LCD_putrs("Moisture:");

    while(1)
    {
        if(cset==0)
        {
            //CALCULA DEZENA E UNIDADE DO LIMITE DA UMIDADE
            cset=1;
            SetMoistR= SetMoist % 10;
            switch (SetMoistR) 
            {
            case 0: SetMoistR=0x3F; break;
            case 1: SetMoistR=0x06; break;
            case 2: SetMoistR=0x5B; break;
            case 3: SetMoistR=0x4F; break;
            case 4: SetMoistR=0x66; break;
            case 5: SetMoistR=0x6D; break;
            case 6: SetMoistR=0x7D; break;
            case 7: SetMoistR=0x07; break;
            case 8: SetMoistR=0x7F; break;
            case 9: SetMoistR=0x6F; break;
            }
            
            SetMoistL= (SetMoist/10) % 10;
            switch (SetMoistL) 
            {
            case 0: SetMoistL=0x3F; break;
            case 1: SetMoistL=0x06; break;
            case 2: SetMoistL=0x5B; break;
            case 3: SetMoistL=0x4F; break;
            case 4: SetMoistL=0x66; break;
            case 5: SetMoistL=0x6D; break;
            case 6: SetMoistL=0x7D; break;
            case 7: SetMoistL=0x07; break;
            case 8: SetMoistL=0x7F; break;
            case 9: SetMoistL=0x6F; break;
            }
            
        }
        //MOSTRA O LIMITE DESEJADO NO DISPLAY DE 7 SEGMENTOS
        PORTD=SetMoistR;
        PORTCbits.RC7=0;
        PORTDbits.RD7=1;
        __delay_ms(100);
        PORTD=SetMoistL;
        PORTDbits.RD7=0;
        PORTCbits.RC7=1;
        __delay_ms(50);

        //BUFFER VAI SER O VALOR LIDO POR ADC_READ
        buffer[0] = (getMoist/10)%10 +48;
        buffer[1] = (getMoist)%10+48;
        LCD_Set_Cursor(0,10);
        LCD_putc(buffer[0]);
        LCD_Set_Cursor(0,11);
        LCD_putc(buffer[1]);
        LCD_Set_Cursor(0,12);
        LCD_putc('%');

        //VERIFICA SE A TELA ESTA LIGADA
        if(turnON==1)
        {
            LCD_ON;
        }
        else
        {
            LCD_OFF;
        }
    }
}      


