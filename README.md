# Compresor Huffman

Este repositorio contiene un programa en C++ que implementa un compresor y descompresor de archivos utilizando el algoritmo de codificación Huffman. El programa realiza la compresión y descompresión de archivos mediante la construcción de un árbol de Huffman basado en la frecuencia de aparición de cada carácter.

---

## Descripción General

El programa se encarga de:

1. **Compresión del archivo:**
   - **Lectura del archivo:** Se lee el archivo de entrada para contar la frecuencia de cada carácter.
   - **Ordenamiento determinista de frecuencias:** Se convierte la tabla de frecuencias en un vector ordenado primero por frecuencia y luego por carácter, garantizando un orden consistente.
   - **Construcción del árbol de Huffman:** Utilizando el vector ordenado de frecuencias y una cola de prioridad (min-heap) se construye un árbol donde cada nodo representa un carácter y su frecuencia.
   - **Generación de códigos binarios:** Se asigna un código binario único a cada carácter mediante una función recursiva que recorre el árbol. Para el caso especial de archivos con un único carácter, se asigna el código "0".
   - **Escritura del encabezado:** Se escribe en el archivo comprimido un encabezado que contiene el número de caracteres únicos y la tabla de frecuencias en el mismo orden determinista.
   - **Codificación y conversión a bytes:** El contenido del archivo se codifica utilizando los códigos generados, se forma una cadena de bits, se agrupan en bytes (completando con ceros si es necesario) y se escribe el resultado en el archivo comprimido.

2. **Descompresión del archivo:**
   - **Lectura del encabezado:** Se lee el encabezado del archivo comprimido para obtener la tabla de frecuencias de los caracteres.
   - **Ordenamiento de frecuencias:** Se ordena la tabla de frecuencias leída usando el mismo criterio determinista que en la compresión.
   - **Reconstrucción del árbol:** A partir del vector ordenado de frecuencias se reconstruye el árbol de Huffman de manera idéntica a la compresión.
   - **Decodificación de la secuencia de bits:** Se lee la secuencia de bytes del archivo comprimido, se convierte cada byte en una cadena de 8 bits y se recorre el árbol para recuperar cada carácter original.
   - **Escritura del archivo original:** Se escribe el contenido decodificado en un archivo de salida.

---

## Características Importantes

### Enfoque Determinista para la Tabla de Frecuencias

Para garantizar una compresión y descompresión consistente, se implementó un enfoque determinista usando vectores ordenados:

1. Las frecuencias se almacenan inicialmente en un `unordered_map<char, int>`.
2. Se convierten a un `vector<pair<char, int>>` para su ordenamiento.
3. Se ordenan usando dos criterios:
   - Primero por frecuencia (menor a mayor)
   - En caso de empate en frecuencia, por valor del carácter (menor a mayor)
4. Este orden se mantiene tanto en la compresión como en la descompresión.

Este enfoque resuelve problemas de inconsistencia que pueden surgir cuando diferentes ejecuciones producen árboles de Huffman distintos debido al orden no determinista de las estructuras de datos.

### Manejo de Casos Especiales

- **Archivos con un único carácter:** Se asigna el código "0" al único carácter presente.
- **Bits restantes:** Al convertir la cadena de bits a bytes, se rellenan con ceros los bits faltantes del último byte.

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

## Compatibilidad

El código está diseñado para ser compatible con versiones anteriores de C++, evitando características modernas como:
- Expresiones lambda
- Bucles range-based for
- Inicializadores de lista
- Palabra clave `auto`

---

## Licencia

Este proyecto se distribuye bajo la licencia MIT.

---
