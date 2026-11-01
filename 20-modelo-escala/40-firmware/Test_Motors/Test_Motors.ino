#include <Arduino.h>

// Pines según mapa-gpio.md
const uint8_t R_PWM_PINS[4] = {25, 27, 13, 5};
const uint8_t L_PWM_PINS[4] = {26, 14, 4, 23};

const uint32_t PWM_FREQ = 5000; // 5 kHz
const uint8_t PWM_RES = 8;      // 8 bits (0 a 255)
const uint32_t TEST_DUTY = 128; // 50% de potencia

void configurarMotor(uint8_t index) {
    #if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
        ledcAttach(R_PWM_PINS[index], PWM_FREQ, PWM_RES);
        ledcAttach(L_PWM_PINS[index], PWM_FREQ, PWM_RES);
    #else
        uint8_t chR = index * 2;
        uint8_t chL = index * 2 + 1;
        ledcSetup(chR, PWM_FREQ, PWM_RES);
        ledcSetup(chL, PWM_FREQ, PWM_RES);
        ledcAttachPin(R_PWM_PINS[index], chR);
        ledcAttachPin(L_PWM_PINS[index], chL);
    #endif
}

// Función que aplica el mismo Duty Cycle a los 4 canales simultáneamente
void moverTodos(uint32_t dutyR, uint32_t dutyL) {
    for (int i = 0; i < 4; i++) {
        #if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
            ledcWrite(R_PWM_PINS[i], dutyR);
            ledcWrite(L_PWM_PINS[i], dutyL);
        #else
            ledcWrite(i * 2, dutyR);
            ledcWrite(i * 2 + 1, dutyL);
        #endif
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n==========================================");
    Serial.println(" TEST SIMULTÁNEO DE MOTORES (BTS7960) ");
    Serial.println("==========================================");

    // Inicializar todos los canales
    for (int i = 0; i < 4; i++) {
        configurarMotor(i);
    }
    
    // Asegurar que arranquen frenados
    moverTodos(0, 0); 
    
    Serial.println("Todos los canales en paralelo a 5 kHz. Iniciando en 3s...");
    delay(3000);
}

void loop() {
    Serial.println(">>> [->] Sentido 1 (R_PWM 50%, L_PWM 0%) en TODOS los puertos");
    moverTodos(TEST_DUTY, 0);
    delay(3000); // 3 segundos para observar/conectar

    Serial.println(">>> [||] Freno");
    moverTodos(0, 0);
    delay(2000); // 2 segundos de pausa para cambiar el motor de puerto seguro

    Serial.println(">>> [<-] Sentido 2 (R_PWM 0%, L_PWM 50%) en TODOS los puertos");
    moverTodos(0, TEST_DUTY);
    delay(3000);

    Serial.println(">>> [||] Freno");
    moverTodos(0, 0);
    delay(2000);
    
    Serial.println("------------------------------------------");
} 
