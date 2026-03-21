//#include <SPI.h>
#include <ESP32-TWAI-CAN.hpp>

const int rpmPin = 10; 
const int timeoutValue = 10; 
volatile unsigned long lastPulseTime; 
volatile unsigned long interval = 0; 
volatile int timeoutCounter; 
long rpm;                                             
long rpm_last;                                               
//int activation_rpm = 1000; // Установите значение активации RPM по умолчанию
//int shift_rpm = 6000; // Установите значение сдвига RPM по умолчанию
int smoothing = 1; // Включите сглаживание
const int numReadings = 1;
int rpmarray[numReadings];
int rpmIndex = 0;                  
long total = 0;                  
long average = 0;                
//float cal = 2.0; // Объявляем переменную cal

#define CAN_TX 3 // Connects to CTX
#define CAN_RX 2 // Connects to CRX

void setup() {
    // ROTARY ENCODER 
    pinMode(rpmPin, INPUT);
    attachInterrupt(digitalPinToInterrupt(rpmPin), sensorIsr, RISING); 

    ESP32Can.setPins(CAN_TX, CAN_RX);
    Serial.begin(115200); // Инициализация последовательного порта
    Serial.println("CAN bus initialized");

    // Start the CAN bus at 500 kbps
    if (ESP32Can.begin(ESP32Can.convertSpeed(500))) {
        Serial.println("CAN bus started!");
    } else {
        Serial.println("CAN bus failed!");
    }
}

void loop() { 
    rpm = long(60e7 / 2 ) / (float)interval; // Используем переменную cal вместо 2

    if (timeoutCounter > 0) { timeoutCounter--; }  
    if (timeoutCounter <= 0) { rpm = 0; }

    if (((rpm > (rpm_last + (rpm_last * 0.2))) || (rpm < (rpm_last - (rpm_last * 0.2)))) && (rpm_last > 0)) {
        rpm = rpm_last; // Если новое значение RPM слишком отличается, используйте предыдущее
    } else {
        rpm_last = rpm;
    }

    if (smoothing) {
        total = total - rpmarray[rpmIndex]; 
        rpmarray[rpmIndex] = rpm;
        total = total + rpmarray[rpmIndex]; 
        rpmIndex = (rpmIndex + 1) % numReadings; // Использование остатка от деления для сброса индекса
        average = total / numReadings; 
        rpm = average;       
    }

    // Уменьшаем значение RPM в 10 раз
    rpm /= 10;

    // Отправка RPM через CAN
    canSender(rpm); // Исправлено на использование переменной rpm
    delay(30); // Задержка для обновления
}

void canSender(float rpm) {
    // Преобразуем RPM в два байта
    uint16_t combined = (uint16_t)((rpm - 40) / 40);
    Serial.print("Sending RPM: ");
    Serial.println(rpm);
    Serial.print("Combined value: 0x");
    Serial.println(combined, HEX);

    // Отправляем 8-байтный пакет
    CanFrame testFrame = { 0 };
    testFrame.identifier = 0x316; // Sets the ID for the packet
    testFrame.extd = 0; // Set extended frame to false
    testFrame.data_length_code = 8; // Устанавливаем длину данных на 8 байт

    // Заполняем пакет
    testFrame.data[0] = 0x00; // Byte 0
    testFrame.data[1] = 0x00; // Byte 1
    testFrame.data[2] = (combined >> 8) & 0xFF; // Старший байт (Byte 2)
    testFrame.data[3] = combined & 0xFF; // Младший байт (Byte 3)
    testFrame.data[4] = 0x00; // Byte 4
    testFrame.data[5] = 0x00; // Byte 5
    testFrame.data[6] = 0x00; // Byte 6
    testFrame.data[7] = 0x00; // Byte 7

    ESP32Can.writeFrame(testFrame); // Передаем пакет
    Serial.println("Packet sent");
}

void sensorIsr() { 
    unsigned long now = micros(); 
    interval = now - lastPulseTime; 
    lastPulseTime = now; 
    timeoutCounter = timeoutValue; 
}
