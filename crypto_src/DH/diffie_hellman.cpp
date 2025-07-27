// crypto_src/DH/diffie_hellman.cpp
#include <string>
#include <emscripten.h>

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

extern "C" {
    // Calculates a public key: g^private_key mod p
    EMSCRIPTEN_KEEPALIVE
    ll generate_dh_public_key(ll g, ll p, ll private_key) {
        return power(g, private_key, p);
    }

    // Calculates the shared secret: other_public_key^private_key mod p
    EMSCRIPTEN_KEEPALIVE
    ll calculate_dh_shared_secret(ll other_public_key, ll p, ll private_key) {
        return power(other_public_key, private_key, p);
    }
}