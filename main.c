
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

#define _XTAL_FREQ 4000000

#include <xc.h>
#include <pic16f887.h>
#include <stdint.h>
//--------------------------------- VARIABLES UART -----------------------------

#define Blue_LED_ON    49
#define Blue_LED_OFF   50
#define Yellow_Toggle  51

unsigned char datos = '0';
//--------------------------------- FIN VARIABLES UART -----------------------------


//--------------PUERTOS ENTRADA PARA EDITAR LA HORA RELOJ-----------------------
#define hora 0            //varaible de bt
#define min  0            //varaible de bt
#define modificar 0       //varaible de bt
//led para avisar que se edita la hora
#define ledEditar RC0
#define digito1 PORTB
//Puertos de salida de los display (4 pines con transistores)
#define disp1 RD1
#define disp2 RD2
#define disp3 RD3
#define disp4 RD4
//configuraciones que se enviarán al rtc
int rtc_config[] = {0b00000000, hora, min, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
    0b00000000, 0b00000000, 0b10001111, 0b10000000, 0b10000000, 0b00000110, 0b00000000, 0b00000000, 0b00000000, 0b00000000};
//arreglo de numeros bcd para mostrar los numeros en el display
int numeros [] = {0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110, 0b01101101,
    0b01111100, 0b00000111, 0b01111111, 0b01100111};
//variables usadas para contar, llevar registro del tiempo, y para el control
int digito1;
int digito2;
int digito3;
int digito4;
int editando; //editando boolean
//---------------------FINALIZA VARIABLES RELOJ---------------------------------

//--------------------------------- VARIABLES I2C ------------------------------
#define led_infoBYTE RD3
#define transmitir 1 
#define boundrate = 15000               //velocidad de transmisión de datos
//--------------------------------- FIN VARIABLES I2C --------------------------

//------------------ FUNCIONES BLUETOOTH - UART --------------------------------

void iniUART_TX(void) {
    //Conf para que sea TX
    TRISC6 = 1;
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
    TRISC7 = 1;
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
    while (!TRMT);
    TXREG = datos;
    TXEN = 0;
}

void __interrupt() ISR(void) {
    if (RCIF == 1) {
        datos = RCREG;
        RCIF = 0;
    }
}
//------------------ FIN FUNCIONES BLUETOOTH - UART ----------------------------

//------------------ FUNCIONES RTC - HORA --------------------------------------

void display_fix() {
    PORTD = numeros[digito1];
    disp1 = 1;
    __delay_ms(5);
    PORTD = numeros[digito2];
    disp1 = 1;
    __delay_ms(5);
    PORTD = numeros[digito3];
    disp1 = 1;
    __delay_ms(5);
    PORTD = numeros[digito4];
    disp1 = 1;
    __delay_ms(5);
}

void editar_reloj() {
    editando = 1;
    hora = 0;
    min = 0;
    __delay_ms(500);
    // mientras editando = 1 repetir lo sig
    while (editando) {
        //si se activa el modificador 'hora', incrementar la hora y relizar los
        //cálculos para modificar el reloj
        if (hora == 1) {
            hora = 0;
            digito2 = digito2 + 1;
            if (digito2 > 9) {
                digito2 = 0;
                digito1 = digito1 + 1;
            }
            if (digito1 == 4 && digito1 == 2) { // o sea si decenas está en 2 y unidades en 4
                digito1 = 0;
                digito2 = 0;
            }
            __delay_ms(100);
        }
        if (min == 1) {
            min = 0;
            digito4 = digito4 + 1;
            if (digito4 > 9) {
                digito4 = 0;
                digito3 = digito3 + 1;
                if (digito3 > 5) {
                    digito3 = 0;
                }
            }
            __delay_ms(100);
        }
        /*SI modificar = 1 -> salir de la edición de la hora y volver al reloj*/
        if (modificar == 1) {
            modificar = 0;
            editando = 0;
        }
    }
}
//------------------ FIN FUNCIONES RTC - HORA ----------------------------------

//-------------------------- FUNCIONES I2C -------------------------------------

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

unsigned char write(unsigned char data) {
    set_wait();
    SSPBUF = data;
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
//----------------------- FIN FUNCIONES I2C ------------------------------------

void main(void) {
    TRISC0 = 0;
    TRISC1 = 0;
    RC0 = 0;
    RC1 = 0;
    iniUART_TX();
    
    TRISB = 0xFF; //I2C

    //--------------------------- RTC I2C --------------------------------------
    unsigned char i = 1;
    set_master();
    if (transmitir) {
        start(); // I2C Start Sequence
        write(0x40); // I2C Slave Device Address + Write
        write(i++); // The Data To Be Sent
        stop(); // I2C Stop Sequence
        __delay_ms(500);
        led_infoBYTE = ~led_infoBYTE; // Toggle LED Each Time A Byte Is Sent!
    }
     if (!transmitir) {
        start();        // I2C Start Sequence
        write(0x41);    // I2C Slave Device Address 0x40 + Read
        PORTD = read(); // Read Data From Slave
        stop();         // I2C Stop Sequence
        __delay_ms(100);
    }
    /*TRANSMITIR: "atar" su valor a una alarma de RTC para que varie cada 30s*/

    while (1) {
        uartTXEscritura('a');
    }
    return;
}