// crypto_src/Playfair/playfair.cpp
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>
#include <emscripten.h>

typedef std::vector<std::vector<char>> char_matrix;
typedef std::pair<int, int> pos;

// Generate the 5x5 key square
char_matrix generate_key_square(const std::string& key) {
    char_matrix square(5, std::vector<char>(5, 0));
    std::string temp_key = "";
    std::vector<bool> present(26, false);

    // Add key characters
    for (char k : key) {
        if (!isalpha(k)) continue;
        char c = toupper(k);
        if (c == 'J') c = 'I';
        if (!present[c - 'A']) {
            temp_key += c;
            present[c - 'A'] = true;
        }
    }

    // Add remaining alphabet characters
    for (char c = 'A'; c <= 'Z'; ++c) {
        if (c == 'J') continue;
        if (!present[c - 'A']) {
            temp_key += c;
        }
    }

    // Populate the square
    for (int i = 0; i < 25; ++i) {
        square[i / 5][i % 5] = temp_key[i];
    }
    return square;
}

// Find character position in the square
pos find_pos(const char_matrix& square, char c) {
    if (c == 'J') c = 'I';
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            if (square[i][j] == c) {
                return {i, j};
            }
        }
    }
    return {-1, -1}; // Should not happen with sanitized text
}

std::string process_playfair(const std::string& text, const std::string& key, bool encrypt) {
    char_matrix square = generate_key_square(key);
    std::string prepared_text = "";
    
    // Prepare plaintext
    for (char t : text) {
        if (isalpha(t)) {
            char c = toupper(t);
            if (c == 'J') c = 'I';
            prepared_text += c;
        }
    }

    // Insert 'X' between double letters
    for (size_t i = 0; i < prepared_text.length(); i += 2) {
        if (i + 1 < prepared_text.length() && prepared_text[i] == prepared_text[i + 1]) {
            prepared_text.insert(i + 1, "X");
        }
    }

    // If odd length, append 'X'
    if (prepared_text.length() % 2 != 0) {
        prepared_text += 'X';
    }

    // Process digraphs
    std::string result = "";
    int dir = encrypt ? 1 : -1;

    for (size_t i = 0; i < prepared_text.length(); i += 2) {
        pos p1 = find_pos(square, prepared_text[i]);
        pos p2 = find_pos(square, prepared_text[i + 1]);

        if (p1.first == p2.first) { // Same row
            result += square[p1.first][(p1.second + dir + 5) % 5];
            result += square[p2.first][(p2.second + dir + 5) % 5];
        } else if (p1.second == p2.second) { // Same column
            result += square[(p1.first + dir + 5) % 5][p1.second];
            result += square[(p2.first + dir + 5) % 5][p2.second];
        } else { // Rectangle
            result += square[p1.first][p2.second];
            result += square[p2.first][p1.second];
        }
    }
    return result;
}

extern "C" {
    EMSCRIPTEN_KEEPALIVE const char* encrypt(const char* text, const char* key) {
        std::string result = process_playfair(text, key, true);
        char* return_string = (char*)malloc(result.length() + 1);
        strncpy(return_string, result.c_str(), result.length());
        return_string[result.length()] = '\0';
        return return_string;
    }
    EMSCRIPTEN_KEEPALIVE const char* decrypt(const char* text, const char* key) {
        std::string result = process_playfair(text, key, false);
        char* return_string = (char*)malloc(result.length() + 1);
        strncpy(return_string, result.c_str(), result.length());
        return_string[result.length()] = '\0';
        return return_string;
    }
}