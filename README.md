# Compresor Huffman

Este repositorio contiene un programa en C++ que implementa un compresor y descompresor de archivos utilizando el algoritmo de codificación Huffman. El programa realiza la compresión y descompresión de archivos mediante la construcción de un árbol de Huffman basado en la frecuencia de aparición de cada carácter.

---

## Descripción General

El programa se encarga de:

1. **Compresión del archivo:**
   - **Lectura del archivo:** Se lee el archivo de entrada para contar la frecuencia de cada carácter.
   - **Construcción del árbol de Huffman:** Utilizando una cola de prioridad (min-heap) se construye un árbol donde cada nodo representa un carácter y su frecuencia.
   - **Generación de códigos binarios:** Se asigna un código binario único a cada carácter mediante una función recursiva que recorre el árbol. Se asume que los caracteres que se encuentran en hojas del árbol tienen un código asignado.
   - **Escritura del encabezado:** Se escribe en el archivo comprimido un encabezado que contiene el número de caracteres únicos y, para cada uno, su frecuencia.
   - **Codificación y conversión a bytes:** El contenido del archivo se codifica utilizando los códigos generados, se forma una cadena de bits, se agrupan en bytes (completando con ceros si es necesario) y se escribe el resultado en el archivo comprimido.

2. **Descompresión del archivo:**
   - **Lectura del encabezado:** Se lee el encabezado del archivo comprimido para obtener la tabla de frecuencias de los caracteres.
   - **Reconstrucción del árbol:** A partir de la tabla de frecuencias se reconstruye el árbol de Huffman.
   - **Decodificación de la secuencia de bits:** Se lee la secuencia de bytes del archivo comprimido, se convierte cada byte en una cadena de 8 bits y se recorre el árbol para recuperar cada carácter original.
   - **Escritura del archivo original:** Se escribe el contenido decodificado en un archivo de salida.

---

## Flujo de Ejecución en Terminal Linux

1. **Compilación del programa:**
   - Descarga o clona el repositorio.
   - Desde la terminal en la raíz del proyecto, compila el código utilizando `g++`:
     ```bash
     g++ main.cpp -o programa
     ```
     Esto generará un ejecutable llamado `programa`.

2. **Ejecución del programa:**
   
   El programa admite diversas opciones que se seleccionan mediante argumentos en la terminal:

   - **Mostrar ayuda:**
     ```bash
     ./programa -h
     ```
     Muestra un mensaje de ayuda indicando las opciones de uso.

   - **Mostrar la versión:**
     ```bash
     ./programa -v
     ```
     Muestra la versión actual del programa.

   - **Comprimir un archivo:**
     ```bash
     ./programa -c <archivo>
     ```
     Comprime el archivo especificado y genera un nuevo archivo con extensión `.huff`.

   - **Descomprimir un archivo:**
     ```bash
     ./programa -x <archivo>
     ```
     Descomprime el archivo especificado (previamente comprimido) y genera un archivo original con la extensión `.orig`.

3. **Ejemplo de uso:**
   - Para comprimir un archivo llamado `texto.txt`:
     ```bash
     ./programa -c texto.txt
     ```
     Se creará un archivo `texto.txt.huff` con el contenido comprimido.

   - Para descomprimir el archivo comprimido:
     ```bash
     ./programa -x texto.txt.huff
     ```
     Se generará un archivo `texto.txt.huff.orig` que contendrá el texto descomprimido.

---

## Licencia

Este proyecto se distribuye bajo la licencia MIT.

---
