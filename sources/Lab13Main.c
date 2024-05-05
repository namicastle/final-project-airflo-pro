#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <math.h>
#include <p18f4620.h>
#include <usart.h>
#include <string.h>

#include "I2C.h"
#include "I2C_Support.h"
#include "Interrupt.h"
#include "Fan_Support.h"
#include "Main.h"
#include "ST7735_TFT.h"
#include "utils.h"

#pragma config OSC = INTIO67
#pragma config BOREN =OFF
#pragma config WDT=OFF
#pragma config LVP=OFF

void test_alarm();
char second = 0x00;
char minute = 0x00;
char hour = 0x00;
char dow = 0x00;
char day = 0x00;
char month = 0x00;
char year = 0x00;

char found;
char tempSecond = 0xff; 
signed int DS1621_tempC, DS1621_tempF;
char setup_second, setup_minute, setup_hour, setup_day, setup_month, setup_year;
char alarm_second, alarm_minute, alarm_hour, alarm_date;
char setup_alarm_second, setup_alarm_minute, setup_alarm_hour;
unsigned char fan_set_temp = 75;
unsigned char Nec_state = 0;
float volt;
char INT0_flag, INT1_flag, INT2_flag;

short Nec_OK = 0;
char Nec_code1;
char FAN;
char duty_cycle;
int rps;
int rpm;
int ALARMEN;
int alarm_mode, MATCHED,color;



char buffer[31]     = " ECE3301L Sp'23 Final\0";
char *nbr;
char *txt;
char tempC[]        = "+25";
char tempF[]        = "+77";
char time[]         = "00:00:00";
char date[]         = "00/00/00";
char alarm_time[]   = "00:00:00";
char Alarm_SW_Txt[] = "OFF";
char Fan_Set_Temp_Txt[] = "075F";
char Fan_SW_Txt[]   = "OFF";                // text storage for Heater Mode
char array1[21]={0xa2,0x62,0xe2,0x22,0x02,0xc2,0xe0,0xa8,0x90,0x68,0x98,0xb0,0x30,0x18,0x7a,0x10,0x38,0x5a,0x42,0x4a,0x52};

    
char DC_Txt[]       = "000";                // text storage for Duty Cycle value
char Volt_Txt[]     = "0.00V";
char RTC_ALARM_Txt[]= "0";                      //
char RPM_Txt[]      = "0000";               // text storage for RPM

char setup_time[]       = "00:00:00";
char setup_date[]       = "01/01/00";
char setup_alarm_time[] = "00:00:00"; 
char setup_fan_set_text[]   = "075F";

void Monitor_Fan();

int Equal_Time();


void putch (char c)
{   
    while (!TRMT);       
    TXREG = c;
}

void init_UART()
{
    OpenUSART (USART_TX_INT_OFF & USART_RX_INT_OFF & USART_ASYNCH_MODE & USART_EIGHT_BIT & USART_CONT_RX & USART_BRGH_HIGH, 25);
    OSCCON = 0x70;
}



void Do_Init()                      // Initialize the ports 
{ 
    init_UART();                    // Initialize the uart
    Init_ADC();
    OSCCON=0x70;                    // Set oscillator to 8 MHz 
    
    ADCON1=0x0E;
    TRISA = 0x11;
    TRISB = 0x07;
    TRISC = 0x01;
    TRISD = 0x00;
    TRISE = 0x00;
    PORTE = 0x00;

    FAN = 0;
    RBPU=0;

    TMR3L = 0x00;                   
    T3CON = 0x03;
    I2C_Init(100000); 

    DS1621_Init();
    Init_INTERRUPT();
    Turn_Off_Fan();
    fan_set_temp = 75;
}


char txt1[21][4] ={"CH-\0","CH \0","CH+\0","|<<\0",">>|\0",">||\0",
                   "VL-\0","VL+\0","EQ \0"," 0 \0","100\0","200\0",
                   " 1 \0"," 2 \0"," 3 \0"," 4 \0"," 5 \0"," 6 \0"," 7 \0",
                   " 8 \0"," 9 \0"};  // Array for buttons on remote



void main() 
{
   init_UART();             // Initialize the UART function
   Init_ADC();
    OSCCON = 0x70;           // 8 Mhz
    nRBPU = 0;               // Enable PORTB internal pull up resistor
    TRISA = 0x01;            // PORTA as input
	TRISB = 0x07;			 // PORTB as input
    TRISC = 0x01;            // PORTC as input
    TRISD = 0x00;            // PORTD as output
    TRISE = 0x00;            // PORTE as output 
//    ADCON1 = 0x0F;           // ADCON1 is set as 0x0F
    Initialize_Screen();     // Initialize the screen
    T3CON = 0x03;           // Set T3CON to 0x03
    
    I2C_Init(100000);       // Initialize I2C
    DS1621_Init();          // Initialize DS1621
	
    Init_INTERRUPT();        // Initialize Interrupt
	
	
    TMR1H = 0;              // Reset Timer1
    TMR1L = 0;              // TMR1L reset
    TMR3L = 0x00;
    INTCONbits.INT0IF = 0 ; // Clear INT0IF
    
    PIR1bits.TMR1IF = 0;    // Clear timer 1 interrupt flag
    PIE1bits.TMR1IE = 1;    // Enable Timer 1 interrupt
    INTCONbits.PEIE = 1;    // Enable Peripheral interrupt
    INTCONbits.GIE = 1;     // Enable global interrupts
    Nec_OK = 0;             // Clear flag and clear code
    Turn_Off_Fan();
    fan_set_temp = 50;
    //FAN = 0;    
    FAN_EN = 0;             // Turn Fan on
    FAN_EN_LED = 0;         // Turn Fan LED on
    
    do_update_pwm(duty_cycle);  // Update duty cycle
    
    alarm_mode = 0;
    ALARMEN = 0;
    Set_RGB_Color(0);
                
    
    DS3231_Turn_Off_Alarm();                                    
    DS3231_Read_Alarm_Time();                                   // Read alarm time for the first time

   
    
    tempSecond = 0xff;
    while (1)
    {
        DS3231_Read_Time();

        if(tempSecond != second)
        {
            do_update_pwm(duty_cycle);
            tempSecond = second;
            rpm = get_RPM();
            volt = read_volt();
            DS1621_tempC = DS1621_Read_Temp();
            DS1621_tempF = (DS1621_tempC * 9 / 5) + 32;
            
            Set_DC_RGB(duty_cycle);						//set the led light
            Set_RPM_RGB(rpm);

            printf ("%02x:%02x:%02x %02x/%02x/%02x",hour,minute,second,month,day,year);
            printf (" Temp = %d C = %d F ", DS1621_tempC, DS1621_tempF);
            printf ("alarm = %d match = %d", RTC_ALARM_NOT, MATCHED);
            printf ("RPM = %d  dc = %d\r\n", rpm, duty_cycle);
            printf ("volt = %f\r\n", volt);
            
            Monitor_Fan();
            test_alarm();
            Update_Screen();
        }
        
        if (check_for_button_input() == 1)
        {

            Nec_OK = 0;
            Enable_INT_Interrupt();
            printf ("Nec_button = %x \r\n", Nec_code1);       //array 1 characters 
            
            switch (found)
            {
                case Setup_Time_Key:        
                    Do_Beep_Good();
                    Do_Setup_Time();
                    break;
                
                case Setup_Alarm_Key:           
                    Do_Beep_Good();
                    Do_Setup_Alarm_Time();
                    break;
                    
                case Setup_Fan_Temp_Key:
                    Do_Beep_Good();
                    Setup_Temp_Fan();            
                    break;
                    
                case Toggle_Fan_Key:
                    Do_Beep_Good();
                    Toggle_Fan();
                     break;           
        
                default:
                    Do_Beep_Bad();
                    break;
            }
            printf("\r\n found = %d \r\n", found);
        } 
		
        if (INT1_flag == 1)
        {
            INT1_flag = 0;
            if (ALARMEN == 0) ALARMEN = 1;
            else ALARMEN = 0;
        }
        
    }
}

void test_alarm()
{
    if(alarm_mode == 0 && ALARMEN == 1)
    {
        DS3231_Turn_On_Alarm(); 
        alarm_mode = 1;
        MATCHED = 0;
    }
    if(alarm_mode == 1 && ALARMEN == 0)
    {
        DS3231_Turn_Off_Alarm(); 
        alarm_mode = 0;
        Set_RGB_Color(0);                                                                                                 // Turn off RGB LEDs
        Deactivate_Buzzer();                                                                                              // Reest match state
        MATCHED = 0;
    }
    if(alarm_mode == 1 && ALARMEN == 1)
    {
        //printf ("\r\n Printing test rtc alarm on = %d \r\n", RTC_ALARM_NOT);
        if(RTC_ALARM_NOT == 0 || Equal_Time() == 1)
        {
            Activate_Buzzer();
            MATCHED = 1;
        }
        if(MATCHED == 1)
        {
            Set_RGB_Color(color++);
            //printf ("\r\n Printing test alarm on = %d \r\n", MATCHED);
            if(color > 7)
            {
                color = 0;
            }
            volt = read_volt();
            if(volt > 2.8)
            {
                MATCHED = 0;                                                                                             
                color = 0;
                Set_RGB_Color(color);
                Deactivate_Buzzer();
                do_update_pwm(duty_cycle);
                alarm_mode = 0;
            }
            else
            {
                Activate_Buzzer();
            }
        }
    }
}

void Monitor_Fan()
{
	float de_float;
    int diff_temp;
    diff_temp = fan_set_temp - DS1621_tempF;
    
    if(FAN == 0)
    {
        Turn_Off_Fan();
    }
    else
    {
        Turn_On_Fan();
    }
    if(diff_temp < 0)
    {
        duty_cycle = 0;
        return;
    }
	if(diff_temp > 50)
	{
		duty_cycle = 100;
	}
	if(diff_temp >= 35 && diff_temp < 50)
	{
		duty_cycle = diff_temp * 2;
	}
	if(diff_temp >= 25 && diff_temp < 35)
	{
		de_float = diff_temp * 1.5;
		duty_cycle = (int)de_float;
	}
	if(diff_temp >= 10 && diff_temp < 25)
	{
		de_float = diff_temp * 2;
		duty_cycle = (int)de_float;
	}
	if(diff_temp >= 0 && diff_temp < 10)
	{
		duty_cycle = diff_temp * 5;
	}
    do_update_pwm(duty_cycle);
}


int Equal_Time()
{
    if(alarm_second == second && alarm_minute == minute && alarm_hour == hour)
    {
        RTC_ALARM_NOT = 0;
        return 1;
    }
    else
    {
        RTC_ALARM_NOT = 1;
        return 0;
    }
}


