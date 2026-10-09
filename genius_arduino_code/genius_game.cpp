// BACKLOG: - selecionar num de jogadores através dos botões de jogadores??
//
// TODO:
// - ligar leds no momento de selecionar num de jogadores no menuSetup()
// pra ter indicação visual de onde fica o botão de cada jogador
//
// TODO: checar quantos segundos leva pra mostrar sequencia atual e se deve diminuir ao longo do jogo
//
// TODO: pensar como será sistema de pontuação
//
// TODO: pensar no feedback ao user -> luz, som, ...?
// som qnd apertar botao, qnd acertar, errar..
//
// TODO: persistência na memória pra guardar highscore
//
// TODO: pesquisar em como deixar a mensagem persistir na tela se chamada dentro de loop()
//
// TODO: FALTA LOGICA DO ON/OFF
//
// FIX: os turnos dagt devem incrementar conforme cada player joga
//
// TEST: talvez deixar sequencia até 20..?
//
// FIX: checar usos de char*

#include <cstdint>
#include <stdint.h> // REMOVE:
#include <stdlib.h>
#include <sys/types.h> // CHECK:
#include <LiquidCrystal.h>
#include <avr/pgmspace.h>

#define MAX_LEDS 4
#define MAX_PLAYERS 4
#define MAX_SEQUENCIA 15
#define PONTO_POR_JOGADA 100
#define STRING_BUFFER_SIZE 100

// NOTE: lógica da sequência de cores
// uint8_t (1 byte), em vez de int (2 ou 4 bytes)
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

// protótipos de nossas funções aqui pra evitar falha do pré-processador do arduino
AcaoBotao verificarBotaoMenu();
void menuSetup();
void exibirCorLcd(Color cor);
void printaString(const char* const* tabelaString, uint8_t idx, uint8_t screenY);

// NOTE: const globais
// pinos leds e botões
const uint8_t PINOS_LEDS[MAX_LEDS] = {13, 12, 11, 10};
const uint8_t PINOS_BOTOES[MAX_LEDS] = {9, 8, 7, 6};
const uint8_t PINO_BOTAO_MENU = 5;
// TODO: FALTA LOGICA DO ON/OFF
const uint8_t PINO_BOTAO_ON_OFF = 4;
// constantes de tempo p clique
const uint16_t TEMPO_DEBOUNCE = 50; //50ms pra como filtro pra ruidos
const uint16_t TEMPO_CLIQUE_DUPLO = 300; // CHECK: ver se vai usar msm
const uint16_t TEMPO_CLIQUE_LONGO = 1000;
// pinos do lcd
const uint8_t PIN_LCD_RS = 2;
const uint8_t PIN_LCD_EN = 3;
const uint8_t PIN_LCD_D4 = A0;
const uint8_t PIN_LCD_D5 = A1;
const uint8_t PIN_LCD_D6 = A2;
const uint8_t PIN_LCD_D7 = A3;

// NOTE: inicialização do display
LiquidCrystal lcd(PIN_LCD_RS, PIN_LCD_EN, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);

// NOTE: strings na memoria flash (PROGMEM)
// cores: strings
const char VERDE_STR[] PROGMEM = "VERDE";
const char VERMELHO_STR[] PROGMEM = "VERMELHO";
const char AZUL_STR[] PROGMEM = "AZUL";
const char AMARELO_STR[] PROGMEM = "AMARELO";
// cores: array de strings
const char* const COLOR_TO_STRING[] PROGMEM = {
    VERDE_STR,
    VERMELHO_STR,
    AZUL_STR,
    AMARELO_STR
};
// menuSetup: strings
const char PROMPT_SETUP[] PROGMEM = "--  Escolha  --";
const char OPCAO_1_SETUP[] PROGMEM = "1 jogador";
const char OPCAO_2_SETUP[] PROGMEM = "2 jogadores";
const char OPCAO_3_SETUP[] PROGMEM = "3 jogadores";
const char OPCAO_4_SETUP[] PROGMEM = "4 jogadores";
const char CONFIRMACAO_SETUP[] PROGMEM = "-- Confirmado --";
// menuSetup: array de strings
const char* const MENU_SETUP_MSGS[] PROGMEM = {
    PROMPT_SETUP,
    OPCAO_1_SETUP,
    OPCAO_2_SETUP,
    OPCAO_3_SETUP,
    OPCAO_4_SETUP,
    CONFIRMACAO_SETUP
};
// menuPausa: strings
const char PROMPT_PAUSA[] PROGMEM = "--   PAUSA   --"; // TODO: loop de pausa
const char OPCAO_1_PAUSA[] PROGMEM = "Voltar"; // TODO: sai do loop
const char OPCAO_2_PAUSA[] PROGMEM = "Pontuação";
const char OPCAO_3_PAUSA[] PROGMEM = "Reiniciar jogo"; // TODO:
const char OPCAO_4_PAUSA[] PROGMEM = "Terminar jogo"; // TODO:
// menuPausa: array de strings
const char* const MENU_PAUSA_MSGS[] PROGMEM = {
    PROMPT_PAUSA,
    OPCAO_1_PAUSA,
    OPCAO_2_PAUSA,
    OPCAO_3_PAUSA,
    OPCAO_4_PAUSA
};
// TODO: lógica pra mostrar de quem é a vez no display
// player turn: strings
const char VEZ_P1[] PROGMEM = "-- Player 1 --";
const char VEZ_P2[] PROGMEM = "-- Player 2 --";
const char VEZ_P3[] PROGMEM = "-- Player 3 --";
const char VEZ_P4[] PROGMEM = "-- Player 4 --";
// player turn: array de strings
const char* const VEZ_ATUAL_MSGS[] PROGMEM = {
    VEZ_P1,
    VEZ_P2,
    VEZ_P3,
    VEZ_P4
};
// mostrarPontuacaoOrdenada: strings
const char TITULO[] PROGMEM = "--  Ranking  --";
const char P1_MSG[] PROGMEM = "P1: ";
const char P2_MSG[] PROGMEM = "P2: ";
const char P3_MSG[] PROGMEM = "P3: ";
const char P4_MSG[] PROGMEM = "P4: ";
// mostrarPontuacaoOrdenada: array de strings
const char* const PONTUACAO_MSGS[] PROGMEM = {
    TITULO,
    P1_MSG,
    P2_MSG,
    P3_MSG,
    P4_MSG
};
// ...
const char CONFIRMACAO_RESET[] PROGMEM = "Reiniciando...";
// TODO: confirmação de que terminou o jogo
// const char CONFIRMACAO_TERMINOU[] = "Desligando...";
const char* const MSGS_SISTEMA[] PROGMEM {
    CONFIRMACAO_RESET
};

// NOTE: fila pra sequência atual de cores (FIFO)
struct filaCor {
    Color sequenciaCor[MAX_SEQUENCIA]; //array de cores
    uint8_t inicio = 0, final = 0, qtd = 0;

    bool enqueue(Color cor) {
        if (qtd == MAX_SEQUENCIA) {
            return false;
        }
        sequenciaCor[final] = cor;
        final = (final + 1) % MAX_SEQUENCIA; //circular
        qtd++;
        return true;
    }

    bool dequeue(Color &corRemovida) {
        if (qtd == 0) {
            return false;
        }
        corRemovida = sequenciaCor[inicio];
        inicio = (inicio + 1) % MAX_SEQUENCIA; //circular
        qtd--;
        return true;
}
};

// NOTE: classe player
class Player {
    private:
        // variável estática pra contagem de objetos player
        static uint8_t countId; //num do player: 1, 2, 3 ou 4

    public:
        // atributos
        uint8_t id;
        uint16_t score;
        bool ativo;

        // construtor
        Player() {
            id = ++countId;
            score = 0;
            ativo = false; //inicia como falso
        }

        void reset() {
            score = 0; //reseta score
            ativo = false; //desativa player
        }

        void ativar() {
            ativo = true; //ativa player
        }

        void desativar() {
            ativo = false; //desativa player
        }

};

// inicializando fora da função (obrigatório em C++)
uint8_t Player::contadorId = 0;

// NOTE: vars globais e funções do jogo

// NOTE: vars globais para mapeamento dos cliques do botão menu
uint8_t ultimoEstadoBotaoMenu = HIGH; //HIGH por conta do INPUT_PULLUP
unsigned long tempoUltimaMudanca = 0;
unsigned long tempoPressionado = 0;
unsigned long tempoLiberado = 0;
uint8_t contagemCliques = 0;
bool cliqueLongoProcessado = false;

// NOTE: vars e funções filaCor
filaCor sequenciaAtualGlobal; //fila global
// TEST: variável global da jogada atual (vai ser resetada a cada tentativa correta)
filaCor tentativaAtual;
// TEST: função pra inserir uma cor aleatória na var global
void newRandomColor() {
    uint8_t randNum = random(0, 4); //num aleatório de 0 a 3
    // insere na var global (damos static_cast de uint8_t -> Color):
    sequenciaAtualGlobal.enqueue(static_cast<Color>(randNum));
}

// NOTE: vars e funções de player
// cria os 4 players na memória
Player playerList[MAX_PLAYERS];
// ponteiro pro player atual
uint8_t playerAtual = 0;
// qtd jogadores atual // CHECK:checar se qnd muda tá alterando
uint8_t totalPlayers = 1;
// função pra iterar até o próx player válido
void proxPlayer() {
    uint8_t proxPlayer; //ponteiro em playerList

    do {
        proxPlayer = (playerAtual + 1) % totalPlayers;
    } while (playerList[proxPlayer].ativo == false);

    playerAtual = proxPlayer;
}
// função pra pontuar playerAtual
void pontuarPlayer(uint8_t pontos) {
    playerList[playerAtual].score += pontos;
}

// NOTE: funções do jogo
// função pra resetar jogo (caso precise)
void resetJogo() {
    // CHECK: ver se falta mais algo pra resetar:
    sequenciaAtualGlobal = filaCor(); //valores padrões de filaCor
    // desativa players
    for (uint8_t i = 0; i < MAX_PLAYERS; i++) {
        playerList[i].reset();
    }
    playerAtual = 0;
    totalPlayers = 1;

    // msg confirmando que resetou tudo
    lcd.clear();
    printaString(MSGS_SISTEMA, 0, 0);
}
// TEST:
// função pra verificar sequência global com tentativa inserida pelo player
// FIX: acho que falta usar dequeue pra verificar a copia??
// deve ter mais erros
bool verificarJogada(filaCor tentativaAtual) {
    // cria uma cópia da variável global
    filaCor sequenciaAtualCopia = sequenciaAtualGlobal;
    if (tentativaAtual.qtd != sequenciaAtualCopia.qtd) return false;
    for (uint8_t i = 0; i < sequenciaAtualGlobal.qtd; i++) {
        if (tentativaAtual.sequenciaCor[i] != sequenciaAtualGlobal.sequenciaCor[i]) return false;
    }
    return true;
}
void mostrarPontuacaoOrdenada() {
    uint8_t ordem[MAX_LEDS]; //array na stack (temporário)
    // populando o array
    for (uint8_t i = 0; i < totalPlayers; i++) {
        ordem[i] = i;
    }

    // selection sort
    for (uint8_t i = 0; i < totalPlayers - 1; i++) {
        for (uint8_t j = i + 1; j < totalPlayers; j++) {
            if (playerList[ordem[j]].score > playerList[ordem[i]].score) {
                uint8_t temp = ordem[i];
                ordem[i] = ordem[j];
                ordem[j] = temp;
            }
        }
    }

    // exibição sequencial no LCD (do maior para o menor)
    printaString(PONTUACAO_MSGS, 0, 0); //titulo no y=0
    for (uint8_t i = 0; i < totalPlayers; i++) {
        //array ordenado e.g. [4, 2, 3, 1] = 1º lugar: P4; ...; último: P1
        uint8_t idx = ordem[i];

        if (playerList[idx].ativo) {
            lcd.clear();

            // TEST:
            lcd.print( + );
            printaString(PONTUACAO_MSGS, idx, 1); //pontuação no y=1
            lcd.print(playerList[idx].score);
            delay(2000);
        }
    }
}

// NOTE: funções pro display

// função pra printar strings guardadas na flash (PROGMEM)
void printaString(const char* const* tabelaString, uint8_t idx, uint8_t screenY) {
    const char* string = (const char*)pgm_read_ptr(&(tabelaString[idx]));

    //limpa
    lcd.setCursor(0, screenY);
    lcd.print("                ");
    //escreve
    lcd.setCursor(0, screenY);
    lcd.print((const __FlashStringHelper*)string);
}

// NOTE: funções pros botões

// função que verifica clique simples, duplo ou longo do botão menu
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

// NOTE: menus

void menuSetup() {
    uint8_t opcaoSelecionada = 1; //opção inicial na tela

    // estados atuais dos botões
    ultimoEstadoBotaoMenu = digitalRead(PINO_BOTAO_MENU);
    contagemCliques = 0;
    cliqueLongoProcessado = false;

    // prompt na tela
    lcd.clear();
    printaString(MENU_SETUP_MSGS, 0, 0); //prompt p escolher num jogadores (y=0)
    printaString(MENU_SETUP_MSGS, opcaoSelecionada, 1); //opção 1 (y=1)

    // loop bloqueante até a confirmação da seleção
    while (true) {
        AcaoBotao acao = verificarBotaoMenu();

        // itera entre opções do menu
        if (acao == CLIQUE_SIMPLES) {
            // opções 1 a 4 circularmente
            opcaoSelecionada = (opcaoSelecionada % 4) + 1;
            printaString(MENU_SETUP_MSGS, opcaoSelecionada, 1);
        }
        // se selecionou uma opção (clique duplo ou longo)
        else if (acao == CLIQUE_DUPLO || acao == CLIQUE_LONGO) {
            // seta globalmente
            totalPlayers = opcaoSelecionada;
            // ativa nossos objetos Player na memória
            for (uint8_t i = 0; i < totalPlayers; i++) {
                playerList[i].ativar();
            }
            // feedback da confirmação
            lcd.clear();
            printaString(MENU_SETUP_MSGS, 5, 0); //"confirmado"
            printaString(MENU_SETUP_MSGS, opcaoSelecionada, 1); //opção
            delay(1500);
            break; //sai do menuSetup
        }
    }
}

// TEST: menu pausa
void menuPausa() {
    uint8_t opcaoSelecionada = 1; //opção inicial na tela

    // estados atuais dos botões
    ultimoEstadoBotaoMenu = digitalRead(PINO_BOTAO_MENU);
    contagemCliques = 0;
    cliqueLongoProcessado = false;

    // prompt na tela
    lcd.clear();
    printaString(MENU_PAUSA_MSGS, 0, 0); //msg de que tá em pausa (y=0)
    printaString(MENU_PAUSA_MSGS, opcaoSelecionada, 1); //opção 1 (y=1)

    // loop bloqueante até a confirmação da seleção
    while (true) {
        AcaoBotao acao = verificarBotaoMenu();

        // itera entre opções do menu
        if (acao == CLIQUE_SIMPLES) {
            // opções 1 a 4 circularmente
            opcaoSelecionada = (opcaoSelecionada % 4) + 1;
            printaString(MENU_SETUP_MSGS, opcaoSelecionada, 1);
        }
        // se selecionou uma opção (clique duplo ou longo)
        else if (acao == CLIQUE_DUPLO || acao == CLIQUE_LONGO) {
            switch (opcaoSelecionada) {
                case 1: //voltar ao jogo
                    break;
                case 2: //mostrar pontuação atual em ordem de ranking
                    mostrarPontuacaoOrdenada();
                    continue; //volta pro menu de pausa
                case 3: //reiniciar jogo
                    resetJogo();
                    break;
                case 4:
                    terminarJogo();
                    break;
            }

            lcd.clear(); // REMOVE: só pra debug
            lcd.print("saindo de menuPausa"); // REMOVE: 
            delay(1000); // REMOVE: 
            break;
        }
    }
}

// TODO:
// função pra receber jogada (com lógica dos botões
// e acender LED como feedback)

// TEST: itera e depois mostra pro P1
void mostrarSequenciaGlobal() {
    for (uint8_t i = 0; i < sequenciaAtualGlobal.qtd; i++) {
        Color corAtual = sequenciaAtualGlobal.sequenciaCor[i];
        digitalWrite(PINOS_LEDS[corAtual], HIGH);
        delay(500); //tempo ligado
        digitalWrite(PINOS_LEDS[corAtual], LOW);
        delay(200); //tempo desligado antes de trocar cor
    }

    proxPlayer();
}

// TODO: ??
// void receberJogada

// NOTE: setup + loop
// NOTE: em setup, as coisas rodam apenas 1 vez, no início.
void setup() {
    // aqui realizamos o setup desejado pros pinos do arduino
    for (uint8_t i = 0; i < MAX_LEDS; i++) {
        pinMode(PINOS_LEDS[i], OUTPUT);
        pinMode(PINOS_BOTOES[i], INPUT);
    }

    // TEST: ver se INPUT_PULLUP dá certo atualmente no circuito
    pinMode(PINO_BOTAO_MENU, INPUT_PULLUP);
    pinMode(PINO_BOTAO_ON_OFF, INPUT_PULLUP);

    lcd.begin(16, 2);

    // leitura analógica de um pino desconectado como seed
    randomSeed(analogRead(2));
    Serial.begin(9600); // REMOVE: pra teste apenas

    menuSetup(); // NOTE: user escolhe qtd de jogadores
}

// NOTE: loop contínuo do jogo
// TODO: checagem do tempo de input
void loop() {
    // FIX: deve estar errado.. tem que usar verificarBotaoMenu
    // verifica botão menu/pausa
    uint8_t estadoBotaoMenu = digitalRead(PINO_BOTAO_MENU);
    if (estadoBotaoMenu == 1) {
        menuPausa();
    }

    // se for P1, vai mostrar a sequenciaAtualGlobal nos LEDs
    if (playerAtual == playerList[0]) {
        mostrarSequenciaGlobal();
    }

    // inicia tudo em LOW (0)
    uint8_t estadoBotoes[MAX_LEDS] = {0};

    // TODO: deve haver um tempo para poder inserir sequência (até 5s após o tempo da sequência atual)

    // recebe tentativa do jogador e compara com sequenciaAtualGlobal
    for (uint8_t i = 0; i < MAX_LEDS; i++) { //itera botões
        uint8_t estadoBotaoAtual = digitalRead(PINOS_BOTOES[i]);
        if (estadoBotaoAtual == 1) { //se HIGH
            tentativaAtual.enqueue(static_cast<Color>(i)); //insere cor escolhida na tentativa
            if (tentativaAtual.qtd == sequenciaAtualGlobal.qtd) { //se tentativa tiver a qtd correta
                if (verificarJogada(tentativaAtual) == true) { //se acertou sequência
                    // pontua jogador e passa pro prox
                    pontuarPlayer(PONTO_POR_JOGADA);
                    proxPlayer();
                    // TODO: gera cor nova ate chegar no máx. se chegar no máx, termina jogo e mostra pontuação!! (incluir delay e msg mostrando q resetou o jogo (talvez no proprio resetJogo()))
                    if (sequenciaAtualGlobal.qtd == MAX_SEQUENCIA) {
                        finalizarJogo(); // TODO:
                    }
                    newRandomColor(); //+1 cor
                } else { //se errou
                    // tira jogador e passa pro próx
                    playerList[playerAtual].desativar();
                    proxPlayer();
                    newRandomColor();
                }
            }
        }
    }
}
