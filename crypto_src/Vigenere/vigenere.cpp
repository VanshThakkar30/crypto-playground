// crypto_src/Vigenere/vigenere.cpp
#include <string>
#include <vector>
#include <cctype>   // for toupper, isalpha, isupper
#include <cstring>  // for strncpy
#include <cstdlib>  // for malloc
#include <emscripten.h>

std::string process_vigenere(const std::string& text, const std::string& key, bool encrypt) {
    std::string result = "";
    std::string processed_key = "";

    // Sanitize key: make it uppercase and only alphabetic characters
    for (char k : key) {
        if (isalpha(k)) {
            processed_key += toupper(k);
        }
    }
    if (processed_key.empty()) return "INVALID KEY";

    int key_index = 0;
    for (char t : text) {
        if (isalpha(t)) {
            char base = isupper(t) ? 'A' : 'a';
            char key_char = processed_key[key_index % processed_key.length()];
            int shift = key_char - 'A';

            if (!encrypt) {
                shift = -shift;
            }

            char processed_char = (t - base + shift + 26) % 26 + base;
            result += processed_char;
            key_index++;
        } else {
            // Keep non-alphabetic characters as they are
            result += t;
        }
    }
    return result;
}

extern "C" {

EMSCRIPTEN_KEEPALIVE
const char* encrypt(const char* text, const char* key) {
    std::string result = process_vigenere(text, key, true);
    char* return_string = (char*)malloc(result.length() + 1);
    // Use strncpy for safety
    strncpy(return_string, result.c_str(), result.length());
    return_string[result.length()] = '\0'; // Ensure null termination
    return return_string;
}

EMSCRIPTEN_KEEPALIVE
const char* decrypt(const char* text, const char* key) {
    std::string result = process_vigenere(text, key, false);
    char* return_string = (char*)malloc(result.length() + 1);
    // Use strncpy for safety
    strncpy(return_string, result.c_str(), result.length());
    return_string[result.length()] = '\0'; // Ensure null termination
    return return_string;
}

} // extern "C"