#include <xc.h>

#pragma config FOSC = INTRC_NOCLKOUT    
#pragma config WDTE = OFF               
#pragma config PWRTE = ON               
#pragma config MCLRE = ON              
#pragma config CP = OFF                
#pragma config CPD = OFF                
#pragma config BOREN = ON              
#pragma config IESO = ON              
#pragma config FCMEN = ON               
#pragma config LVP = OFF                
#pragma config BOR4V = BOR40V           
#pragma config WRT = OFF               

#include <pic16f887.h>
#define boundrate  15000 //velocidad de transmisión de datos
#define _XTAL_FREQ 4000000
#define led_infoBYTE RD3
#define transmitir 1 

#include <xc.h>
// ----------------------------- VARIABLES UART --------------------------------

#define num1 49
#define num2 50
#define num3 51

unsigned char datos = '0';
unsigned char mSR_r[8];
int contador = 0;
//--------------------------------- FIN VARIABLES UART -------------------------


//--------------PUERTOS ENTRADA PARA EDITAR LA HORA RELOJ-----------------------
int c_alarma;
int editando;
int m_hora = 0; //varaible de bt
int m_min = 0; //varaible de bt
int modificar = 0; //varaible de bt

#define alarma2 RC1
//led para avisar que se edita la hora
#define ledEditar RC0
//Puertos de salida de los display (4 pines con transistores)
#define disp1 RD1
#define disp2 RD2
#define disp3 RD3
#define disp4 RD4
//configuraciones que se enviarán al rtc
int rtc_config[] = {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
    0b00000000, 0b00000000, 0b10001111, 0b10000000, 0b10000000, 0b00000110, 0b00000000, 0b00000000, 0b00000000, 0b00000000};
//arreglo de numeros bcd para mostrar los numeros en el display
int numeros [] = {0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110, 0b01101101,
    0b01111100, 0b00000111, 0b01111111, 0b01100111};
//variables usadas para contar, llevar registro del tiempo, y para el control
int digito[4] = {0};

//variables de rtc_config();
#define horas 1
#define minutos 2

//------------------------FINALIZA VARIABLES RELOJ------------------------------

//----------------------------- VARIABLES I2C ----------------------------------

#define led_infoBYTE RD3
//#define transmitir modificar
#define boundrate 15000               //velocidad de transmisión de datos

//---------------------------- FIN VARIABLES I2C -------------------------------

//------------------------------- FUNCIONES I2C --------------------------------

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

void ACK() {
    set_wait();
    ACKDT = 0; // 0-> ACK y 1-> NACK
    ACKEN = 1; //mirar datasheet mańana
}

void NACK() {
    set_wait();
    ACKDT = 1; // 1 -> NACK, 0 -> ACK
    ACKEN = 1; // Send NACK Signal!
}

unsigned char write(unsigned char data) {
    set_wait();
    SSPBUF = data;
    set_wait();
    return ACKSTAT; // acknowledge status bit
}

unsigned char read(void) {
    RCEN = 1; //activa start y recepción.
    while (!SSPIF); //espera hasta compilación
    SSPIF = 0; // clear d einterrupt flag bit
    return SSPBUF; // return de received byte
}

//----------------------- FIN FUNCIONES I2C ------------------------------------

//------------------ FUNCIONES RTC - HORA --------------------------------------

char conversorInt_Char(int dato) {
    unsigned char comoChar = dato + '0';
    return comoChar;
}

int conversorChar_Int(unsigned char dato) {
    int comoEntero = (int) (dato);
    return comoEntero;
}

void configurar_rtc() {
    start(); // I2C Start Sequence
    write(0x40); // I2C Slave Device Address + Write
    for (int i = 0; i > 19; i++) {
        write(conversorInt_Char(rtc_config[i])); // The Data To Be Sent
        __delay_ms(500); //probar sacandolo no se por que esta
        led_infoBYTE = ~led_infoBYTE; // Toggle LED Each Time A Byte Is Sent!
        if (i == 19) {
            stop(); // I2C Stop Sequence
        } else {
            restart();
        }
    }
}

void seteo_horario() {
    //configurar minutos y horas que recibo para porde mandarlos al RTC

    int hh_decenas = rtc_config[horas] - (rtc_config[horas] / 10) * 10;
    int hh_unidades = rtc_config[horas] / 10;
    int mm_decenas = rtc_config[minutos] - (rtc_config[minutos] / 10) * 10;
    int mm_unidades = rtc_config[minutos] / 10;

    digito[0] = hh_decenas;
    digito[1] = hh_unidades;
    digito[2] = mm_decenas;
    digito[3] = mm_unidades;

    switch (hh_decenas) {
        case 0:
            rtc_config[minutos] = 0x00 + hh_unidades;
            break;
        case 1:
            rtc_config[horas] = (1 << 4) + hh_unidades;
            break;
        case 2:
            rtc_config[horas] = (2 << 4) + hh_unidades;

    }


}

void editar_reloj() {
    editando = 1;
    m_hora = 0;
    m_min = 0;
    __delay_ms(500);
    // mientras editando = 1 repetir lo sig
    while (editando) {
        //si se activa el modificador 'hora', incrementar la hora y relizar los
        //cálculos para modificar el reloj
        if (m_hora == 1) {
            m_hora = 0;
            digito[1] = digito[1] + 1;
            if (digito[1] > 9) {
                digito[1] = 0;
                digito[0] = digito[0] + 1;
            }
            if (digito[0] == 4 && digito[0] == 2) { // o sea si decenas está en 2 y unidades en 4
                digito[0] = 0;
                digito[1] = 0;
            }
            __delay_ms(100);
        }
        if (m_min == 1) {
            m_min = 0;
            digito[3] = digito[3] + 1;
            if (digito[3] > 9) {
                digito[3] = 0;
                digito[2] = digito[2] + 1;
                if (digito[2] > 5) {
                    digito[2] = 0;
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

void display_fix() {
    disp1 = 1;
    PORTD = numeros[digito[0]];
    __delay_ms(5);
    disp1 = 0;

    disp2 = 1;
    PORTD = numeros[digito[1]];
    __delay_ms(5);
    disp2 = 0;

    disp3 = 1;
    PORTD = numeros[digito[2]];
    __delay_ms(5);
    disp3 = 0;

    disp4 = 1;
    PORTD = numeros[digito[3]];
    __delay_ms(5);
    disp4 = 0;
}


//------------------ FIN FUNCIONES RTC - HORA ----------------------------------


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
    for (int i = 0; i < 8; i++) {
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
        unsigned char dato = RCREG;
        //MSR_r[] = new char[dato.lenght];

        switch (dato) {
            case'm':
                modificar = 1;
                mSR_r[contador] = dato;
                break;

            case 'f':
                //traducir
                seteo_horario();
                configurar_rtc();
                break;

            default:
                contador++;
                conversorChar_Int(dato);
                mSR_r[contador] = dato;
                break;
        }
        RCIF = 0;
    }
}
//------------------ FIN FUNCIONES BLUETOOTH - UART ----------------------------

void main(void) {
    iniUART_RX();
    set_master();
    TRISB = 0xFF; //I2C
    digito[0] = 0; //Dec de hora
    digito[1] = 0; //Uni de hora
    digito[2] = 0; //Dec de min
    digito[3] = 0; //Uni de min
    disp1 = 0;
    disp2 = 0;
    disp3 = 0;
    disp4 = 0;
    /*Salida 1 al puerto del led
        Nota: por la conexión de dicho LED, 0 ES ENCENDIDO, 1 ES APAGADO*/
    ledEditar = 1;
    /*Ciclo infinito donde se realizan las operaciones del reloj*/
    c_alarma = 0;
    //-------------------- TRANSMISIÓN Y RECEPCIÓN -----------------------------
    while (1) {
        display_fix();

        //reseteo para evitar defasaje a largo plazo
        if (c_alarma == 24) {
            c_alarma = 0;
            start(); // I2C Start Sequence
            write(0x41); // I2C Slave Device Address 0x40 + Read               
            PORTD = read(); // Read Data From Slave
            stop(); // I2C Stop Sequence
            __delay_ms(100);
        }

        if (modificar) {
            modificar = 0;
            ledEditar = 0; //prendido
            editar_reloj();
            ledEditar = 1; //apagado
            // cont = 50;
            __delay_ms(500);
        }
        if (!modificar) {
            //operaciones de reloj
            digito[3] = digito[3] + 1;
            if (digito[3] > 9) {
                digito[3] = 0;
                digito[2] = digito[2] + 1;
                if (digito[2] > 6) { //o sea si pasa los 60min
                    digito[2] = 0;
                    digito[1] = digito[1] + 1;
                    if (digito[1] > 4) {
                        digito[1] = 0;
                        digito[0] = digito[0] + 1;
                    }
                    if (digito[0] == 2 && digito[1] == 4) { //cuando llegue a las 24 hs (23:59)
                        digito[0] = 0;
                        digito[1] = 0;
                    }
                }
            }
        }
        //checkear si es necesario porque ya estamos amndando info en funcion ASCI
        uartTXEscritura(datos);
        while (!alarma2) {
            display_fix();
        }
        c_alarma++;
    }
    return;
}
