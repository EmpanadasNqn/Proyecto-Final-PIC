#include <xc.h>

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

#include <pic16f887.h>
#define boundrate = 15000 //velocidad de transmisión de datos
#define _XTAL_FREQ 4000000
#define led_infoBYTE RD3
#define transmitir 1 

/*
 MSSP Control Register 1 - SSPCON1
 MSSP Control Register 2 - SSPCON2
 MSSP Status Register    - SSPSTAT
 Serial Receive/Transmit Buffer Register  - SSPBUF
 MSSP Shift Register     - SSPSR  (not directly accessible)
 MSSP Adress Register    - SSPADD
 SBUFF register : Synchronous Serial Port Receive Buffer/Transmit Register
 */

//-------------------FUNCIONES RTC-------------------

void set_wait() {
    //asegura al master de qUe se ha llegado a la habilitación (IDLE)
    while ((SSPSTAT & 0x04) || (SSPCON2 & 0x1F));
}

void set_master() {
    SSPCON = 0x28;
    SSPCON2 = 0x00;
    SSPSTAT = 0x00;
    SSPADD = ((_XTAL_FREQ / 4) / boundrate) - 1;
    TRISC3 = 1;
    TRISC4 = 1;
}

void start() {
    //inicia la secuencia
    set_wait();
    SEN = 1; //datasheet : start condition enabled bit
    //con SEN inicializa SDA y SCL.
}

void stop() {
    set_wait();
    PEN = 1;
}

void restart() {
    set_wait();
    RSEN = 1;
}
//ACK master receiver mode only

void ACK() {
    set_wait();
    ACKDT = 0; // 0-> ACK y 1-> NACK
    ACKEN = 1; //mirar datasheet mañana
}
//NAKC master receiver mode only

void NACK() {
    set_wait();
    ACKDT = 1; // 1 -> NACK, 0 -> ACK
    ACKEN = 1; // Send NACK Signal!
}
//Send Byte Via I2C Bus, And Return The ACK/NACK From The Slave

unsigned char write((unsigned char Data) {
    set_wait();
    SSPBUF = Data;
    set_wait();
    return ACKSTAT; // acknowledge status bit
}
//recibe y devuelve byte de i2c bus

unsigned char read(void) {
    RCEN = 1; //activa start y recepción.
    while (!SSPIF); //espera hasta compilación
    SSPIF = 0; // clear d einterrupt flag bit
    return SSPBUF; // return de received byte
}
//-------------------FIN DE FUNCIONES RTC-------------------

//-----------------------------MAIN-------------------------

void main() {
    TRISB = 0xFF;
    /*TRISD = 0x00;
    PORTD = 0x00;*/
    unsigned char i = 1;
    set_master();
    //---------------master transmitter firmaware----------------
    if (transmitir){
        start();     // I2C Start Sequence
        write(0x40); // I2C Slave Device Address + Write
        write(i++);  // The Data To Be Sent
        stop();      // I2C Stop Sequence
        __delay_ms(500);
        led_infoBYTE = ~led_infoBYTE; // Toggle LED Each Time A Byte Is Sent!
    }
    //----------------master receiver firmaware------------------
    if (!transmitir) {
        start();        // I2C Start Sequence
        write(0x41);    // I2C Slave Device Address 0x40 + Read
        PORTD = read(); // Read Data From Slave
        stop();         // I2C Stop Sequence
        __delay_ms(100);
    }
    /*TRANSMITIR: "atar" su valor a una alarma de RTC para que varie cada 30s*/

}