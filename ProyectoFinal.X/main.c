/*
 * File:   main.c
 * Author: Santino Ponchiardi
 * Created on 19 de octubre de 2022, 03:32
 */

#define _XTAL_FREQ 4000000

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
#include <pic16f887.h>
#include <stdint.h>

#define Blue_LED_ON    49
#define Blue_LED_OFF   50
#define Yellow_Toggle  51

unsigned int datos = 0;

void iniUART_TX(void) {

    //Conf para que sea TX
    TRISC6 = 0;
    TRISC7 = 1;

    //Config para 8 bits asincronica
    BRGH = 1;
    BRG16 = 0;
    SYNC = 0;

    //Si trabajamos a 4Mhz funcionaria correctamente
    SPBRG = 25;

    //Hablitamos el sistema
    SPEN = 1;

    //TXEN = 1; Hace que se empiecen a generar los 9600 BR y este todo listo para que la comunicacion empiece.
}

void iniUART_RX(void) {

    //Conf para que sea RX
    TRISC6 = 1;
    TRISC7 = 0;
    
    //Config para 8 bits asincronica
    BRGH = 1;
    BRG16 = 0;
    SYNC = 0;

    //Si trabajamos a 4Mhz funcionaria correctamente
    SPBRG = 25;

    //Hablitamos el sistema
    SPEN = 1;
    
    //Configuracion de pines especificos para el receptor
    RCIE = 1;
    PEIE = 1;
    GIE = 1;

    CREN = 1; //Hace que se empiecen a generar los 9800BR y este todo listo para que la comunicacion empiece
}

void uartTXEscrituraStrings(unsigned char datos[]) { //En el caso de que lo que se quiera transmitir sea un String

    TXEN = 1;
    for (int i = 0; i < sizeof (datos); i++) {
        TXREG = datos[i];
        while (!TRMT) {
        }
    }
    TXEN = 0;
}

void uartTXEscritura(unsigned char datos) { //En el caso de que lo que se quiera transmitir sea una char

    TXEN = 1;
    while (!TRMT) {
        TXREG = datos;
    }
    TXEN = 0;
}

void ISR(void) {

    if (RCIF == 1) {
        datos = RCREG;
        RCIF = 0;
    }
}

void main(void) {
    
    iniUART_RX();
    
    TRISC0 = 0;
    TRISC1 = 0;
    RC0 = 0;
    RC1 = 0;
    
    ISR();
    if (datos == Blue_LED_ON) {RC0 = 1;}
    if (datos == Blue_LED_OFF) {RC0 = 0;}
    if (datos == Yellow_Toggle) {RC1 = ~RC1;}
    
    return;
}