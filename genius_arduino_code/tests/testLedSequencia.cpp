#define MAX_LEDS 4
#define MAX_SEQUENCIA 10

enum Color : uint8_t {
    VERDE = 0,
    VERMELHO = 1,
    AZUL = 2,
    AMARELO = 3
};

// NOTE: precisa declarar protótipos das funções logo pra não dar
// erro pelo pré-processador do arduino

// protótipos e constantes
struct filaCor {
    Color sequenciaCor[MAX_SEQUENCIA]; //array de cores
    uint8_t inicio = 0, final = 0, qtd = 0;
    // protótipos
    bool enqueue(Color cor);
    bool dequeue(Color &corRemovida);
};

// TEST: protótipo do nosso teste
void testeMostrarSequencia(filaCor sequencia);

// pinos
const uint8_t PINOS_LEDS[MAX_LEDS] = {13, 12, 11, 10};

// implementação dos métodos de filaCor
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

// TEST:
// sequencia pro teste vazia
filaCor sequenciaTeste;

// bool terminouSeq = true;
void testeMostrarSequencia(filaCor sequencia) {
    // if (!terminouSeq) return;

    for (int i = 0; i < sequencia.qtd; i++) {
        Color corAtual = sequencia.sequenciaCor[i];
        digitalWrite(PINOS_LEDS[corAtual], HIGH);
        delay(500); //tempo ligado
        digitalWrite(PINOS_LEDS[corAtual], LOW);
        delay(200); //tempo desligado antes de trocar cor
    }

    // terminouSeq = true;
}

void setup() {
    // setup dos pinos
    for (int i = 0; i < MAX_LEDS; i++) {
        pinMode(PINOS_LEDS[i], OUTPUT);
    }

    // populando sequencia p teste
    sequenciaTeste.enqueue(VERDE);
    sequenciaTeste.enqueue(VERMELHO);
    sequenciaTeste.enqueue(AZUL);
    sequenciaTeste.enqueue(VERDE);
    sequenciaTeste.enqueue(VERMELHO);
    sequenciaTeste.enqueue(AMARELO); 
}

void loop() {
    testeMostrarSequencia(sequenciaTeste);
    delay(1000);
}
