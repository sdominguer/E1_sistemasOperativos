#include <iostream>
#include <unordered_map>
#include <queue>
#include <vector>
#include <bitset>
#include <fcntl.h>
#include <unistd.h>
#include <cstdint>

using namespace std;

struct Node {
    char ch;
    int freq;
    Node *left, *right;
    Node(char c, int f) : ch(c), freq(f), left(nullptr), right(nullptr) {}
};

struct Compare {
    bool operator()(Node* a, Node* b) { return a->freq > b->freq; }
};

void buildHuffmanCodes(Node* root, string code, unordered_map<char, string>& huffmanCodes) {
    if (!root) return;
    if (!root->left && !root->right) huffmanCodes[root->ch] = code;
    buildHuffmanCodes(root->left, code + "0", huffmanCodes);
    buildHuffmanCodes(root->right, code + "1", huffmanCodes);
}

void compressFile(const string& inputFile, const string& outputFile) {
    int fd = open(inputFile.c_str(), O_RDONLY);
    if (fd == -1) { perror("Error opening file"); return; }

    unordered_map<char, int> freq;
    char buffer[1024];
    ssize_t bytesRead;
    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < bytesRead; i++) freq[buffer[i]]++;
    }
    close(fd);

    priority_queue<Node*, vector<Node*>, Compare> pq;
    for (auto& pair : freq) pq.push(new Node(pair.first, pair.second));
    while (pq.size() > 1) {
        Node* left = pq.top(); pq.pop();
        Node* right = pq.top(); pq.pop();
        Node* parent = new Node('\0', left->freq + right->freq);
        parent->left = left; parent->right = right;
        pq.push(parent);
    }
    Node* root = pq.top();

    unordered_map<char, string> huffmanCodes;
    buildHuffmanCodes(root, "", huffmanCodes);

    fd = open(inputFile.c_str(), O_RDONLY);
    if (fd == -1) { perror("Error reopening file"); return; }
    int outFd = open(outputFile.c_str(), O_WRONLY | O_CREAT, 0644);
    if (outFd == -1) { perror("Error creating output file"); return; }

    string encodedText = "";
    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < bytesRead; i++) encodedText += huffmanCodes[buffer[i]];
    }
    close(fd);

    string byteString = "";
    vector<uint8_t> byteData;
    for (char bit : encodedText) {
        byteString += bit;
        if (byteString.size() == 8) {
            byteData.push_back(bitset<8>(byteString).to_ulong());
            byteString = "";
        }
    }
    if (!byteString.empty()) {
        while (byteString.size() < 8) byteString += "0";
        byteData.push_back(bitset<8>(byteString).to_ulong());
    }

    write(outFd, byteData.data(), byteData.size());
    close(outFd);

    cout << "Archivo comprimido guardado como: " << outputFile << endl;
}

void showHelp() {
    cout << "Uso: ./programa <opcion> <archivo>\n";
    cout << "Opciones:\n";
    cout << "  -h, --help         Mostrar este mensaje de ayuda\n";
    cout << "  -v, --version      Mostrar la versión del programa\n";
    cout << "  -c, --compress     Comprimir el archivo\n";
    cout << "  -x, --decompress   Descomprimir el archivo\n";
}

void showVersion() {
    cout << "Compresor Huffman - Versión 1.0\n";
}

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
    } else {
        cout << "Opción no reconocida. Usa -h para ayuda.\n";
        return 1;
    }
    return 0;
}
