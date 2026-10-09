#include <LiquidCrystal.h>
#include <stdint.h> // CHECK:
#include <avr/pgmspace.h>

#define MAX_LEDS 4

enum Color : uint8_t {
    VERDE = 0,
    VERMELHO = 1,
    AZUL = 2,
    AMARELO = 3
};

enum AcaoBotao : uint8_t {
    ACAO_NENHUMA = 0,
    CLIQUE_SIMPLES,
    CLIQUE_DUPLO,
    CLIQUE_LONGO
};

// protótipos de nossas funções aqui pra evitar falha do
// pré-processador do arduino
AcaoBotao verificarBotaoMenu();
void menuSetup();
void exibirCorLcd(Color cor);
void printaString(const char* const* tabelaString, uint8_t idx, uint8_t screenY);

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
const char PROMPT_SETUP[] PROGMEM = "Escolha:";
const char OPCAO_1_SETUP[] PROGMEM = "1 jogador";
const char OPCAO_2_SETUP[] PROGMEM = "2 jogadores";
const char OPCAO_3_SETUP[] PROGMEM = "3 jogadores";
const char OPCAO_4_SETUP[] PROGMEM = "4 jogadores";
const char CONFIRMACAO_SETUP[] PROGMEM = "Confirmado:";

// array de strings na flash
const char* const MENU_SETUP_MSGS[] PROGMEM = {
    PROMPT_SETUP,
    OPCAO_1_SETUP,
    OPCAO_2_SETUP,
    OPCAO_3_SETUP,
    OPCAO_4_SETUP,
    CONFIRMACAO_SETUP
};

// NOTE: vars, funções, loop

// vars globais para mapeamento dos cliques do botão menu
uint8_t ultimoEstadoBotaoMenu = HIGH; //HIGH por conta do INPUT_PULLUP
unsigned long tempoUltimaMudanca = 0;
unsigned long tempoPressionado = 0;
unsigned long tempoLiberado = 0;
uint8_t contagemCliques = 0;
bool cliqueLongoProcessado = false;

// printa string no display LCD (x do cursor = 0)
void printaString(const char* const* tabelaString, uint8_t idx, uint8_t screenY) {
    const char* string = (const char*)pgm_read_ptr(&(tabelaString[idx]));

    //limpa
    lcd.setCursor(0, screenY);
    lcd.print("                ");
    //escreve
    lcd.setCursor(0, screenY);
    lcd.print((const __FlashStringHelper*)string);
}

AcaoBotao verificarBotaoMenu() {
    uint8_t leituraAtual = digitalRead(PINO_BOTAO_MENU); //INPUT_PULLUP
    unsigned long agora = millis();

    // pré-processando sinal
    if (leituraAtual != ultimoEstadoBotaoMenu) {
        delay(TEMPO_DEBOUNCE);
        leituraAtual = digitalRead(PINO_BOTAO_MENU);

        if (leituraAtual != ultimoEstadoBotaoMenu) {
            ultimoEstadoBotaoMenu = leituraAtual;

            if (leituraAtual == LOW) {
                tempoPressionado = agora;
                cliqueLongoProcessado = false;
            } else {
                if (!cliqueLongoProcessado) {
                    contagemCliques++;
                    tempoLiberado = agora;
                }
            }
        }
    }

    // checagem de clique longo (botão mantido em LOW)
    if (leituraAtual == LOW && !cliqueLongoProcessado) {
        if (agora - tempoPressionado >= TEMPO_CLIQUE_LONGO) {
            cliqueLongoProcessado = true;
            contagemCliques = 0;
            return CLIQUE_LONGO;
        }
    }

    // checagem de clique simples ou duplo após liberação (HIGH)
    if (contagemCliques > 0 && leituraAtual == HIGH) {
        if (contagemCliques >= 2) {
            contagemCliques = 0;
            return CLIQUE_DUPLO;
        }
        if (agora - tempoLiberado >= TEMPO_CLIQUE_DUPLO) {
            contagemCliques = 0;
            return CLIQUE_SIMPLES;
        }
    }

    return ACAO_NENHUMA;
}

void menuSetup() {
    uint8_t opcaoSelecionada = 1; // padrão: 1 jogador (índice 1 no MENU_SETUP_MSGS)

    ultimoEstadoBotaoMenu = digitalRead(PINO_BOTAO_MENU);
    contagemCliques = 0;
    cliqueLongoProcessado = false;

    lcd.clear();
    printaString(MENU_SETUP_MSGS, 0, 0); // Linha 0: Prompt
    printaString(MENU_SETUP_MSGS, opcaoSelecionada, 1); // Linha 1: Opção

    // loop bloqueante até a confirmação da seleção
    while (true) {
        AcaoBotao acao = verificarBotaoMenu();

        if (acao == CLIQUE_SIMPLES) {
            // avança para a próxima opção (rotaciona de 1 a 4)
            opcaoSelecionada = (opcaoSelecionada % 4) + 1;
            printaString(MENU_SETUP_MSGS, opcaoSelecionada, 1);
        }
        else if (acao == CLIQUE_DUPLO || acao == CLIQUE_LONGO) {
            // confirmação da opção
            lcd.clear();
            printaString(MENU_SETUP_MSGS, 5, 0);
            printaString(MENU_SETUP_MSGS, opcaoSelecionada, 1);
            delay(1500);
            break;
        }
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
    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("--fim do teste--");
}
