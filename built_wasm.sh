#!/bin/bash

# This script compiles all C++ source files into their WebAssembly modules.

echo "--- Building Rail Fence ---"
emcc crypto_src/RailFence/railfence.cpp -o app/static/wasm/railfence.js -s WASM=1 -sMODULARIZE=1 -sEXPORT_ES6=1 -s EXPORTED_FUNCTIONS='["_encrypt", "_decrypt"]' -s EXPORTED_RUNTIME_METHODS='["cwrap"]'

echo "--- Building AES ---"
emcc crypto_src/AES/aes.cpp -o app/static/wasm/aes.js -s WASM=1 -sMODULARIZE=1 -sEXPORT_ES6=1 -s EXPORTED_FUNCTIONS='["_process_aes", "_malloc", "_free"]' -s EXPORTED_RUNTIME_METHODS='["cwrap", "HEAPU8"]'

echo "--- Building DES ---"
emcc crypto_src/DES/des.cpp -o app/static/wasm/des.js -s WASM=1 -sMODULARIZE=1 -sEXPORT_ES6=1 -s EXPORTED_FUNCTIONS='["_process_des", "_malloc", "_free"]' -s EXPORTED_RUNTIME_METHODS='["cwrap", "HEAPU8"]'

echo "--- Building Vigenere ---"
emcc crypto_src/Vigenere/vigenere.cpp -o app/static/wasm/vigenere.js -s WASM=1 -sMODULARIZE=1 -sEXPORT_ES6=1 -s EXPORTED_FUNCTIONS='["_encrypt", "_decrypt", "_malloc", "_free"]' -s EXPORTED_RUNTIME_METHODS='["cwrap"]'

echo "--- Building Playfair ---"
emcc crypto_src/Playfair/playfair.cpp -o app/static/wasm/playfair.js -s WASM=1 -sMODULARIZE=1 -sEXPORT_ES6=1 -s EXPORTED_FUNCTIONS='["_encrypt", "_decrypt", "_malloc", "_free"]' -s EXPORTED_RUNTIME_METHODS='["cwrap"]'

echo "--- Building RSA ---"
emcc crypto_src/RSA/rsa.cpp -o app/static/wasm/rsa.js -s WASM=1 -sMODULARIZE=1 -sEXPORT_ES6=1 -s EXPORTED_FUNCTIONS='["_generate_keys", "_process_rsa", "_malloc", "_free"]' -s EXPORTED_RUNTIME_METHODS='["cwrap"]'

echo "--- Building ECC ---"
emcc crypto_src/ECC/ecc.cpp -o app/static/wasm/ecc.js -s WASM=1 -sMODULARIZE=1 -sEXPORT_ES6=1 -s EXPORTED_FUNCTIONS='["_generate_ecc_keys", "_calculate_shared_secret", "_malloc", "_free"]' -s EXPORTED_RUNTIME_METHODS='["cwrap"]'

echo "--- All modules built successfully! ---"