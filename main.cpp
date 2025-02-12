#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <queue>
#include <bitset>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

// Estructura para los nodos del árbol de Huffman
struct HuffmanNode {
    char data;
    int freq;
    HuffmanNode *left, *right;
    HuffmanNode(char d, int f) : data(d), freq(f), left(nullptr), right(nullptr) {}
};

// Comparador para la cola de prioridad
struct Compare {
    bool operator()(HuffmanNode* l, HuffmanNode* r) {
        return l->freq > r->freq;
    }
};

// Función para construir el árbol de Huffman
typedef unordered_map<char, string> HuffmanCodeMap;
HuffmanNode* buildHuffmanTree(unordered_map<char, int>& freqMap) {
    priority_queue<HuffmanNode*, vector<HuffmanNode*>, Compare> minHeap;
    
    for (auto& pair : freqMap) {
        minHeap.push(new HuffmanNode(pair.first, pair.second));
    }
    
    while (minHeap.size() > 1) {
        HuffmanNode *left = minHeap.top(); minHeap.pop();
        HuffmanNode *right = minHeap.top(); minHeap.pop();
        
        HuffmanNode *merged = new HuffmanNode('\0', left->freq + right->freq);
        merged->left = left;
        merged->right = right;
        
        minHeap.push(merged);
    }
    return minHeap.top();
}

// Función para generar códigos de Huffman
void generateHuffmanCodes(HuffmanNode* root, string code, HuffmanCodeMap &huffmanCode) {
    if (!root) return;
    
    if (root->data != '\0') {
        huffmanCode[root->data] = code;
    }
    
    generateHuffmanCodes(root->left, code + "0", huffmanCode);
    generateHuffmanCodes(root->right, code + "1", huffmanCode);
}

// Función para leer un archivo y calcular frecuencias
unordered_map<char, int> computeFrequencies(const string& filename) {
    unordered_map<char, int> freqMap;
    int fd = open(filename.c_str(), O_RDONLY);
    if (fd < 0) {
        perror("Error abriendo archivo");
        exit(1);
    }
    
    char buffer[1024];
    ssize_t bytesRead;
    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < bytesRead; i++) {
            freqMap[buffer[i]]++;
        }
    }
    
    close(fd);
    return freqMap;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Uso: " << argv[0] << " --compress <archivo>" << endl;
        return 1;
    }
    
    string option = argv[1];
    string filename = argv[2];
    
    if (option == "--compress") {
        unordered_map<char, int> freqMap = computeFrequencies(filename);
        HuffmanNode* root = buildHuffmanTree(freqMap);
        HuffmanCodeMap huffmanCode;
        generateHuffmanCodes(root, "", huffmanCode);
        
        cout << "Códigos de Huffman generados:" << endl;
        for (auto& pair : huffmanCode) {
            cout << pair.first << " : " << pair.second << endl;
        }
    }
    
    return 0;
}
