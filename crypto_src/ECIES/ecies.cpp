// crypto_src/ECIES/ecies.cpp
#include <string>
#include <vector>
#include <sstream>
#include <emscripten.h>

// --- ECC Math (copied from ecc.cpp for simplicity) ---
typedef long long int ll;
const ll P = 3851;
const ll A = 0;
struct Point { ll x, y; };
ll modInverse(ll n) {
    n = (n % P + P) % P; // Ensure n is positive
    for (ll x = 1; x < P; x++)
        if ((n * x) % P == 1)
            return x;
    return 1;
}
Point point_add(Point p1, Point p2) {
    ll m;
    if (p1.x == p2.x && p1.y == p2.y) { // Point doubling
        ll numerator = (3 * p1.x * p1.x + A);
        ll denominator = (2 * p1.y);
        m = (numerator * modInverse(denominator));
    } else { // Point addition
        ll numerator = (p2.y - p1.y);
        ll denominator = (p2.x - p1.x);
        m = (numerator * modInverse(denominator));
    }
    
    // Perform all calculations with the modulus
    m = (m % P + P) % P;
    ll x3 = (m * m - p1.x - p2.x);
    x3 = (x3 % P + P) % P;
    ll y3 = (m * (p1.x - x3) - p1.y);
    y3 = (y3 % P + P) % P;
    return {x3, y3};
}
Point scalar_mult(ll k, Point p) {
    Point res = {0, 0}; // Point at infinity
    Point addend = p;
    while (k > 0) {
        if (k & 1) { // If k is odd
             if (res.x == 0 && res.y == 0) res = addend;
             else res = point_add(res, addend);
        }
        addend = point_add(addend, addend);
        k >>= 1; // k = k / 2
    }
    return res;
}

// --- AES Logic (copied from aes.cpp for simplicity) ---
void placeholder_aes_process_block(const uint8_t* input, const uint8_t* key, uint8_t* output) {
    for (int i = 0; i < 16; ++i) output[i] = input[i] ^ key[i];
}

std::vector<uint8_t> process_aes_ecb(const std::vector<uint8_t>& data, const std::vector<uint8_t>& key, bool encrypt) {
    std::vector<uint8_t> padded_data = data;
    if (encrypt) {
        size_t padding = 16 - (data.size() % 16);
        for(size_t i = 0; i < padding; ++i) padded_data.push_back(padding);
    }
    std::vector<uint8_t> result(padded_data.size());
    for(size_t i = 0; i < padded_data.size(); i += 16) {
        placeholder_aes_process_block(padded_data.data() + i, key.data(), result.data() + i);
    }
    if (!encrypt) {
        size_t padding = result.back();
        if (padding > 0 && padding <= 16) result.resize(result.size() - padding);
    }
    return result;
}


extern "C" {
    EMSCRIPTEN_KEEPALIVE
    const char* encrypt_ecies(const char* plaintext, ll pub_x, ll pub_y) {
        Point recipient_pub_key = {pub_x, pub_y};
        Point G = {2, 5};
        srand(time(NULL));
        ll ephemeral_priv_key = (rand() % 200) + 50;
        Point ephemeral_pub_key = scalar_mult(ephemeral_priv_key, G);
        Point shared_point = scalar_mult(ephemeral_priv_key, recipient_pub_key);
        ll shared_secret_x = shared_point.x;
        std::vector<uint8_t> aes_key;
        for(int i = 0; i < 16; ++i) aes_key.push_back((shared_secret_x >> (i % 8)) & 0xFF);
        std::string text_str(plaintext);
        std::vector<uint8_t> plaintext_bytes(text_str.begin(), text_str.end());
        std::vector<uint8_t> ciphertext_bytes = process_aes_ecb(plaintext_bytes, aes_key, true);
        std::string result = std::to_string(ephemeral_pub_key.x) + "," + std::to_string(ephemeral_pub_key.y) + ",";
        for(size_t i=0; i<ciphertext_bytes.size(); ++i) {
            result += std::to_string(ciphertext_bytes[i]) + (i == ciphertext_bytes.size() - 1 ? "" : " ");
        }
        char* c_str = (char*)malloc(result.length() + 1);
        strncpy(c_str, result.c_str(), result.length());
        c_str[result.length()] = '\0';
        return c_str;
    }
    
    // --- NEW DECRYPTION FUNCTION ---
    EMSCRIPTEN_KEEPALIVE
    const char* decrypt_ecies(const char* ciphertext, ll private_key) {
        std::string c_text(ciphertext);
        std::stringstream ss(c_text);
        std::string part;
        
        // 1. Unpack the ciphertext: R.x, R.y, and encrypted bytes
        getline(ss, part, ',');
        ll R_x = std::stoll(part);
        getline(ss, part, ',');
        ll R_y = std::stoll(part);
        
        std::vector<uint8_t> ciphertext_bytes;
        int byte_val;
        while(ss >> byte_val) {
            ciphertext_bytes.push_back(byte_val);
        }
        Point ephemeral_pub_key = {R_x, R_y};

        // 2. Derive shared secret
        Point shared_point = scalar_mult(private_key, ephemeral_pub_key);
        ll shared_secret_x = shared_point.x;

        // 3. Use shared secret to derive the same AES key
        std::vector<uint8_t> aes_key;
        for(int i = 0; i < 16; ++i) aes_key.push_back((shared_secret_x >> (i % 8)) & 0xFF);

        // 4. Decrypt message with AES
        std::vector<uint8_t> decrypted_bytes = process_aes_ecb(ciphertext_bytes, aes_key, false);
        std::string result(decrypted_bytes.begin(), decrypted_bytes.end());
        
        char* c_str = (char*)malloc(result.length() + 1);
        strncpy(c_str, result.c_str(), result.length());
        c_str[result.length()] = '\0';
        return c_str;
    }
}