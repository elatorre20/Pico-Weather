#define METER_PIN 21

void setup(){
    Serial.begin(9800);
    pinMode(METER_PIN, OUTPUT);
    analogWriteResolution(16);
}

void loop(){
    for(uint16_t i = 0; i < 65535; i++){
        Serial.println(i);
        analogWrite(METER_PIN, i);
        delayMicroseconds(500);
    }
}