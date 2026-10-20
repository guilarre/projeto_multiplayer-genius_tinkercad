#define MAX_LEDS 4

const uint8_t PINOS_LEDS[MAX_LEDS] = {13, 12, 11, 10};
const uint8_t PINOS_BOTOES[MAX_LEDS] = {9, 8, 7, 6};

void setup() {
    for (uint8_t i = 0; i < MAX_LEDS; i++) {
        pinMode(PINOS_LEDS[i], OUTPUT);
        pinMode(PINOS_BOTOES[i], INPUT_PULLUP);
    }
}

void loop() {
    for (uint8_t i = 0; i < MAX_LEDS; i++) {
        if (digitalRead(PINOS_BOTOES[i]) == LOW) {
            digitalWrite(PINOS_LEDS[i], HIGH);
            delay(500);
            digitalWrite(PINOS_LEDS[i], LOW);
            while (digitalRead(PINOS_BOTOES[i]) == LOW) {
                delay(10);
            }
        }
    }
}
