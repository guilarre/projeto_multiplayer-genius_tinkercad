// TODO: menuSetup()
// - ligar leds no momento de selecionar num de jogadores no menuSetup()
// pra ter indicação visual de onde fica o botão de cada jogador
// BACKLOG: - selecionar num de jogadores através dos botões de jogadores??
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
// TODO: URGENTE: sequência de cores NÃO PODE TER REPETIÇÕES
//
// TEST: checar usos de char*

#include <stdint.h> // REMOVE:
#include <stdlib.h>
#include <sys/types.h> // CHECK:
#include <LiquidCrystal.h>
#include <avr/pgmspace.h>

#define MAX_LEDS 4
#define MAX_PLAYERS 4
#define MAX_SEQUENCIA 10
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

// NOTE: const globais
// FIX: vai precisar ser usando PROGMEM e
// ref de strings pras cores
const char *colorToString[] = {"VERDE", "VERMELHO", "AZUL", "AMARELO"};
// pinos leds e botões
const uint8_t PINOS_LEDS[MAX_LEDS] = {13, 12, 11, 10};
const uint8_t PINOS_BOTOES[MAX_LEDS] = {9, 8, 7, 6};
const uint8_t PINO_BOTAO_MENU = 5;
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

// vars globais para mapeamento dos cliques do botão menu
uint8_t ultimoEstadoBotao = HIGH; //HIGH por conta do INPUT_PULLUP
unsigned long tempoUltimaMudanca = 0;
unsigned long tempoPressionado = 0;
unsigned long tempoLiberado = 0;
uint8_t contagemCliques = 0;
bool cliqueLongoProcessado = false;

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
    public:
        // atributos
        uint16_t score;
        // CHECK: como vai ser a checagem do ranking?
        // uint8_t rank;
        bool ativo;
        // NOTE: deveria ter uma var pra jogada??

        // construtor
        Player() {
            score = 0;
            ativo = false;
            // rank = 0;
        }

        void reset() {
            score = 0;
            bool ativo = false;
        }
};

// NOTE: vars globais e funções do jogo

// NOTE: sequência de cores
// variável global pra guardar sequência
filaCor sequenciaAtualGlobal;
// variável global da jogada atual (vai ser resetada a cada tentativa correta)
filaCor jogadaAtual;
// função pra resetar jogo (caso precise)
void resetJogo() {
    sequenciaAtualGlobal = filaCor(); //valores padrões
}
// TEST: função pra inserir uma cor aleatória na var global
void randomColor() {
    uint8_t randNum = random(0, 4); //de 0 a 3
    sequenciaAtualGlobal.enqueue(static_cast<Color>(randNum));
}

// TEST:
// função pra comparar sequência atual com sequência inserida pelo player
// FIX: acho que falta usar dequeue pra verificar a copia?? ou dá no msm?
bool verificarJogada(filaCor jogadaAtual) {
    // cria uma cópia da variável global
    filaCor sequenciaAtualCopia = sequenciaAtualGlobal;
    if (jogadaAtual.qtd != sequenciaAtualCopia.qtd) return false;
    for (int i = 0; i < sequenciaAtualGlobal.qtd; i++) {
        if (jogadaAtual.sequenciaCor[i] != sequenciaAtualGlobal.sequenciaCor[i]) return false;
    }
    return true;
}

// NOTE: menus

// TODO:
// menu de setup: pegar qtd de jogadores desejada e setar globalmente (totalPlayers)
void menuSetup() {
    // TODO: printar o menu, solicitar totalPlayers...
    const char promptInicial[] = "Escolha a qtd de jogadores...";
    const char opcaoSetup1[] = "1 jogadores";
    const char opcaoSetup2[] = "2 jogadores";
    const char opcaoSetup3[] = "3 jogadores";
    const char opcaoSetup4[] = "4 jogadores";

    // lógica pra alternar opções no display
    // lcd.print("")

    // TODO: receber a qtd e setar totalPlayers
    // totalPlayers = ;
}

// TODO: menu pausa (mostrar que está em pausa e deixar opções pra mostrar ranking, score, terminar jogo, resetar...)
void menu_pausa() {
    // FIX: String -> char*
    String opcao1 = "Voltar ao jogo";
    String opcao2 = "Ver pontuação";
    String opcao3 = "Reiniciar jogo";
    String opcao4 = "Terminar jogo"; //termina e mostra pontuação
    String opcao5 = "";
    String opcao6 = "";
}

// TODO:
// função pra receber jogada (com lógica dos botões
// e acender LED como feedback)

// NOTE: strings pro menu
// FIX: String -> char*
String vezPlayer1 = "-- Player 1 --";
String vezPlayer2 = "-- Player 2 --";
String vezPlayer3 = "-- Player 3 --";
String vezPlayer4 = "-- Player 4 --";

// NOTE: instanciação dos jogadores
// cria os 4 jogadores na memória
Player playerList[MAX_PLAYERS];
// ponteiro pro player atual
uint8_t playerAtual = 0;
// qtd jogadores escolhida
uint8_t totalPlayers = 1;
// função pra ir pro próx player
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

// TEST: itera e depois mostra pro P1
void mostrarSequenciaGlobal() {
    for (int i = 0; i < sequenciaAtualGlobal.qtd; i++) {
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
    for (int i = 0; i < MAX_LEDS; i++) {
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
    // verifica botão menu/pausa
    uint8_t estadoBotaoMenu = digitalRead(PINO_BOTAO_MENU);
    if (estadoBotaoMenu == 1) {
        menu_pausa();
    }

    // se for P1, vai mostrar a sequenciaAtualGlobal nos LEDs
    if (playerAtual == playerList[0]) {
        mostrarSequenciaGlobal();
    }

    // inicia tudo em LOW (0)
    uint8_t estadoBotoes[MAX_LEDS] = {0};

    // TODO: deve haver um tempo para poder inserir sequência (até 5s após o tempo da sequência atual)

    // recebe até que jogadaAtual.qtd == sequenciaAtualGlobal.qtd
    // e compara jogadaAtual com sequenciaAtualGlobal
    for (int i = 0; i < MAX_LEDS; i++) {
        uint8_t estadoBotaoAtual = digitalRead(PINOS_BOTOES[i]);
        if (estadoBotaoAtual == 1) {
            jogadaAtual.enqueue(static_cast<Color>(i));
            if (jogadaAtual.qtd == sequenciaAtualGlobal.qtd) {
                if (verificarJogada(jogadaAtual) == true) { // NOTE: se acertou sequência
                    // pontua jogador e passa pro prox
                    pontuarPlayer(PONTO_POR_JOGADA);
                    proxPlayer();
                } else { // NOTE: se errou sequência
                    // tira jogador e passa pro próx
                    playerList[playerAtual].ativo = false;
                    proxPlayer();
                }
            }
        }
    }
}
