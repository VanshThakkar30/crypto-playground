// app/static/js/main.js

const wasmModules = {};
async function loadWasmModule(name) {
    if (wasmModules[name]) return wasmModules[name];
    try {
        const moduleFactory = (await import(`/static/wasm/${name}.js`)).default;
        const moduleInstance = await moduleFactory();
        console.log(`${name} WASM module loaded.`);
        wasmModules[name] = moduleInstance;
        return moduleInstance;
    } catch (e) {
        console.error(`Failed to load WASM module ${name}:`, e);
        throw e;
    }
}

document.addEventListener('DOMContentLoaded', () => {
    // --- Element References ---
    const cryptoTypeRadios = document.querySelectorAll('input[name="crypto-type"]');
    const symmetricControls = document.getElementById('symmetric-controls');
    const asymmetricControls = document.getElementById('asymmetric-controls');
    const symmetricAlgoSelect = document.getElementById('symmetric-algo-select');
    const asymmetricAlgoSelect = document.getElementById('asymmetric-algo-select');
    
    // Panels
    const standardPanel = document.getElementById('standard-panel');
    const keyPanel = document.getElementById('key-panel');
    const rsaPanel = document.getElementById('rsa-panel');
    const eccPanel = document.getElementById('ecc-panel');
    const actions = document.querySelector('.actions');
    
    // All other element references
    const inputText = document.getElementById('input-text');
    const outputText = document.getElementById('output-text');
    const keyInput = document.getElementById('key-input');
    const encryptBtn = document.getElementById('encrypt-btn');
    const decryptBtn = document.getElementById('decrypt-btn');
    const generateRsaBtn = document.getElementById('generate-rsa-btn');
    const generateEccBtns = document.querySelectorAll('.generate-ecc-btn');
    const calculateSecretBtn = document.getElementById('calculate-secret-btn');
    const rsaN = document.getElementById('rsa-n'), rsaE = document.getElementById('rsa-e'), rsaD = document.getElementById('rsa-d');
    const rsaPublicKey = document.getElementById('rsa-public-key');
    const rsaPrivateKey = document.getElementById('rsa-private-key');

    // --- Helper to get current algorithm ---
    function getCurrentAlgorithm() {
        const type = document.querySelector('input[name="crypto-type"]:checked').value;
        if (type === 'symmetric') {
            return symmetricAlgoSelect.value;
        } else {
            return asymmetricAlgoSelect.value;
        }
    }

    // --- UI Logic ---
    function updatePanels() {
        const type = document.querySelector('input[name="crypto-type"]:checked').value;
        const algorithm = getCurrentAlgorithm();
        
        console.log(`UI Update for: ${type} -> ${algorithm}`);
        inputText.value = ''; outputText.value = '';

        // Hide all major panels first
        standardPanel.style.display = 'none';
        rsaPanel.style.display = 'none';
        eccPanel.style.display = 'none';
        actions.style.display = 'flex';

        if (type === 'symmetric') {
            standardPanel.style.display = 'block';
            keyPanel.style.display = 'block';
            // Clear RSA fields when switching away
            rsaN.value = ''; rsaE.value = ''; rsaD.value = '';
            rsaPublicKey.value = ''; rsaPrivateKey.value = '';
        } else { // Asymmetric
            if (algorithm === 'rsa') {
                standardPanel.style.display = 'block';
                keyPanel.style.display = 'none';
                rsaPanel.style.display = 'block';
            } else if (algorithm === 'ecc') {
                eccPanel.style.display = 'flex';
                actions.style.display = 'none';
            }
        }
    }
    
    // --- Crypto and Event Handler Functions ---
    async function handleCryptoAction(action) {
        const algorithm = getCurrentAlgorithm();
        const text = inputText.value;
        const key = keyInput.value;

        if (!text && (algorithm !== 'rsa' || algorithm !== 'ecc')) {
             alert('Please provide input text.');
             return;
        }

        try {
            const Module = await loadWasmModule(algorithm);
            let result = '';

            if (algorithm === 'rsa') {
                if (!text) { alert('Please provide input text.'); return; }
                const n = rsaN.value;
                const e = rsaE.value;
                const d = rsaD.value;
                if (!n || !e || (action === 'decrypt' && !d)) {
                    alert('For RSA, please generate or provide n, e, and d values.');
                    return;
                }
                const c_process_rsa = Module.cwrap('process_rsa', 'string', ['string', 'number', 'number']);
                if (action === 'encrypt') result = c_process_rsa(text, BigInt(n), -BigInt(e));
                else result = c_process_rsa(text, BigInt(n), BigInt(d));

            } else if (algorithm === 'railfence') {
                if (!key) { alert('Please provide a key.'); return; }
                const c_encrypt = Module.cwrap('encrypt', 'string', ['string', 'number']);
                const c_decrypt = Module.cwrap('decrypt', 'string', ['string', 'number']);
                result = (action === 'encrypt') ? c_encrypt(text, parseInt(key)) : c_decrypt(text, parseInt(key));

            } else if (algorithm === 'vigenere' || algorithm === 'playfair') {
                if (!key) { alert('Please provide a key.'); return; }
                const c_encrypt = Module.cwrap('encrypt', 'string', ['string', 'string']);
                const c_decrypt = Module.cwrap('decrypt', 'string', ['string', 'string']);
                result = (action === 'encrypt') ? c_encrypt(text, key) : c_decrypt(text, key);

            } else if (algorithm === 'aes' || algorithm === 'des') {
                if (!key) { alert('Please provide a key.'); return; }
                const blockSize = (algorithm === 'aes') ? 16 : 8;
                const c_process = Module.cwrap(`process_${algorithm}`, null, ['number', 'number', 'number', 'number', 'boolean']);
                const encoder = new TextEncoder();
                const decoder = new TextDecoder();
                const keyBytes = encoder.encode(key.padEnd(blockSize, '\0')).slice(0, blockSize);
                let dataBytes;

                if (action === 'encrypt') {
                    const originalBytes = encoder.encode(text);
                    const paddingValue = blockSize - (originalBytes.length % blockSize);
                    dataBytes = new Uint8Array(originalBytes.length + paddingValue);
                    dataBytes.set(originalBytes);
                    dataBytes.fill(paddingValue, originalBytes.length);
                } else {
                    try {
                        dataBytes = Uint8Array.from(atob(text), c => c.charCodeAt(0));
                        if (dataBytes.length % blockSize !== 0) {
                            alert(`Invalid ciphertext. Length must be a multiple of ${blockSize}.`); return;
                        }
                    } catch (e) {
                        alert('Invalid Base64 input for decryption.'); return;
                    }
                }
                const dataPtr = Module._malloc(dataBytes.length);
                const keyPtr = Module._malloc(keyBytes.length);
                const outputPtr = Module._malloc(dataBytes.length);
                Module.HEAPU8.set(dataBytes, dataPtr);
                Module.HEAPU8.set(keyBytes, keyPtr);
                c_process(dataPtr, dataBytes.length, keyPtr, outputPtr, (action === 'encrypt'));
                let resultBytes = Module.HEAPU8.slice(outputPtr, outputPtr + dataBytes.length);
                Module._free(dataPtr); Module._free(keyPtr); Module._free(outputPtr);

                if (action === 'encrypt') {
                    result = btoa(String.fromCharCode.apply(null, resultBytes));
                } else {
                    const paddingValue = resultBytes[resultBytes.length - 1];
                    if (paddingValue > 0 && paddingValue <= blockSize) {
                       resultBytes = resultBytes.slice(0, resultBytes.length - paddingValue);
                    }
                    result = decoder.decode(resultBytes);
                }
            }
            outputText.value = result;
        } catch (e) {
            console.error("Error during crypto action:", e);
            alert(`An error occurred while processing '${algorithm}'. Check console.`);
        }
    }
    
    async function generateRsaKeys() {
        try {
            const Module = await loadWasmModule('rsa');
            const c_generate_keys = Module.cwrap('generate_keys', 'string', []);
            const keys = c_generate_keys().split(',');
            const n_val = keys[0], e_val = keys[1], d_val = keys[2];
            rsaN.value = n_val;
            rsaE.value = e_val;
            rsaD.value = d_val;
            rsaPublicKey.value = `(${e_val}, ${n_val})`;
            rsaPrivateKey.value = `(${d_val}, ${n_val})`;
        } catch (e) {
             console.error("Error generating RSA keys:", e);
             alert('Failed to generate RSA keys. Check console.');
        }
    }
    
    async function generateEccKeys(party) {
        try {
            const Module = await loadWasmModule('ecc');
            const c_generate_keys = Module.cwrap('generate_ecc_keys', 'string', []);
            const keys = c_generate_keys().split(',');
            document.getElementById(`ecc-priv-${party}`).value = keys[0];
            document.getElementById(`ecc-pub-${party}`).value = `(${keys[1]}, ${keys[2]})`;
        } catch(e) { console.error("Error generating ECC keys:", e); }
    }
    
    async function calculateSharedSecret() {
        try {
            const Module = await loadWasmModule('ecc');
            const c_calculate_secret = Module.cwrap('calculate_shared_secret', 'string', ['number', 'number', 'number']);
            const privA = BigInt(document.getElementById('ecc-priv-a').value);
            const pubA_str = document.getElementById('ecc-pub-a').value.replace(/[() ]/g, '').split(',');
            const privB = BigInt(document.getElementById('ecc-priv-b').value);
            const pubB_str = document.getElementById('ecc-pub-b').value.replace(/[() ]/g, '').split(',');
            if (!privA || !privB || pubA_str.length < 2 || pubB_str.length < 2) {
                alert("Please generate keys for both parties first."); return;
            }
            const secretA = c_calculate_secret(privA, BigInt(pubB_str[0]), BigInt(pubB_str[1]));
            document.getElementById('ecc-secret-a').value = secretA;
            const secretB = c_calculate_secret(privB, BigInt(pubA_str[0]), BigInt(pubA_str[1]));
            document.getElementById('ecc-secret-b').value = secretB;
        } catch(e) { console.error("Error calculating shared secret:", e); }
    }


    // --- Event Listeners ---
    cryptoTypeRadios.forEach(radio => radio.addEventListener('change', () => {
        const type = document.querySelector('input[name="crypto-type"]:checked').value;
        if (type === 'symmetric') {
            symmetricControls.style.display = 'block';
            asymmetricControls.style.display = 'none';
        } else {
            symmetricControls.style.display = 'none';
            asymmetricControls.style.display = 'block';
        }
        updatePanels();
        loadWasmModule(getCurrentAlgorithm()).catch(err => console.error(err));
    }));

    symmetricAlgoSelect.addEventListener('change', () => {
        updatePanels();
        loadWasmModule(getCurrentAlgorithm()).catch(err => console.error(err));
    });

    asymmetricAlgoSelect.addEventListener('change', () => {
        updatePanels();
        loadWasmModule(getCurrentAlgorithm()).catch(err => console.error(err));
    });

    encryptBtn.addEventListener('click', () => handleCryptoAction('encrypt'));
    decryptBtn.addEventListener('click', () => handleCryptoAction('decrypt'));
    generateRsaBtn.addEventListener('click', generateRsaKeys);
    generateEccBtns.forEach(btn => btn.addEventListener('click', (e) => generateEccKeys(e.target.dataset.party)));
    calculateSecretBtn.addEventListener('click', calculateSharedSecret);

    // --- Initial Page Setup ---
    updatePanels();
    loadWasmModule(getCurrentAlgorithm()).catch(err => console.error(err));
});