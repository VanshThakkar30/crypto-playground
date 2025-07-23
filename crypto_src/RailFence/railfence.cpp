// crypto_src/RailFence/railfence.cpp
#include <string>
#include <vector>
#include <cstring> // For strcpy
#include <cstdlib> // For malloc and free
#include <emscripten.h>

extern "C" {

// A new function to free the memory we allocate
EMSCRIPTEN_KEEPALIVE
void free_memory(void* ptr) {
    free(ptr);
}

EMSCRIPTEN_KEEPALIVE
const char* encrypt(const char* raw_text, int key) {
    std::string text(raw_text);
    std::string result;

    if (key <= 1) {
        result = text;
    } else {
        std::vector<std::string> rail(key);
        int row = 0;
        bool down = true;

        for (char c : text) {
            rail[row] += c;
            if (row == key - 1) down = false;
            else if (row == 0) down = true;
            down ? row++ : row--;
        }

        for (int i = 0; i < key; i++) {
            result += rail[i];
        }
    }

    // Allocate memory with malloc and copy the result
    char* return_string = (char*)malloc(result.length() + 1);
    strcpy(return_string, result.c_str());
    return return_string;
}

EMSCRIPTEN_KEEPALIVE
const char* decrypt(const char* cipher_text, int key) {
    std::string cipher(cipher_text);
    std::string result = "";
    int len = cipher.length();

    if (key <= 1) {
        result = cipher;
    } else {
        std::vector<std::string> rail(key, "");
        int row = 0;
        bool down = true;

        for (int i = 0; i < len; i++) {
            rail[row] += ' ';
            if (row == key - 1) down = false;
            else if (row == 0) down = true;
            down ? row++ : row--;
        }

        int index = 0;
        for (int i = 0; i < key; i++) {
            for (size_t j = 0; j < rail[i].length() && index < len; j++) {
                rail[i][j] = cipher[index++];
            }
        }

        row = 0;
        down = true;
        for (int i = 0; i < len; i++) {
            result += rail[row][0];
            rail[row].erase(0, 1);
            if (row == key - 1) down = false;
            else if (row == 0) down = true;
            down ? row++ : row--;
        }
    }

    char* return_string = (char*)malloc(result.length() + 1);
    strcpy(return_string, result.c_str());
    return return_string;
}

} // extern "C"