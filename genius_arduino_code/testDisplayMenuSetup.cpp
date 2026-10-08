#include <LiquidCrystal.h>
#include <cstdint>
#include <stdint.h> // CHECK:
#include <avr/pgmspace.h>

enum Color : uint8_t {
    VERDE = 0,
    VERMELHO = 1,
    AZUL = 2,
    AMARELO = 3
};

// protótipos de nossas funções aqui pra evitar falha do
// pré-processador do arduino
void exibirCorLcd(Color cor);
void printa_string(const char* const tabelaString);

// pinos do lcd
const uint8_t PIN_LCD_RS = 2;
const uint8_t PIN_LCD_EN = 3;
const uint8_t PIN_LCD_D4 = A0;
const uint8_t PIN_LCD_D5 = A1;
const uint8_t PIN_LCD_D6 = A2;
const uint8_t PIN_LCD_D7 = A3;
// pinos leds e botões
const uint8_t PINOS_LEDS[MAX_LEDS] = {13, 12, 11, 10};
const uint8_t PINOS_BOTOES[MAX_LEDS] = {9, 8, 7, 6};
const uint8_t PINO_BOTAO_MENU = 5;
const uint8_t PINO_BOTAO_ON_OFF = 4;

// constantes de tempo
const uint16_t TEMPO_DEBOUNCE = 50; //50ms pra como filtro pra ruidos
const uint16_t TEMPO_CLIQUE_DUPLO = 300; // CHECK: ver se vai usar msm
const uint16_t TEMPO_CLIQUE_LONGO = 1000;

// inicialização do display
LiquidCrystal lcd(PIN_LCD_RS, PIN_LCD_EN, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);

// NOTE: strings na memoria flash (PROGMEM)
// strings pro menuSetup
const char PROMPT_SETUP[] PROGMEM = "Escolha a qtd de jogadores...";
const char OPCAO_SETUP_1[] PROGMEM = "1 jogadores";
const char OPCAO_SETUP_2[] PROGMEM = "2 jogadores";
const char OPCAO_SETUP_3[] PROGMEM = "3 jogadores";
const char OPCAO_SETUP_4[] PROGMEM = "4 jogadores";

// array de strings na flash
const char* const MENU_SETUP_MSGS[] PROGMEM = {
    PROMPT_SETUP,
    OPCAO_SETUP_1,
    OPCAO_SETUP_2,
    OPCAO_SETUP_3,
    OPCAO_SETUP_4
};

// NOTE: vars, funções, loop

// vars globais para mapeamento dos cliques do botão menu
uint8_t ultimoEstadoBotao = HIGH; //HIGH por conta do INPUT_PULLUP
unsigned long tempoUltimaMudanca = 0;
unsigned long tempoPressionado = 0;
unsigned long tempoLiberado = 0;
uint8_t contagemCliques = 0;
bool cliqueLongoProcessado = false;

// printa string no display LCD (x do cursor = 0)
void printaString(const char* const tabelaString, uint8_t idx, uint8_t screenY) {
    const char* string = (const char*)pgm_read_ptr(&(tabelaString[idx]));

    lcd.clear();
    lcd.setCursor(0, screenY);
    lcd.print((const __FlashStringHelper*)string);
}

void menuSetup() {
    uint8_t estadoBotaoMenu = digitalRead(PINO_BOTAO_MENU);
    printaString(MENU_SETUP_MSGS, 0);

    // TODO: parei aqui
    if (estadoBotaoMenu == 0) {
        
    }
}

extern "C" void setup() {
    // inicializando botões
    for (int i = 0; i < MAX_LEDS; i++) {
        pinMode(PINOS_LEDS[i], OUTPUT);
        pinMode(PINOS_BOTOES[i], INPUT);
    }
    // TEST: ver se INPUT_PULLUP dá certo atualmente no circuito
    pinMode(PINO_BOTAO_MENU, INPUT_PULLUP); //pullup = sempre HIGH, ativa qnd LOW
    pinMode(PINO_BOTAO_ON_OFF, INPUT_PULLUP); 

    // inicializando display
    lcd.begin(16, 2);
    lcd.clear();
    lcd.setCursor(0, 0);

    menuSetup();
}

extern "C" void loop() {
    Color corPraMostrar = VERDE;
    exibirCorLcd(corPraMostrar); // HACK: continua sendo chamado pra sempre
}
