// crypto_src/RSA/rsa.cpp
#include <string>
#include <vector>
#include <numeric>
#include <cstdlib> // For rand()
#include <emscripten.h>
#include<time.h>

// Type for large integers
typedef long long int ll;

// Modular exponentiation: (base^exp) % mod
ll power(ll base, ll exp, ll mod) {
    ll res = 1;
    base %= mod;
    while (exp > 0) {
        if (exp % 2 == 1) res = (res * base) % mod;
        base = (base * base) % mod;
        exp /= 2;
    }
    return res;
}

// Miller-Rabin primality test (for key generation)
bool is_prime(ll n, int k = 5) {
    if (n <= 1 || n == 4) return false;
    if (n <= 3) return true;
    ll d = n - 1;
    while (d % 2 == 0) d /= 2;
    for (int i = 0; i < k; i++) {
        ll a = 2 + rand() % (n - 4);
        ll x = power(a, d, n);
        if (x == 1 || x == n - 1) continue;
        bool prime = false;
        while (d != n - 1) {
            x = (x * x) % n;
            d *= 2;
            if (x == 1) return false;
            if (x == n - 1) {
                prime = true;
                break;
            }
        }
        if (!prime) return false;
    }
    return true;
}

// Modular inverse: (a * x) % m = 1
ll modInverse(ll a, ll m) {
    for (ll x = 1; x < m; x++)
        if (((a % m) * (x % m)) % m == 1)
            return x;
    return 1;
}

extern "C" {
    EMSCRIPTEN_KEEPALIVE const char* generate_keys() {
        srand(time(0));
        ll p = 0, q = 0;
        // Find two small prime numbers
        while (!is_prime(p)) p = (rand() % 100) + 50;
        while (!is_prime(q) || p == q) q = (rand() % 100) + 50;

        ll n = p * q;
        ll phi = (p - 1) * (q - 1);
        ll e = 2;
        while (e < phi) {
            if (std::gcd(e, phi) == 1) break;
            e++;
        }
        ll d = modInverse(e, phi);
        
        std::string result = std::to_string(n) + "," + std::to_string(e) + "," + std::to_string(d);
        char* return_string = (char*)malloc(result.length() + 1);
        strncpy(return_string, result.c_str(), result.length());
        return_string[result.length()] = '\0';
        return return_string;
    }

    EMSCRIPTEN_KEEPALIVE const char* process_rsa(const char* text, ll n, ll key) {
        std::string input(text);
        std::string result = "";
        bool first = true;

        if (key > 0) { // Encrypting or Decrypting a numeric string
             std::string current_num_str;
             for(char c : input) {
                 if (c == ',') {
                     ll num = std::stoll(current_num_str);
                     result += (char)power(num, key, n);
                     current_num_str = "";
                 } else {
                     current_num_str += c;
                 }
             }
             if (!current_num_str.empty()) {
                  ll num = std::stoll(current_num_str);
                  result += (char)power(num, key, n);
             }
        } else { // Encrypting a plain text string
            for (char c : input) {
                if (!first) result += ",";
                result += std::to_string(power(c, -key, n));
                first = false;
            }
        }

        char* return_string = (char*)malloc(result.length() + 1);
        strncpy(return_string, result.c_str(), result.length());
        return_string[result.length()] = '\0';
        return return_string;
    }
}