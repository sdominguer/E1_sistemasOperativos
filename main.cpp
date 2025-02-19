#include <iostream>
#include <unordered_map>
#include <queue>
#include <vector>
#include <bitset>
#include <fcntl.h>
#include <unistd.h>
#include <cstdint>
#include <algorithm>
#include <sys/stat.h>
#include <sys/mman.h>

using namespace std;

// ---------------------------------------------------------------------
// Estructura que representa un nodo en el árbol de Huffman.
// Cada nodo almacena un carácter, su frecuencia y punteros hacia sus hijos.
// ---------------------------------------------------------------------
struct Node {
    char ch;
    uint32_t freq;
    Node *left, *right;
    Node(char c, uint32_t f) : ch(c), freq(f), left(nullptr), right(nullptr) {}
};

// ---------------------------------------------------------------------
// Estructura para comparar nodos en la cola de prioridad.
// Se utiliza para formar un min-heap basado en la frecuencia.
// ---------------------------------------------------------------------
struct Compare {
    bool operator()(Node* a, Node* b) {
        return a->freq > b->freq;
    }
};

// ---------------------------------------------------------------------
// Comparador global para ordenar los pares (carácter, frecuencia)
// de forma determinista: primero por frecuencia (menor a mayor) y,
// en caso de empate, por valor del carácter (menor a mayor).
// ---------------------------------------------------------------------
struct FrequencyCompare {
    bool operator()(const pair<char, uint32_t>& a, const pair<char, uint32_t>& b) const {
        if (a.second == b.second)
            return a.first < b.first;
        return a.second < b.second;
    }
};

// ---------------------------------------------------------------------
// Función recursiva para liberar la memoria del árbol de Huffman.
// ---------------------------------------------------------------------
void freeHuffmanTree(Node* root) {
    if (!root)
        return;
    freeHuffmanTree(root->left);
    freeHuffmanTree(root->right);
    delete root;
}

// ---------------------------------------------------------------------
// Función recursiva que recorre el árbol de Huffman para asignar a cada
// carácter su código binario. Se añade '0' al código al ir por la rama
// izquierda y '1' al ir por la derecha.
// ---------------------------------------------------------------------
void buildHuffmanCodes(Node* root, string code, unordered_map<char, string>& huffmanCodes) {
    if (!root)
        return;

    if (!root->left && !root->right) {
        // Caso especial: si el archivo tiene un único carácter, se asigna "0"
        if (code == "")
            code = "0";
        huffmanCodes[root->ch] = code;
        return;
    }
    
    buildHuffmanCodes(root->left, code + "0", huffmanCodes);
    buildHuffmanCodes(root->right, code + "1", huffmanCodes);
}

// -------------------------------------------------------------------------------------
// Función que comprime un archivo utilizando el algoritmo de codificación Huffman.
// Utiliza fstat y mmap para leer el archivo en bloque, reduciendo copias y llamadas al sistema.
// -------------------------------------------------------------------------------------
void compressFile(const string& inputFile, const string& outputFile) {
    int fd = open(inputFile.c_str(), O_RDONLY);
    if (fd == -1) { 
        perror("Error opening input file");
        return; 
    }
    
    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("Error getting file stats");
        close(fd);
        return;
    }
    size_t fileSize = st.st_size;
    char* fileData = (char*) mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, fd, 0);
    if (fileData == MAP_FAILED) {
        perror("Error mapping file");
        close(fd);
        return;
    }
    close(fd);

    unordered_map<char, uint32_t> freq;
    for (size_t i = 0; i < fileSize; i++) {
        freq[fileData[i]]++;
    }
    munmap(fileData, fileSize);

    vector< pair<char, uint32_t> > frequencies(freq.begin(), freq.end());
    sort(frequencies.begin(), frequencies.end(), FrequencyCompare());

    priority_queue<Node*, vector<Node*>, Compare> pq;
    for (vector< pair<char, uint32_t> >::iterator it = frequencies.begin(); it != frequencies.end(); ++it) {
        pq.push(new Node(it->first, it->second));
    }
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

    unordered_map<char, string> huffmanCodes;
    buildHuffmanCodes(root, "", huffmanCodes);

    fd = open(inputFile.c_str(), O_RDONLY);
    if (fd == -1) { 
        perror("Error reopening input file");
        freeHuffmanTree(root);
        return; 
    }
    if (fstat(fd, &st) == -1) {
        perror("Error getting file stats on second pass");
        close(fd);
        freeHuffmanTree(root);
        return;
    }
    fileSize = st.st_size;
    fileData = (char*) mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, fd, 0);
    if (fileData == MAP_FAILED) {
        perror("Error mapping file on second pass");
        close(fd);
        freeHuffmanTree(root);
        return;
    }
    close(fd);

    string encodedText;
    encodedText.reserve(fileSize); 
    for (size_t i = 0; i < fileSize; i++) {
        encodedText += huffmanCodes[fileData[i]];
    }
    munmap(fileData, fileSize);

    string byteString;
    vector<uint8_t> byteData;
    for (string::iterator it = encodedText.begin(); it != encodedText.end(); ++it) {
        byteString += *it;
        if (byteString.size() == 8) {
            byteData.push_back((uint8_t) bitset<8>(byteString).to_ulong());
            byteString = "";
        }
    }
    if (!byteString.empty()) {
        while (byteString.size() < 8)
            byteString += "0";
        byteData.push_back((uint8_t) bitset<8>(byteString).to_ulong());
    }

    int outFd = open(outputFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (outFd == -1) { 
        perror("Error creating output file");
        freeHuffmanTree(root);
        return; 
    }
    uint32_t uniqueCount = frequencies.size();
    write(outFd, &uniqueCount, sizeof(uniqueCount));
    for (vector< pair<char, uint32_t> >::iterator it = frequencies.begin(); it != frequencies.end(); ++it) {
        write(outFd, &it->first, sizeof(char));
        uint32_t frequency = it->second;
        write(outFd, &frequency, sizeof(frequency));
    }
    write(outFd, byteData.data(), byteData.size());
    close(outFd);

    freeHuffmanTree(root);
    cout << "Archivo comprimido guardado como: " << outputFile << endl;
}

// -------------------------------------------------------------------------------------
// Función que descomprime un archivo comprimido utilizando el algoritmo de Huffman.
// Utiliza mmap para mapear todo el archivo comprimido y acceder directamente al buffer de memoria.
// -------------------------------------------------------------------------------------
void decompressFile(const string& inputFile, const string& outputFile) {
    int fd = open(inputFile.c_str(), O_RDONLY);
    if (fd == -1) { 
        perror("Error opening compressed file");
        return; 
    }
    
    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("Error getting compressed file stats");
        close(fd);
        return;
    }
    size_t fileSize = st.st_size;
    char* fileData = (char*) mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, fd, 0);
    if (fileData == MAP_FAILED) {
        perror("Error mapping compressed file");
        close(fd);
        return;
    }
    close(fd);

    char* pos = fileData;
    uint32_t uniqueCount = *(uint32_t*) pos;
    pos += sizeof(uint32_t);

    vector< pair<char, uint32_t> > frequencies;
    for (uint32_t i = 0; i < uniqueCount; i++) {
        char c = *pos;
        pos += sizeof(char);
        uint32_t f = *(uint32_t*) pos;
        pos += sizeof(uint32_t);
        frequencies.push_back(make_pair(c, f));
    }
    sort(frequencies.begin(), frequencies.end(), FrequencyCompare());

    priority_queue<Node*, vector<Node*>, Compare> pq;
    for (vector< pair<char, uint32_t> >::iterator it = frequencies.begin(); it != frequencies.end(); ++it) {
        pq.push(new Node(it->first, it->second));
    }
    if (pq.empty()) {
        munmap(fileData, fileSize);
        return;
    }
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

    size_t headerSize = pos - fileData;
    size_t compressedSize = fileSize - headerSize;

    string bitString;
    for (size_t i = 0; i < compressedSize; i++) {
        bitString += bitset<8>((uint8_t) pos[i]).to_string();
    }
    munmap(fileData, fileSize);

    int totalChars = 0;
    for (vector< pair<char, uint32_t> >::iterator it = frequencies.begin(); it != frequencies.end(); ++it) {
        totalChars += it->second;
    }

    int outFd = open(outputFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (outFd == -1) { 
        perror("Error creating decompressed file");
        freeHuffmanTree(root);
        return;
    }

    Node* current = root;
    int decodedCount = 0;
    for (string::iterator it = bitString.begin(); it != bitString.end(); ++it) {
        current = (*it == '0' ? current->left : current->right);
        if (!current->left && !current->right) {
            write(outFd, &current->ch, 1);
            decodedCount++;
            if (decodedCount == totalChars)
                break;
            current = root;
        }
    }
    close(outFd);

    freeHuffmanTree(root);
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
    if (argc < 2) {
        cout << "Uso incorrecto. Usa -h para ver las opciones.\n";
        return 1;
    }
    
    string option = argv[1];
    if (option == "-h" || option == "--help") {
        showHelp();
    } else if (option == "-v" || option == "--version") {
        showVersion();
    } else if ((option == "-c" || option == "--compress") && argc == 3) {
        string filename = argv[2];
        compressFile(filename, filename + ".huff");
    } else if ((option == "-x" || option == "--decompress") && argc == 3) {
        string filename = argv[2];
        decompressFile(filename, filename + ".orig");
    } else {
        cout << "Opción no reconocida. Usa -h para ayuda.\n";
        return 1;
    }
    return 0;
}
