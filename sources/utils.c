#include <stdio.h>
#include <stdlib.h>
#include <xc.h>

#include <p18f4620.h>
#include "utils.h"

extern char found;
extern char Nec_code1;
extern short Nec_OK;
extern char array1[21];
extern char duty_cycle;

char check_for_button_input()
{       
    if (Nec_OK == 1)
    {
            Nec_OK = 0;

//            printf ("NEC_Code = %x\r\n", Nec_code1);

            INTCON3bits.INT2IE = 1;          // Enable external interrupt
            INTCON2bits.INTEDG2 = 0;        // Edge programming for INT0 falling edge

            found = 0xff;
            for (int j=0; j< 21; j++)
            {
                if (Nec_code1 == array1[j]) 
                {
                    found = j;
                    j = 21;
                }
            }
            
            if (found == 0xff) 
            {
                printf ("Cannot find button \r\n");
                return (0);
            }
            else
            {
                return (1);
            }
    }
}

char bcd_2_dec (char bcd)
{
    int dec;
    dec = ((bcd>> 4) * 10) + (bcd & 0x0f);
    return dec;
}

int dec_2_bcd (char dec)
{
    int bcd;
    bcd = ((dec / 10) << 4) + (dec % 10);
    return bcd;
}

void Do_Beep()
{
    Activate_Buzzer();
    PORTDbits.RD3 = 1;
    Wait_One_Sec();
    Deactivate_Buzzer();
    PORTDbits.RD3 = 0;
    Wait_One_Sec();
    do_update_pwm(duty_cycle);
}

void Do_Beep_Good()
{
   Activate_Buzzer_2KHz();  // add code here using Activate_Buzzer_2KHz()
   PORTDbits.RD3 = 1;
   Wait_One_Sec();   
   Deactivate_Buzzer();
   PORTDbits.RD3 = 0;
   Wait_One_Sec();
   do_update_pwm(duty_cycle);
   
}

void Do_Beep_Bad()
{
    Activate_Buzzer_500Hz(); // add code here using Activate_Buzzer_500Hz()
    PORTDbits.RD3 = 1;
    Wait_One_Sec();    
    Deactivate_Buzzer();
    PORTDbits.RD3 = 0;
    Wait_One_Sec();
    do_update_pwm(duty_cycle);

}

void Wait_One_Sec()
{
    for (int k=0;k<0xffff;k++);
}

void Activate_Buzzer()
{
    PR2 = 0b11111001 ;
    T2CON = 0b00000101 ;
    CCPR2L = 0b01001010 ;
    CCP2CON = 0b00111100 ;
}

void Activate_Buzzer_500Hz()
{
    PR2 = 0b11111001 ;
    T2CON = 0b00000111 ;
    CCPR2L = 0b01111100 ;
    CCP2CON = 0b00111100 ;
    // add code here
}

void Activate_Buzzer_2KHz()
{
    PR2 = 0b11111001 ;
    T2CON = 0b00000101 ;
    CCPR2L = 0b01111100 ;
    CCP2CON = 0b00111100 ;
    // add code here
}

void Activate_Buzzer_4KHz()
{
    PR2 = 0b01111100;
    T2CON = 0b00000101;
    CCPR2L = 0b00111110;
    CCP2CON = 0b00011100;
    // add code here
}

void Deactivate_Buzzer()
{
    CCP2CON = 0x0;
	PORTBbits.RB3 = 0;
}

void do_update_pwm(char duty_cycle) 
{ 
	float dc_f;
	int dc_I;
	PR2 = 0b00000100 ;                      // Set the frequency for 25 Khz 
	T2CON = 0b00000111 ;                    // As given in website
	dc_f = ( 4.0 * duty_cycle / 20.0) ;     // calculate factor of duty cycle versus a 25 Khz signal
	dc_I = (int) dc_f;                      // Truncate integer
	if (dc_I > duty_cycle) dc_I++;          // Round up function
	CCP1CON = ((dc_I & 0x03) << 4) | 0b00001100;
	CCPR1L = (dc_I) >> 2;
}

void Set_ADCON0 (char ch)
{
    ADCON0 = ch * 4 + 1;
}

void Set_RGB_Color(char color)
{
   // if ((color > 7) || (color < 0)
    {
        PORTD = (PORTD & 0x7) | (color << 4);
        
        
    }
    // add code here
}

float read_volt()
{
    unsigned int val = get_full_ADC();
    float ans = ((val * 4)/1000.0);
    return ans;
    // add code here
}

unsigned int get_full_ADC()
{
    int result;
    ADCON0bits.GO = 1;
    while(ADCON0bits.DONE == 1);
    result = (ADRESH * 0x100) + ADRESL;
    return result;
    // add code here
}

void Init_ADC()
{
    ADCON0 = 0x01;
    ADCON1 = 0x0B;
    ADCON2 = 0xA9;
    // add code here
}


