/*
 * File:   newmain.c
 * Author: Marco
 *
 * Created on 1 settembre 2019, 20.42
 */

// CONFIG1H
#pragma config OSC = HS         // Oscillator Selection bits (XT Oscillator)
#pragma config FSCM = ON        // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor enabled)
#pragma config IESO = ON        // Internal External Switchover bit (Internal External Switchover mode enabled)

// CONFIG2L
#pragma config PWRT = OFF       // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOR = ON         // Brown-out Reset Enable bit (Brown-out Reset enabled)
// BORV = No Setting

// CONFIG2H
#pragma config WDT = OFF        // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))
#pragma config WDTPS = 32768    // Watchdog Timer Postscale Select bits (1:32768)

// CONFIG3H
#pragma config MCLRE = ON       // MCLR Pin Enable bit (MCLR pin enabled, RA5 input pin disabled)

// CONFIG4L
#pragma config STVR = ON        // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config LVP = OFF         // Low-Voltage ICSP Enable bit (Low-Voltage ICSP enabled)

// CONFIG5L
#pragma config CP0 = OFF        // Code Protection bit (Block 0 (00200-000FFFh) not code-protected)
#pragma config CP1 = OFF        // Code Protection bit (Block 1 (001000-001FFFh) not code-protected)

// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot Block (000000-0001FFh) not code-protected)
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM not code-protected)

// CONFIG6L
#pragma config WRT0 = OFF       // Write Protection bit (Block 0 (00200-000FFFh) not write-protected)
#pragma config WRT1 = OFF       // Write Protection bit (Block 1 (001000-001FFFh) not write-protected)

// CONFIG6H
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) not write-protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot Block (000000-0001FFh) not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protection bit (Block 0 (00200-000FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection bit (Block 1 (001000-001FFFh) not protected from table reads executed in other blocks)

// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot Block (000000-0001FFh) not protected from table reads executed in other blocks)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#define _XTAL_FREQ 8000000   

#include <xc.h>
#include <pic18f1320.h>

unsigned int pollo = 0;

unsigned long counter = 5;
void PausaStep()
{
    counter = 100;
    __delay_ms(50);
}

char UART_Init(const long int baudrate)
{
  long int x;
  x = (_XTAL_FREQ - baudrate*64)/(baudrate*64);   //SPBRG for Low Baud Rate
  if(x>255)                                       //If High Baud Rage Required
  {
    x = (_XTAL_FREQ - baudrate*16)/(baudrate*16); //SPBRG for High Baud Rate
    BRGH = 1;                                     //Setting High Baud Rate
  }
  if(x<256)
  {
    SPBRG = (unsigned char)x;                                    //Writing SPBRG Register
    SYNC = 0;                                     //Setting Asynchronous Mode, ie UART
    SPEN = 1;                                     //Enables Serial Port
    CREN = 1;                                     //Enables Continuous Reception
    TXEN = 1;                                     //Enables Transmission
    return 1;                                     //Returns 1 to indicate Successful Completion
  }
  return 0;                                       //Returns 0 to indicate UART initialization failed
}
char UART_Data_Ready()
{
  return RCIF;
}
char UART_Read()
{
  return RCREG;
}

void UART_Write(char data)
{
  PORTBbits.RB5 = 1;
  __delay_ms(2);
  while(!TRMT);
  TXREG = data;
  __delay_ms(2);
  PORTBbits.RB5 = 0;
}

unsigned long ActualStep = 0;

void main(void) {
    TRISB = 0x10;
    TRISA = 0;
    //???
    ADCON2 = 0b00011101;   //bit7: Left justification
    //bit6: unused
    //bit5-3: 6 TAD aquisition time
    //bit2-0: Fosc/16 Conversion Clock

    ADCON1 = 0b01111100;   //bit7: unused
    //bit6-0: all ports configured as digital (1) channels

    ADCON0 = 0b00001100;   //bit7-6: Set Vred+/- to Avdd/Avss (What does this mean?)
    //bit5: unused
    //bit4-2: Use analog channel 3 (Pin RA3)
    //bit1: GO/DONE bit set to 0 for now.
    //bit0: ADON not on (i turn it on and set GO/DONE to one in the loop)
    
    LATBbits.LATB5 = 0; //for RS-485
    UART_Init(9600);
	
	PausaStep();
	LATAbits.LATA0 = 1;
	LATBbits.LATB3 = 0;
	LATAbits.LATA1 = 0;
	LATBbits.LATB2 = 0;
	PausaStep();
	
    while(1)
    {
        if(RCSTAbits.OERR)
        {
            RCSTAbits.CREN = 0;
        }
        if(RCSTAbits.FERR)
        {
            pollo = RCREG;
        }
        
        if(UART_Data_Ready())
        {
            counter = 100;
            if(UART_Read() == 0x31)
            {
				switch(ActualStep)
				{
					case 0:		
						LATAbits.LATA0 = 1;
						LATBbits.LATB3 = 0;
						LATAbits.LATA1 = 0;
						LATBbits.LATB2 = 0;
						PausaStep();
						LATAbits.LATA0 = 1;
						LATBbits.LATB3 = 1;
						LATAbits.LATA1 = 0;
						LATBbits.LATB2 = 0;
						PausaStep();
						LATAbits.LATA0 = 0;
						LATBbits.LATB3 = 1;
						LATAbits.LATA1 = 0;
						LATBbits.LATB2 = 0;
						PausaStep();
						ActualStep++;
                        UART_Write(0x35);
						break;
					case 1:		
						LATAbits.LATA0 = 0;
						LATBbits.LATB3 = 1;
						LATAbits.LATA1 = 0;
						LATBbits.LATB2 = 0;
						PausaStep();
						LATAbits.LATA0 = 0;
						LATBbits.LATB3 = 1;
						LATAbits.LATA1 = 1;
						LATBbits.LATB2 = 0;
						PausaStep();
						LATAbits.LATA0 = 0;
						LATBbits.LATB3 = 0;
						LATAbits.LATA1 = 1;
						LATBbits.LATB2 = 0;
						PausaStep();
						ActualStep++;
                        UART_Write(0x35);
						break;
					case 2:		
						LATAbits.LATA0 = 0;
						LATBbits.LATB3 = 0;
						LATAbits.LATA1 = 1;
						LATBbits.LATB2 = 0;
						PausaStep();
						LATAbits.LATA0 = 0;
						LATBbits.LATB3 = 0;
						LATAbits.LATA1 = 1;
						LATBbits.LATB2 = 1;
						PausaStep();
						LATAbits.LATA0 = 0;
						LATBbits.LATB3 = 0;
						LATAbits.LATA1 = 0;
						LATBbits.LATB2 = 1;
						PausaStep();
						ActualStep++;
                        UART_Write(0x35);
						break;
					case 3:		
						LATAbits.LATA0 = 0;
						LATBbits.LATB3 = 0;
						LATAbits.LATA1 = 0;
						LATBbits.LATB2 = 1;
						PausaStep();
						LATAbits.LATA0 = 1;
						LATBbits.LATB3 = 0;
						LATAbits.LATA1 = 0;
						LATBbits.LATB2 = 1;
						PausaStep();
						LATAbits.LATA0 = 1;
						LATBbits.LATB3 = 0;
						LATAbits.LATA1 = 0;
						LATBbits.LATB2 = 0;
						PausaStep();
						ActualStep = 0;
                        UART_Write(0x35);
						break;
				}
            }
            if(UART_Read() == 0x32)
            {
				switch(ActualStep)
				{
					case 0:		
						LATAbits.LATA0 = 1;
						LATBbits.LATB3 = 0;
						LATAbits.LATA1 = 0;
						LATBbits.LATB2 = 0;
						PausaStep();
						LATAbits.LATA0 = 1;
						LATBbits.LATB3 = 0;
						LATAbits.LATA1 = 0;
						LATBbits.LATB2 = 1;
						PausaStep();
						LATAbits.LATA0 = 0;
						LATBbits.LATB3 = 0;
						LATAbits.LATA1 = 0;
						LATBbits.LATB2 = 1;
						PausaStep();
						ActualStep=3;
                        UART_Write(0x35);
						break;
					case 1:		
						LATAbits.LATA0 = 0;
						LATBbits.LATB3 = 1;
						LATAbits.LATA1 = 0;
						LATBbits.LATB2 = 0;
						PausaStep();
						LATAbits.LATA0 = 1;
						LATBbits.LATB3 = 1;
						LATAbits.LATA1 = 0;
						LATBbits.LATB2 = 0;
						PausaStep();
						LATAbits.LATA0 = 1;
						LATBbits.LATB3 = 0;
						LATAbits.LATA1 = 0;
						LATBbits.LATB2 = 0;
						PausaStep();
						ActualStep--;
                        UART_Write(0x35);
						break;
					case 2:		
						LATAbits.LATA0 = 0;
						LATBbits.LATB3 = 0;
						LATAbits.LATA1 = 1;
						LATBbits.LATB2 = 0;
						PausaStep();
						LATAbits.LATA0 = 0;
						LATBbits.LATB3 = 1;
						LATAbits.LATA1 = 1;
						LATBbits.LATB2 = 0;
						PausaStep();
						LATAbits.LATA0 = 0;
						LATBbits.LATB3 = 1;
						LATAbits.LATA1 = 0;
						LATBbits.LATB2 = 0;
						PausaStep();
						ActualStep--;
                        UART_Write(0x35);
						break;
					case 3:		
						LATAbits.LATA0 = 0;
						LATBbits.LATB3 = 0;
						LATAbits.LATA1 = 0;
						LATBbits.LATB2 = 1;
						PausaStep();
						LATAbits.LATA0 = 0;
						LATBbits.LATB3 = 0;
						LATAbits.LATA1 = 1;
						LATBbits.LATB2 = 1;
						PausaStep();
						LATAbits.LATA0 = 0;
						LATBbits.LATB3 = 0;
						LATAbits.LATA1 = 1;
						LATBbits.LATB2 = 0;
						PausaStep();
						ActualStep--;
                        UART_Write(0x35);
						break;
				}
            }
        }
        if(counter > 0 )
        {
            counter--;
            if(counter == 0)
            {
                LATAbits.LATA0 = 0;
                LATBbits.LATB3 = 0;
                LATAbits.LATA1 = 0;
                LATBbits.LATB2 = 0;
            }
        }
        
    }
    return;
}
