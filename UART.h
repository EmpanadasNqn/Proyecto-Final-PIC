#include <xc.h>
#include <pic16f887.h>

void UartInicializacionTX() {
    
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

void UartInicializacionRX() {
    
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

void UartTXEscrituraStrings(unsigned char datos[]) {        //En el caso de que lo que se quiera transmitir sea un String
    
    TXEN = 1;
    for(int i = 0; i < sizeof(datos); i++) {
        TXREG = datos[i];
        while(!TRMT) {}
    }
    TXEN = 0;
}

void UartTXEscritura(unsigned char datos) {                 //En el caso de que lo que se quiera transmitir sea una char

    TXEN = 1;
    while(!TRMT) {
        TXREG = datos;
    }
    TXEN = 0;
}