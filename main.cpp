#include <iostream>
#include <fstream>
#include <bitset>
#include <string>

using namespace std;

// variables globales

const string FILENAME = "archivo.bin"; // definir a FILENAME como el archivo a leer
const bool IS_BINARY = true; // indica si es bin o txt- Cambiar a false si es un archivo de txt

// funcion que convierte un byte en su representación binaria de 8 bits
string byteToBits(unsigned char byte) {
    return bitset<8>(byte).to_string();
}

// Lee un archivo (bin o txt) y devuelve su contenido como una cadena de bits
string readFileAsBits(const string &filename, bool isBinary) {
    ifstream file(filename, isBinary ? ios::binary : ios::in);
    if (!file) {
        cerr << "Error al abrir el archivo: " << filename << endl;
        return "";
    }
// lee el archivo byte por byte
    string bitStream;
    char byte;
    while (file.get(byte)) {
        bitStream += byteToBits(static_cast<unsigned char>(byte));
    }

    file.close();
    return bitStream;
}

int main() {
    string bitStream = readFileAsBits(FILENAME, IS_BINARY);
    if (bitStream.empty()) return 1;

    cout << "\nCadena de bits:\n" << bitStream << endl;

    return 0;
}
