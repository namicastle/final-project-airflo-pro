int get_RPM();
void Toggle_Fan();
void Turn_Off_Fan();
void Turn_On_Fan();
void Increase_Speed();
void Decrease_Speed();
void do_update_pwm(char) ;
void Set_DC_RGB(int);
void Set_RPM_RGB(int);

#define FAN_EN			PORTAbits.RA5       // Defines FAN_EN as PORTA 5
#define FAN_EN_LED		PORTAbits.RA4       // Defines FAN_EN_LED as PORTA 4
#define FAN_PWM			PORTCbits.RC2       // Defines FAN_PWM as PORTC 2





