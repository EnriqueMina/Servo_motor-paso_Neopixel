#include "mbed.h"
#include "TM1638.h"
#include "NeoPixel.h"

// Pines
#define STB D2
#define CLK D3
#define DIO D4
#define POT A0
#define NUM_LEDS 12

TM1638 tm(STB, CLK, DIO);
AnalogIn pot(POT);
NeoPixel leds(D11, NUM_LEDS);    
PwmOut pwm_servo(D10);

// Motor paso a paso
DigitalOut step1(D6);
DigitalOut step2(D7);
DigitalOut step3(D8);
DigitalOut step4(D9);

// Secuencia del motor paso a paso
const int step_seq[8][4] = {
    {1,0,0,1},{1,0,0,0},{1,1,0,0},{0,1,0,0},
    {0,1,1,0},{0,0,1,0},{0,0,1,1},{0,0,0,1}
};

// Variable para evitar rebotes
uint8_t ultimo_boton = 0; //guarda el ultimo estado para evitr el antirebote 

// Leer botones del TM1638
uint8_t leerBoton(){
    uint8_t keys = tm.readKeys();
    uint8_t btn = 0;

    if(keys & 0x01) btn = 1;   // S1
    else if(keys & 0x02) btn = 2; // S2
    else if(keys & 0x04) btn = 3; // S3
    else if(keys & 0x08) btn = 4; // S4
    else if(keys & 0x10) btn = 5; // S5
    else if(keys & 0x20) btn = 6; // S6
    else if(keys & 0x40) btn = 7; // S7
    else if(keys & 0x80) btn = 8; // S8


//antirebote, guarda el utlimo estado del boton y espera 200ms 
    if(btn != 0 && btn != ultimo_boton){
        ultimo_boton = btn;
        ThisThread::sleep_for(200ms);
        return btn;
    }
    if(btn == 0) ultimo_boton = 0;
    return 0;
}
// Mostrar en display + consola
void mostrarDisplay(const char* texto, const char* etiqueta = ""){
    char buf[9]; // 8 dígitos + '\0'
    strncpy(buf, texto, 8);
    buf[8] = '\0';
    tm.show(buf);
    if(strlen(etiqueta) > 0){
        printf("%s: %s\n", etiqueta, buf);
    }
}
// Mostrar fecha
void mostrarFecha(){
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char buf[9];
    snprintf(buf, 9, "%02d%02d%04d", tm_info->tm_mday, tm_info->tm_mon+1, tm_info->tm_year+1900);
    mostrarDisplay(buf, "FECHA");
    ThisThread::sleep_for(2s);
}
// Mostrar hora
void mostrarHora(){
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char buf[9];
    snprintf(buf, 9, "%02d%02d%02d", tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
    mostrarDisplay(buf, "HORA");
    ThisThread::sleep_for(2s);
}

// llama ambas funciones
void mostrarFechaYHora(){
    mostrarFecha();
    mostrarHora();
}


void girarMotor(int pasos, int direccion){
    for(int i = 0; i < pasos; i++){
        for(int j = 0; j < 8; j++){
            int idx = (direccion == 1) ? j : 7-j;
            step1 = step_seq[idx][0];
            step2 = step_seq[idx][1];
            step3 = step_seq[idx][2];
            step4 = step_seq[idx][3];
            ThisThread::sleep_for(20ms);
        }
    }
    step1 = step2 = step3 = step4 = 0;
}

// Mover servo
void moverServo(int angulo){
    if(angulo < 0) angulo = 0;
    if(angulo > 180) angulo = 180;
    int pulso_us = 1000 + (angulo * 1000) / 180;
    pwm_servo.pulsewidth_us(pulso_us);
    ThisThread::sleep_for(500ms);
}

// Vúmetro con potenciómetro y NeoPixel
void vumetro(){
    printf("\nMenu Vumetro\n");
    printf("S1: Volver al menu\n");
    printf("S8: Salir\n");

    leds.clear();
    leds.show();
    ultimo_boton = 0;

    // Calibración de tu potenciómetro
    float min_real = 0.0f;
    float max_real = 0.75f;

    while(true){
        float valor = pot.read();

        // Normalizar lectura
        valor = (valor - min_real) / (max_real - min_real);
        if(valor < 0.0f) valor = 0.0f;
        if(valor > 1.0f) valor = 1.0f;

        // Calcular LEDs encendidos
        int leds_encendidos = (int)round(valor * NUM_LEDS);
        if(leds_encendidos > NUM_LEDS) leds_encendidos = NUM_LEDS;

        // Apagar todos
        leds.clear();

        // Encender LEDs 
        for(int i = 0; i < leds_encendidos; i++){
            if(i < 4) leds.setPixelColor(i, 0, 50, 0);       // Verde
            else if(i < 8) leds.setPixelColor(i, 50, 50, 0); // Amarillo
            else leds.setPixelColor(i, 50, 0, 0);            // Rojo
        }
        leds.show();

        // Mostrar en display
        char buf[9];
        snprintf(buf, 9, "U%03d", (int)(valor * 100));
        tm.show(buf);

        // Lee losBotones
        uint8_t btn = leerBoton();
        if(btn == 1){  // S1 - Volver
            leds.clear();
            leds.show();
            return;
        } else if(btn == 8){  // S8 - Salir
            mostrarDisplay("BYE");
            printf("Saliendo.\n");
            leds.clear();
            leds.show();
            while(true) ThisThread::sleep_for(1s);
        }
        ThisThread::sleep_for(100ms);
    }
}

void menuMotor(){
    printf("\nMenu Motor Paso a Paso\n");
    printf("Seleccione cantidad de pasos:\nS1=50, S2=100, S3=200\n");

    ultimo_boton = 0;
    int pasos = 0;

    while(pasos == 0){
        uint8_t btn = leerBoton();
        if(btn == 1) pasos = 50;
        else if(btn == 2) pasos = 100;
        else if(btn == 3) pasos = 200;
        ThisThread::sleep_for(50ms);
    }

    char buf[9];
    snprintf(buf, 9, "%d", pasos);
    printf("Pasos seleccionados: %d\n", pasos);
    printf("Seleccione direccion:\nS4=Horario, S5=Antihorario\n");

    ultimo_boton = 0;
    int direccion = -1;
    const char* sentido = "";

    while(direccion == -1){
        uint8_t btn = leerBoton();
        if(btn == 4){ direccion = 1; sentido="Horario"; }
        else if(btn == 5){ direccion = 0; sentido="Antihorario"; }
        ThisThread::sleep_for(50ms);
    }

    printf("Pasos: %d, Sentido: %s\n", pasos, sentido);
    snprintf(buf,9,"P%d",pasos);
    tm.show(buf);

    girarMotor(pasos,direccion);
    printf("Motor ejecutado\n");
    ThisThread::sleep_for(1s);
}

void menuServo(){
    printf("\nMenu Servomotor\n");
    printf("Seleccione angulo:\nS1=45, S2=90, S3=135\n");

    ultimo_boton = 0;
    int angulo = -1;

    while(angulo == -1){
        uint8_t btn = leerBoton();
        if(btn == 1) angulo = 45;
        else if(btn == 2) angulo = 90;
        else if(btn == 3) angulo = 135;
        ThisThread::sleep_for(50ms);
    }

    printf("Angulo seleccionado: %d\n", angulo);
    char buf[9];
    snprintf(buf,9,"S%03d",angulo);
    mostrarDisplay(buf);

    moverServo(angulo);
    printf("Servo ejecutado\n");
    ThisThread::sleep_for(1s);
}
void menuTM1638(){
    while(true){
        printf("\n MENU TM1638 \n");
        printf("S1: Motor Paso a Paso\n");
        printf("S2: Servomotor\n");
        printf("S3: Mostrar Fecha y Hora\n");
        printf("S4: Vumetro NeoPixel\n");
        printf("S8: Salir\n");

        ultimo_boton = 0;

        while(true){
            uint8_t btn = leerBoton();
            if(btn == 1){ menuMotor(); break; }
            else if(btn == 2){ menuServo(); break; }
            else if(btn == 3){ mostrarFechaYHora(); break; }
            else if(btn == 4){ vumetro(); break; }
            else if(btn == 8){
                printf("Saliendo.\n");
                leds.clear();
                leds.show();
                while(true) ThisThread::sleep_for(1s);
            }
            ThisThread::sleep_for(50ms);
        }
    }
}

// MAIN
int main(){
    tm.clear();
    tm.setBrightness(5);
    pwm_servo.period_ms(20);
    leds.clear();
    leds.show();

    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    printf("  SISTEMA DE CONTROL INICIALIZADO  \n");
    printf("Hora de arranque: %02d/%02d/%04d %02d:%02d:%02d\n",
           tm_info->tm_mday, tm_info->tm_mon+1, tm_info->tm_year+1900,
           tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);

    mostrarFechaYHora();
    menuTM1638();

    while(true) ThisThread::sleep_for(1s);
}
