#pragma config FOSC = INTRC_NOCLKOUT    // Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF               // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = ON               // Power-up Timer Enable bit (PWRT enabled)
#pragma config MCLRE = ON               // RE3/MCLR pin function select bit (RE3/MCLR pin function is MCLR)
#pragma config CP = OFF                 // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF                // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = ON               // Brown Out Reset Selection bits (BOR enabled)
#pragma config IESO = ON                // Internal External Switchover bit (Internal/External Switchover mode is enabled)
#pragma config FCMEN = ON               // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is enabled)
#pragma config LVP = OFF                // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

#pragma config BOR4V = BOR40V           // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF                // Flash Program Memory Self Write Enable bits (Write protection off)
#include <xc.h>
#include <I2C_rtc.c>

//-----------------------INICIO MAIN------------------------
//---------------master transmitter firmaware----------------
#define led_infoBYTE RD3
void main() {
    TRISB = 0xFF;
  /*TRISD = 0x00;
    PORTD = 0x00;*/
    unsigned char i = 1;
    set_master();
    while (1) {
        start(); // I2C Start Sequence
        write(0x40); // I2C Slave Device Address + Write
        write(i++); // The Data To Be Sent
        stop(); // I2C Stop Sequence
        __delay_ms(500);
        led_infoBYTE = ~led_infoBYTE; // Toggle LED Each Time A Byte Is Sent!
    }
}
