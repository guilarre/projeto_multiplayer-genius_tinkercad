// HACK: existe um bug no compilador do arduino, onde funções que recebem tipos definidos pelo desenvolvedor (e.g. verificarJogada(filaCor tentativa)) dão erro. a solução atual seria usar funções que recebem apenas tipos padrões (uint8_t, int, char...) como parâmetro, e utilizar variáveis GLOBAIS.
// (esse erro ocorre pq o compilador do arduino (pelo menos no tinkercad) não deixa de sobrescrever os protótipos das nossas funções, por algum motivo.)

#include <LiquidCrystal.h>
#include <avr/pgmspace.h>

#define MAX_LEDS 4
#define MAX_PLAYERS 4
#define MAX_SEQUENCIA 12
#define PONTO_POR_ACERTO 100
#define STRING_BUFFER_SIZE 100
#define TEMPO_LED_ACESO 400

// tem que declarar prototipo aqui p nao dar problema

// SECTION: enums, structs e classes

// enum pra cores (uint8_t (1 byte), em vez de int (2 ou 4 bytes))
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

// struct fila pra sequência atual de cores (FIFO)
struct filaCor {
    Color sequenciaCor[MAX_SEQUENCIA]; //array de cores
    uint8_t inicio = 0, final = 0, qtd = 0;

    bool enqueue(Color cor);
    bool dequeue(Color &corRemovida);
};

bool filaCor::enqueue(Color cor) {
    if (qtd == MAX_SEQUENCIA) {
        return false;
    }
    sequenciaCor[final] = cor;
    final = (final + 1) % MAX_SEQUENCIA; //circular
    qtd++;
    return true;
}

bool filaCor::dequeue(Color &corRemovida) {
    if (qtd == 0) {
        return false;
    }
    corRemovida = sequenciaCor[inicio];
    inicio = (inicio + 1) % MAX_SEQUENCIA; //circular
    qtd--;
    return true;
}

// classe player
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
uint8_t Player::countId = 0;

// SECTION: protótipos de nossas funções no início, pra evitar que pré-processador do arduino sobrescreva

AcaoBotao verificarBotaoMenu();
void menuSetup();
void menuPausa();
void printaString(const char* const* tabelaString, uint8_t idx, uint8_t screenY);
bool verificarJogada();
void mostrarPlacar();
void resetJogo();
void terminarJogo();
void newRandomColor();
void proxPlayer();
void pontuarPlayer(uint8_t pontos);
void mostrarSequenciaGlobal();

// SECTION: const globais

// pinos leds e botões
const uint8_t PINOS_LEDS[MAX_LEDS] = {13, 12, 11, 10};
const uint8_t PINOS_BOTOES[MAX_LEDS] = {9, 8, 7, 6};
const uint8_t PINO_BOTAO_MENU = 5;
const uint8_t PINO_RANDOM_SEED = A4;
// constantes de tempo p clique
const uint16_t TEMPO_DEBOUNCE = 50; //50ms pra como filtro pra ruidos
const uint16_t TEMPO_CLIQUE_DUPLO = 500; // CHECK: ver se vai usar msm
const uint16_t TEMPO_CLIQUE_LONGO = 500;
// pinos do lcd
const uint8_t PIN_LCD_RS = 2;
const uint8_t PIN_LCD_EN = 3;
const uint8_t PIN_LCD_D4 = A0;
const uint8_t PIN_LCD_D5 = A1;
const uint8_t PIN_LCD_D6 = A2;
const uint8_t PIN_LCD_D7 = A3;

// SECTION: inicialização do display

LiquidCrystal lcd(PIN_LCD_RS, PIN_LCD_EN, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);

// SECTION: strings na memoria flash (PROGMEM)

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
const char PROMPT_PAUSA[] PROGMEM = "--   PAUSA   --";
const char OPCAO_1_PAUSA[] PROGMEM = "Voltar";
const char OPCAO_2_PAUSA[] PROGMEM = "Placar atual";
const char OPCAO_3_PAUSA[] PROGMEM = "Reiniciar jogo";
const char OPCAO_4_PAUSA[] PROGMEM = "Terminar jogo";
// menuPausa: array de strings
const char* const MENU_PAUSA_MSGS[] PROGMEM = {
    PROMPT_PAUSA,
    OPCAO_1_PAUSA,
    OPCAO_2_PAUSA,
    OPCAO_3_PAUSA,
    OPCAO_4_PAUSA
};
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
// mostrarPlacar: strings
const char P1_MSG[] PROGMEM = "P1: ";
const char P2_MSG[] PROGMEM = "P2: ";
const char P3_MSG[] PROGMEM = "P3: ";
const char P4_MSG[] PROGMEM = "P4: ";
// mostrarPlacar: array de strings
const char* const PONTUACAO_MSGS[] PROGMEM = {
    P1_MSG,
    P2_MSG,
    P3_MSG,
    P4_MSG
};
// mensagens de sistema
const char CONFIRMACAO_RESET[] PROGMEM = "Reiniciando...";
const char COUNTDOWN_3[] PROGMEM = "3...";
const char COUNTDOWN_2[] PROGMEM = "2...";
const char COUNTDOWN_1[] PROGMEM = "1...";
const char COUNTDOWN_GO[] PROGMEM = "START!";
const char FEEDBACK_ERROU[] PROGMEM = "ERROU!";
const char FEEDBACK_ACERTOU[] PROGMEM = "ACERTOU!";
// array de strings
const char* const MSGS_SISTEMA[] PROGMEM {
    CONFIRMACAO_RESET,
    COUNTDOWN_3,
    COUNTDOWN_2,
    COUNTDOWN_1,
    COUNTDOWN_GO,
    FEEDBACK_ERROU,
    FEEDBACK_ACERTOU
};

// SECTION: vars globais e funções do jogo

// mapeamento dos cliques do botão menu
uint8_t ultimoEstadoBotaoMenu = HIGH; //HIGH por conta do INPUT_PULLUP
unsigned long tempoUltimaMudanca = 0;
unsigned long tempoPressionado = 0;
unsigned long tempoLiberado = 0;
uint8_t contagemCliques = 0;
bool cliqueLongoProcessado = false;
// vars e funções filaCor
filaCor sequenciaAtualGlobal = filaCor(); //fila global
filaCor tentativaAtualGlobal = filaCor();
// função pra inserir uma cor aleatória na var global
void newRandomColor() {
    uint8_t randNum = random(0, 4); //num aleatório de 0 a 3
    // insere na var global (damos static_cast de uint8_t -> Color):
    sequenciaAtualGlobal.enqueue(static_cast<Color>(randNum));
}
// vars e funções de player
Player playerList[MAX_PLAYERS]; //cria os 4 players na memória
// ponteiro pro player atual
uint8_t playerAtual = 0;
// qtd jogadores atual // CHECK:checar se qnd muda tá alterando efetivamente
uint8_t totalPlayers = 1;
// função pra iterar até o próx player válido
void proxPlayer() {
    if (totalPlayers == 1) return;

    uint8_t proxPlayer = playerAtual; //ponteiro em playerList

    do {
        proxPlayer = (proxPlayer + 1) % totalPlayers;
    } while (!playerList[proxPlayer].ativo && proxPlayer != playerAtual);

    playerAtual = proxPlayer;
}
// função pra pontuar playerAtual
void pontuarPlayer(uint8_t pontos) {
    playerList[playerAtual].score += pontos;
}

// SECTION: funções do jogo

// função pra resetar jogo
void resetJogo() {
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
    delay(2000);

    // chama setup novamente...
    menuSetup();
    newRandomColor();
}
// função pra verificar sequência global com tentativa inserida pelo player
bool verificarJogada() {
    // cria uma cópia da variável global
    filaCor sequenciaAtualCopia = sequenciaAtualGlobal;
    filaCor tentativaAtualCopia = tentativaAtualGlobal;

    if (tentativaAtualCopia.qtd != sequenciaAtualCopia.qtd) return false;

    for (uint8_t i = 0; i < sequenciaAtualGlobal.qtd; i++) {
        Color corAtualTentativa;
        Color corAtualGlobal;

        tentativaAtualCopia.dequeue(corAtualTentativa);
        sequenciaAtualCopia.dequeue(corAtualGlobal);

        if (corAtualTentativa != corAtualGlobal) return false;
    }
    return true;
}
void mostrarPlacar() {
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

    bool sairPlacar = false; //flag de controle
    while (!sairPlacar) {
        // loop dos players (exibição do maior pro menor)
        for (uint8_t i = 0; i < totalPlayers && !sairPlacar; i++) {
            //array ordenado e.g. [4, 2, 3, 1] = 1º lugar: P4; ...; último: P1
            uint8_t idx = ordem[i];

            if (playerList[idx].ativo) {
                lcd.clear();

                lcd.setCursor(0, 0); //ranking no y=0
                lcd.print(F("-- "));
                lcd.print(i+1);
                lcd.print(F("o lugar --"));

                lcd.setCursor(0, 1); //pontuação no y=1
                printaString(PONTUACAO_MSGS, idx, 1);
                lcd.print(playerList[idx].score);

                // verifica se quer sair
                unsigned long inicioEspera = millis();
                while (millis() - inicioEspera < 2000) { //2s na tela
                    // se quiser sair
                    if (verificarBotaoMenu() == CLIQUE_SIMPLES) {
                        sairPlacar = true; //sai do loop
                        break;
                    }
                    delay(10); //debounce
                }
            }
        }
    }
}
// função pra terminar e mostrar placar até que user deseje resetar
void terminarJogo() {
    mostrarPlacar();
    resetJogo();
}
// função alternativa pra delay(), pra verificar pausa
bool delayComPausa(unsigned long tempoMs) {
    unsigned long inicio = millis();
    while (millis() - inicio < tempoMs) {
        if (verificarBotaoMenu() == CLIQUE_SIMPLES) {
            menuPausa();
            return true; //foi pausado
        }
        delay(10); //bom pra estabilidade do botão e p CPU respirar, aparentemente
    }
    return false; //não pausou
}

// SECTION: funções pro display

// função pra printar strings guardadas na flash (PROGMEM)
void printaString(const char* const* tabelaString, uint8_t idx, uint8_t screenY) {
    const char* string = (const char*)pgm_read_ptr(&(tabelaString[idx]));

    //limpa
    lcd.setCursor(0, screenY);
    lcd.print(F("                "));
    //escreve
    lcd.setCursor(0, screenY);
    lcd.print((const __FlashStringHelper*)string);
}

// SECTION: funções pros botões

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

// SECTION: menus

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

// menu pausa
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
            printaString(MENU_PAUSA_MSGS, opcaoSelecionada, 1);
        }
        // se selecionou uma opção (clique duplo ou longo)
        else if (acao == CLIQUE_DUPLO || acao == CLIQUE_LONGO) {
            switch (opcaoSelecionada) {
                case 1: //voltar ao jogo
                    break;
                case 2: //mostrar placar + pontuação
                    mostrarPlacar();
                    opcaoSelecionada = 1; //opção 1: voltar
                    printaString(MENU_PAUSA_MSGS, opcaoSelecionada, 1); //mostra opção
                    continue; //volta pro menu de pausa
                case 3: //reiniciar jogo
                    resetJogo();
                    break;
                case 4:
                    terminarJogo();
                    break;
            }

            break;
        }
    }
}

// mostra sequência nos leds
void mostrarSequenciaGlobal() {
    for (uint8_t i = 0; i < sequenciaAtualGlobal.qtd; i++) {
        Color corAtual = sequenciaAtualGlobal.sequenciaCor[i];
        digitalWrite(PINOS_LEDS[corAtual], HIGH); //acende
        delay(TEMPO_LED_ACESO);
        digitalWrite(PINOS_LEDS[corAtual], LOW); //apaga
        delay(200);
    }
}

// SECTION: setup + loop

// NOTE: em setup, as coisas rodam apenas 1 vez, no início.
void setup() {
    // aqui realizamos o setup desejado pros pinos do arduino
    for (uint8_t i = 0; i < MAX_LEDS; i++) {
        pinMode(PINOS_LEDS[i], OUTPUT);
        pinMode(PINOS_BOTOES[i], INPUT_PULLUP);
    }

    pinMode(PINO_BOTAO_MENU, INPUT_PULLUP);

    lcd.begin(16, 2);

    // leitura analógica de um pino desconectado como seed
    randomSeed(analogRead(PINO_RANDOM_SEED));

    menuSetup(); // NOTE: user escolhe qtd de jogadores
    newRandomColor(); //primeira cor
}

// TODO: checagem do tempo de input (deve haver um tempo para poder inserir sequência (e.g. até 5s após o tempo da sequência atual))
// NOTE: loop contínuo do jogo
void loop() {
    // verifica botão menu/pausa
    AcaoBotao acao = verificarBotaoMenu();
    // FIX: chamada tem q ocorrer de qualquer local do código
    if (acao == CLIQUE_SIMPLES) {
        menuPausa();
    }

    // mostra de quem é a vez atual em y=0
    lcd.clear();
    printaString(VEZ_ATUAL_MSGS, playerAtual, 0);

    // antes de mostrar a sequência global, mensagem de 3 2 1... go
    // TEST: testar colocar delay menor OU mostrar countdown apenas na primeira rodada
    for (uint8_t i = 1; i < 5; i++) {
        printaString(MSGS_SISTEMA, i, 1);
        delayComPausa(1000);
    }

    mostrarSequenciaGlobal(); //a cada player, incrementa e mostra

    tentativaAtualGlobal = filaCor(); //limpa tentativaAtualGlobal pro player novo

    // recebe tentativa do player e compara com sequenciaAtualGlobal
    while (tentativaAtualGlobal.qtd < sequenciaAtualGlobal.qtd) {
        for (uint8_t i = 0; i < MAX_LEDS; i++) { //itera botões
            if (digitalRead(PINOS_BOTOES[i]) == LOW) {
                // pisca cor selecionada no LED (feedback)
                digitalWrite(PINOS_LEDS[i], HIGH);
                delay(TEMPO_LED_ACESO);
                digitalWrite(PINOS_LEDS[i], LOW);

                // insere cor escolhida na tentativa
                tentativaAtualGlobal.enqueue(static_cast<Color>(i));

                // debounce enquanto botão n é solto
                while (digitalRead(PINOS_BOTOES[i]) == LOW) {
                    delay(10);
                }
                break;
            }
        }
    }

    // se tentativa tiver a qtd correta, hora de verificar
    if (verificarJogada()) { //se acertou sequência
        printaString(MSGS_SISTEMA, 6, 1);
        delayComPausa(1000);

        // pontua jogador e passa pro prox
        pontuarPlayer(PONTO_POR_ACERTO);
        proxPlayer();
        // verifica se sequência chegou na qtd máx.
        if (sequenciaAtualGlobal.qtd == MAX_SEQUENCIA) {
            terminarJogo(); //termina e mostra placar
        } else {
            newRandomColor(); //se acertar, +1 cor
        }
    } else { //se errou
        printaString(MSGS_SISTEMA, 5, 1);
        delayComPausa(1000);
        if (totalPlayers == 1) { //se errou e só tem 1 player
            terminarJogo(); //termina e mostra placar
        } else {
            // tira jogador e passa pro próx
            playerList[playerAtual].desativar();
            totalPlayers -= 1;
            proxPlayer();
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// FIX: ao finalizar o jogo, n retira player atual (pelo menos na tela)
// FIX: verificar a cada inserção
//
// TODO:
// - remodelar seleção de qtd de jogadores pra ser através dos botões dos leds
// - ligar leds no momento de selecionar num de jogadores no menuSetup() pra ter indicação visual de onde fica o botão de cada jogador
// - checar quantos segundos leva pra mostrar sequencia atual e se deve diminuir ao longo do jogo
// - pensar melhor como será sistema de pontuação...
// - melhorar feedback ao user -> piscar as luzes, som, som qnd selecionar opção, qnd acertar, errar..
//
// TEST: talvez deixar sequencia até 15/20..?
//
// BACKLOG:
// - selecionar num de jogadores através dos botões de jogadores
// - persistência na memória pra guardar highscore
// - limitar numero de cores repetidas na sequência.. (2 ou 3 max)
// - se fizer houver bateria, vai ter que ter como ligar/desligar (botão on/off ou switch?)
// - aumentar dificuldade conforme passam rodadas
