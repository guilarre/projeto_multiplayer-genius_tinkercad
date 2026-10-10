#include <iostream>
#include <cstdint>
#include <sys/types.h>
#include <random>

#define MAX_SEQUENCIA 10

using namespace std;

enum Color : uint8_t {
    VERDE,
    VERMELHO,
    AZUL,
    AMARELO
};

Color randomColor() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> distrib(0,3);
    uint8_t randNum = distrib(gen);
    return static_cast<Color>(randNum);
}

struct filaCor {
    Color sequenciaAtual[MAX_SEQUENCIA];
    uint8_t inicio = 0, final = 0, qtd = 0;

    bool enqueue(Color cor) {
        if (qtd == MAX_SEQUENCIA) {
            return false;
        }
        sequenciaAtual[final] = cor;
        // CHECK:
        final = (final + 1) % MAX_SEQUENCIA; //circular
        qtd++;
        return true;
    }

    bool dequeue(Color &corRemovida) {
        if (qtd == 0) {
            return false;
        }
        corRemovida = sequenciaAtual[inicio];
        // CHECK:
        inicio = (inicio + 1) % MAX_SEQUENCIA; //circular
        qtd--;
        return true;
    }
};

const char* colorString[] = {"VERDE", "VERMELHO", "AZUL", "AMARELO"};

int main() {

    int playerAtual = 0;
    for (int i = 0; i < 4; i++) {
        playerAtual = (playerAtual + 1) % 4;
        cout << playerAtual << endl;
    }


    return 0;
}
