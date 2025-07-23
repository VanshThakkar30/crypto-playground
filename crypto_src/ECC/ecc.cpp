// crypto_src/ECC/ecc.cpp
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <emscripten.h>

// Using a small, specific curve for demonstration: y^2 = x^3 + 7 (mod 3851)
typedef long long int ll;
const ll P = 3851; // A prime modulus
const ll A = 0;

struct Point { ll x, y; };

// Modular inverse: (n * x) % P = 1
ll modInverse(ll n) {
    n = (n % P + P) % P; // Ensure n is positive
    for (ll x = 1; x < P; x++)
        if ((n * x) % P == 1)
            return x;
    return 1;
}

// Add two points on the curve
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

// Scalar multiplication: k * P (double-and-add algorithm)
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

extern "C" {
    EMSCRIPTEN_KEEPALIVE const char* generate_ecc_keys() {
        srand(time(NULL));
        Point G = {2, 5};
        ll private_key = (rand() % 200) + 50;
        Point public_key = scalar_mult(private_key, G);
        std::string result = std::to_string(private_key) + "," + std::to_string(public_key.x) + "," + std::to_string(public_key.y);
        char* c_str = (char*)malloc(result.length() + 1);
        strncpy(c_str, result.c_str(), result.length());
        c_str[result.length()] = '\0';
        return c_str;
    }

    EMSCRIPTEN_KEEPALIVE const char* calculate_shared_secret(ll private_key, ll their_pub_x, ll their_pub_y) {
        Point their_pub_key = {their_pub_x, their_pub_y};
        Point shared_secret_point = scalar_mult(private_key, their_pub_key);
        std::string result = std::to_string(shared_secret_point.x);
        char* c_str = (char*)malloc(result.length() + 1);
        strncpy(c_str, result.c_str(), result.length());
        c_str[result.length()] = '\0';
        return c_str;
    }
}