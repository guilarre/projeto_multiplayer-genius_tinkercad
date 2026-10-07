#include <LiquidCrystal.h>
#include <stdint.h> // CHECK:
#include <avr/pgmspace.h>

enum Color : uint8_t {
    VERDE = 0,
    VERMELHO = 1,
    AZUL = 2,
    AMARELO = 3
};

// protótipo evita falha do pré-processador do arduino
void exibirCorLcd(Color cor);

// pinos do lcd
const uint8_t PIN_LCD_RS = 2;
const uint8_t PIN_LCD_EN = 3;
const uint8_t PIN_LCD_D4 = A0;
const uint8_t PIN_LCD_D5 = A1;
const uint8_t PIN_LCD_D6 = A2;
const uint8_t PIN_LCD_D7 = A3;

// NOTE: inicialização do display
LiquidCrystal lcd(PIN_LCD_RS, PIN_LCD_EN, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);

// strings pras cores na memoria flash (PROGMEM)
const char VERDE_STR[] PROGMEM = "VERDE";
const char VERMELHO_STR[] PROGMEM = "VERMELHO";
const char AZUL_STR[] PROGMEM = "AZUL";
const char AMARELO_STR[] PROGMEM = "AMARELO";
// array de strings na flash
const char* const COLOR_TO_STRING[] PROGMEM = {
    VERDE_STR,
    VERMELHO_STR,
    AZUL_STR,
    AMARELO_STR
};

// NOTE: vars, funções, loop

// controla renderização pra ser apenas 1 vez
// e não ser afetada pelo loop()

// HACK: isso não faz nenhum sentido em outros contextos.
// neste teste, o loop está chamando exibirCorLcd sempre,
// executa apenas 1 vez e nunca mais desce pro resto do
// código
bool lcdPrecisaAtualizar = true;

void exibirCorLcd(Color cor) {
    if (cor > AMARELO) return;
    if (!lcdPrecisaAtualizar) return;

    const char* corString = (const char*)pgm_read_ptr(&(COLOR_TO_STRING[cor]));

    // limpa antes de imprimir (F() macro)
    lcd.setCursor(0, 1);
    lcd.print(F("                ")); //16 carac
    lcd.setCursor(0, 1);
    lcd.print((const __FlashStringHelper*)corString);

    lcdPrecisaAtualizar = false;
}

extern "C" void setup() {
    lcd.begin(16, 2);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Cor selecionada:"));
}

extern "C" void loop() {
    exibirCorLcd(VERMELHO);
}
