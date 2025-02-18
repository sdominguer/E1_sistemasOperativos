#include <iostream>
#include <unordered_map>
#include <queue>
#include <vector>
#include <bitset>
#include <fcntl.h>
#include <unistd.h>
#include <cstdint>

using namespace std;

// ---------------------------------------------------------------------
// Estructura que representa un nodo en el árbol de Huffman.
// Cada nodo almacena un carácter, su frecuencia y punteros a sus hijos.
// ---------------------------------------------------------------------
struct Node {
    char ch;         // Carácter (válido si el nodo es una hoja)
    int freq;        // Frecuencia de aparición del carácter
    Node *left, *right; // Punteros a los nodos hijo izquierdo y derecho

    // Constructor que inicializa el nodo con un carácter y su frecuencia
    Node(char c, int f) : ch(c), freq(f), left(nullptr), right(nullptr) {}
};

// -----------------------------------------------------------
// Estructura para comparar nodos en la cola de prioridad.
// Se utiliza para formar un min-heap, dando prioridad a nodos
// con menor frecuencia.
// -----------------------------------------------------------
struct Compare {
    bool operator()(Node* a, Node* b) { 
        return a->freq > b->freq; // Regresa true si la frecuencia de 'a' es mayor que la de 'b'
    }
};

// ------------------------------------------------------------------
// Función recursiva que recorre el árbol de Huffman para asignar a cada
// carácter su código binario. Se añade '0' al código al ir por la rama
// izquierda y '1' al ir por la derecha.
// ------------------------------------------------------------------
void buildHuffmanCodes(Node* root, string code, unordered_map<char, string>& huffmanCodes) {
    if (!root) return; // Caso base: si el nodo es nulo, se regresa

    // Si el nodo es una hoja, se asigna el código binario acumulado para ese carácter
    if (!root->left && !root->right) {
        huffmanCodes[root->ch] = code;
    }
    
    // Recorrido recursivo por el subárbol izquierdo agregando '0'
    buildHuffmanCodes(root->left, code + "0", huffmanCodes);
    // Recorrido recursivo por el subárbol derecho agregando '1'
    buildHuffmanCodes(root->right, code + "1", huffmanCodes);
}

// -------------------------------------------------------------------------------------
// Función que comprime un archivo dado utilizando el algoritmo de codificación Huffman.
// Realiza dos lecturas del archivo: la primera para calcular las frecuencias de lectura y
// la segunda para generar el texto codificado.
// -------------------------------------------------------------------------------------
void compressFile(const string& inputFile, const string& outputFile) {
    // Abrir el archivo de entrada en modo lectura
    int fd = open(inputFile.c_str(), O_RDONLY);
    if (fd == -1) { 
        perror("Error opening file");
        return; 
    }

    // Contar la frecuencia de cada carácter en el archivo
    unordered_map<char, int> freq;
    char buffer[1024];
    ssize_t bytesRead;
    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < bytesRead; i++) {
            freq[buffer[i]]++;
        }
    }
    close(fd); // Cerrar el archivo de entrada

    // Crear la cola de prioridad (min-heap) para construir el árbol de Huffman
    priority_queue<Node*, vector<Node*>, Compare> pq;
    for (auto& pair : freq) {
        pq.push(new Node(pair.first, pair.second));
    }
    // Combinar nodos hasta formar el árbol completo
    while (pq.size() > 1) {
        Node* left = pq.top(); 
        pq.pop();
        Node* right = pq.top(); 
        pq.pop();
        // Crear un nodo padre con frecuencia igual a la suma de las frecuencias
        Node* parent = new Node('\0', left->freq + right->freq);
        parent->left = left; 
        parent->right = right;
        pq.push(parent);
    }
    // Nodo raíz del árbol de Huffman
    Node* root = pq.top();

    // Generar los códigos de Huffman para cada carácter
    unordered_map<char, string> huffmanCodes;
    buildHuffmanCodes(root, "", huffmanCodes);

    // Reabrir el archivo de entrada para generar el texto codificado
    fd = open(inputFile.c_str(), O_RDONLY);
    if (fd == -1) { 
        perror("Error reopening file");
        return; 
    }
    // Abrir el archivo de salida donde se escribirá el contenido comprimido
    int outFd = open(outputFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (outFd == -1) { 
        perror("Error creating output file");
        return; 
    }

    // Escribir el encabezado del archivo comprimido:
    // Primero, se escribe la cantidad de caracteres únicos
    uint32_t uniqueCount = freq.size();
    write(outFd, &uniqueCount, sizeof(uniqueCount));
    // Luego, para cada carácter único se escribe el carácter y su frecuencia
    for (auto& pair : freq) {
        write(outFd, &pair.first, sizeof(char));
        uint32_t frequency = pair.second;
        write(outFd, &frequency, sizeof(frequency));
    }

    // Codificar el contenido original usando los códigos de Huffman
    string encodedText = "";
    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < bytesRead; i++) {
            encodedText += huffmanCodes[buffer[i]];
        }
    }
    close(fd); // Cerrar el archivo de entrada

    // Convertir la cadena de bits a bytes para escribirlos en el archivo comprimido
    string byteString = "";
    vector<uint8_t> byteData;
    for (char bit : encodedText) {
        byteString += bit;
        // Cada vez que se acumulan 8 bits se convierte a un byte
        if (byteString.size() == 8) {
            byteData.push_back(bitset<8>(byteString).to_ulong());
            byteString = "";
        }
    }
    // Si quedan bits sin completar un byte, se rellenan con ceros a la derecha.
    if (!byteString.empty()) {
        while (byteString.size() < 8) {
            byteString += "0";
        }
        byteData.push_back(bitset<8>(byteString).to_ulong());
    }

    // Escribir los bytes codificados en el archivo comprimido
    write(outFd, byteData.data(), byteData.size());
    close(outFd); // Cerrar el archivo de salida

    cout << "Archivo comprimido guardado como: " << outputFile << endl;
}

// -------------------------------------------------------------------------------------
// Función que descomprime un archivo comprimido utilizando el algoritmo de Huffman.
// Lee el encabezado para reconstruir la tabla de frecuencias, reconstruye el árbol y
// decodifica la secuencia de bits para recuperar el contenido original.
// -------------------------------------------------------------------------------------
void decompressFile(const string& inputFile, const string& outputFile) {
    // Abrir el archivo comprimido en modo lectura
    int fd = open(inputFile.c_str(), O_RDONLY);
    if (fd == -1) { 
        perror("Error opening compressed file");
        return; 
    }

    // Leer el encabezado: número de caracteres únicos que fueron comprimidos
    uint32_t uniqueCount;
    if (read(fd, &uniqueCount, sizeof(uniqueCount)) != sizeof(uniqueCount)) {
        perror("Error reading header");
        close(fd);
        return;
    }

    // Leer la tabla de frecuencias del encabezado
    unordered_map<char, uint32_t> freq;
    for (uint32_t i = 0; i < uniqueCount; i++) {
        char c;
        uint32_t f;
        if (read(fd, &c, sizeof(c)) != sizeof(c)) {
            perror("Error reading frequency table");
            close(fd);
            return;
        }
        if (read(fd, &f, sizeof(f)) != sizeof(f)) {
            perror("Error reading frequency table");
            close(fd);
            return;
        }
        freq[c] = f;
    }

    // Reconstruir el árbol de Huffman a partir de la tabla de frecuencias leída
    priority_queue<Node*, vector<Node*>, Compare> pq;
    for (auto& p : freq) {
        pq.push(new Node(p.first, p.second));
    }
    if (pq.empty()) {
        close(fd);
        return;
    }
    // Unir nodos hasta formar el árbol completo
    while (pq.size() > 1) {
        Node* left = pq.top(); 
        pq.pop();
        Node* right = pq.top(); 
        pq.pop();
        Node* parent = new Node('\0', left->freq + right->freq);
        parent->left = left; 
        parent->right = right;
        pq.push(parent);
    }
    Node* root = pq.top();

    // Leer el resto del archivo comprimido y almacenarlo en un vector de bytes
    vector<uint8_t> byteData;
    char buffer[1024];
    ssize_t bytesRead;
    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < bytesRead; i++) 
            byteData.push_back(static_cast<uint8_t>(buffer[i]));
    }
    close(fd); // Cerrar el archivo comprimido

    // Convertir cada byte leído en una cadena de 8 bits
    string bitString = "";
    for (uint8_t byte : byteData) {
        bitString += bitset<8>(byte).to_string();
    }

    // Calcular el número total de caracteres originales usando la tabla de frecuencias
    int totalChars = 0;
    for (auto &p : freq) {
        totalChars += p.second;
    }

    // Abrir el archivo de salida en donde se escribirá el contenido descomprimido
    int outFd = open(outputFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (outFd == -1) { 
        perror("Error creating decompressed file");
        return;
    }

    // Decodificar la cadena de bits recorriéndola y utilizando el árbol de Huffman
    Node* current = root;
    int decodedCount = 0;
    for (char bit : bitString) {
        // Desplazarse a la izquierda si el bit es '0', a la derecha si es '1'
        current = (bit == '0' ? current->left : current->right);
        // Si se llega a una hoja, se ha decodificado un carácter y se escribe en el archivo
        if (!current->left && !current->right) {
            write(outFd, &current->ch, 1);
            decodedCount++;
            // Si ya se han decodificado todos los caracteres originales, se finaliza el proceso
            if (decodedCount == totalChars) 
                break;
            // Reiniciar la búsqueda en la raíz para decodificar el siguiente carácter
            current = root;
        }
    }
    close(outFd); // Cerrar el archivo de salida

    cout << "Archivo descomprimido guardado como: " << outputFile << endl;
}

// ---------------------------------------------------------------------
// Función que muestra la ayuda con las opciones de uso del programa.
// ---------------------------------------------------------------------
void showHelp() {
    cout << "Uso: ./programa <opcion> <archivo>\n";
    cout << "Opciones:\n";
    cout << "  -h, --help         Mostrar este mensaje de ayuda\n";
    cout << "  -v, --version      Mostrar la versión del programa\n";
    cout << "  -c, --compress     Comprimir el archivo\n";
    cout << "  -x, --decompress   Descomprimir el archivo\n";
}

// ---------------------------------------------------------------------
// Función que muestra la versión del programa.
// ---------------------------------------------------------------------
void showVersion() {
    cout << "Compresor Huffman - Versión 1.0\n";
}

// ---------------------------------------------------------------------
// Función principal donde se evalúan los argumentos y se direcciona
// el flujo del programa según la opción seleccionada.
// ---------------------------------------------------------------------
int main(int argc, char* argv[]) {
    // Verificar si se proporcionó al menos una opción.
    if (argc < 2) {
        cout << "Uso incorrecto. Usa -h para ver las opciones.\n";
        return 1;
    }
    
    // Obtener la opción pasada como argumento
    string option = argv[1];
    if (option == "-h" || option == "--help") {
        // Mostrar el mensaje de ayuda
        showHelp();
    } else if (option == "-v" || option == "--version") {
        // Mostrar la versión del programa
        showVersion();
    } else if ((option == "-c" || option == "--compress") && argc == 3) {
        // Si se selecciona compresión, se procesa el archivo y se crea un archivo .huff
        string filename = argv[2];
        compressFile(filename, filename + ".huff");
    } else if ((option == "-x" || option == "--decompress") && argc == 3) {
        // Si se selecciona descompresión, se procesa el archivo y se crea un archivo .orig
        string filename = argv[2];
        decompressFile(filename, filename + ".orig");
    } else {
        // Opción no reconocida, se notifica al usuario
        cout << "Opción no reconocida. Usa -h para ayuda.\n";
        return 1;
    }
    return 0;
}
