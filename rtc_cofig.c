

//--------------PUERTOS ENTRADA PARA EDITAR LA HORA RELOJ-----------------------
#define hora 0 //varaible de bt
#define min  0//varaible de bt
#define modificar 0//varaible de bt
//led para avisar que se edita la hora
#define ledEditar PORTC0
#define digito1 PORTB
//num bcd para representar numeros en display
int numeros [] = {0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110, 0b01101101,
    0b01111100, 0b00000111, 0b01111111, 0b01100111};
//variables usadas para contar, llevar registro del tiempo, y para el control
int digito1;
int digito2;
int digito3;
int digito4;
//editando boolean
int editando;
//---------------------FINALIZA VARIABLES RELOJ---------------------------------

//----------------------DETALLES DE EDICIÓN RELOJ-------------------------------

/*Cófig que maneja detalles de la edición del reloj cuando e presiona
 el botón modificar (cuando desde bt se envía un activador de modificar)
 Nota: reloj de 24hs*/
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
                /*En algun lugar tengo que hacer que digito se pase a BCD*/
            }
            if (digito1 == 3 && digito1 == 1) { // o sea si decenas está en 2 y unidades en 0
                digito1 = 0;
                digito2 = 1;
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
//-----------------------FINALIZA EDITAR RELOJ----------------------------------

void main() {
    //reloj se inicializa a las 00 al encenderse
    digito1 = 0; //Dec de hora
    digito2 = 0; //Uni de hora
    digito3 = 0; //Dec de min
    digito4 = 0; //Uni de min
    hora = 0;
    min = 0;
    int cont = 0;
    /*Salida 1 al puerto del led
        Nota: por la conexión de dicho LED, 0 ES ENCENDIDO, 1 ES APAGADO*/
    ledEditar = 1;
    /*Ciclo infinito donde se realizan las operaciones del reloj*/
    while (1) {
        if (modificar == 1) {
            modificar = 0;
            ledEditar = 0; //prendido
            editar_reloj();
            ledEditar = 1; //apagado
            cont = 50;
            __delay_ms(500);
        }
        if (cont = 50) {
            cont = 0;
            /*operaciones del reloj, que se encargan de saber qué digitos
            incremenas ccuando otros exceden el límite. nota: es un reloj de 24hs*/
            digito4 = digito4 + 1;
            if (digito4 > 9) {
                digito4 = 0;
                digito3 = digito3 + 1;
                if (digito3 > 5) { //o sea si pasa los 60min
                    digito3 = 0;
                    digito2 = digito2 + 1;
                    if (digito2 > 9) {
                        digito2 = 0;
                        digito1 = digito1 + 1;
                    }
                    if (digito2 = 4 && digito1 = 2) { //cuando llegue a las 24 hs (23:59)
                        digito1 = 0;
                        digito2 = 1;
                    }
                }
            }
        }
    }
}
