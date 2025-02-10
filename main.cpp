#include <iostream>  
#include <fstream>  
#include <bitset>  
#include <string>  
#include <sstream>  
#include "Rle.h" // Incluir el archivo de encabezado  

using namespace std;  

// Constantes globales  
const string FILENAME = "archivo.bin"; // Archivo a leer  
const bool IS_BINARY = true; // Indica si es binario  

// Función que convierte un byte en su representación binaria de 8 bits  
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
    
    // Lee el archivo byte por byte  
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
    if (bitStream.empty()) return 1; // Salir si el contenido está vacío  

    cout << "\nCadena de bits:\n" << bitStream << endl;  

    // Aplicar la compresión RLE  
    string compressed = rleCompress(bitStream);  
    cout << "\nContenido comprimido:\n" << compressed << endl;  

    return 0;  
}
