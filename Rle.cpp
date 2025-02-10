#include <sstream>  
#include "Rle.h"  // Incluir el archivo de encabezado  

std::string rleCompress(const std::string &input) {  
    std::ostringstream compressed;  // Asegúrate de utilizar std::  
    for (size_t i = 0; i < input.length(); ++i) {  
        char currentChar = input[i];  
        int count = 1;  

        while (i + 1 < input.length() && input[i + 1] == currentChar) {  
            ++count;  
            ++i;  
        }  

        // Agregar la cantidad y el carácter  
        compressed << count << currentChar;  
    }  
    return compressed.str();  
}