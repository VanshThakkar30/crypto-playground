// app/static/js/main.js

const wasmModules = {};
async function loadWasmModule(name) {
    if (name === 'ecies' || name === 'ecc_encryption') name = 'ecies';
    if (name === 'ecdh') name = 'ecc';
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
    const keyExchangeControls = document.getElementById('key-exchange-controls');
    const symmetricAlgoSelect = document.getElementById('symmetric-algo-select');
    const asymmetricAlgoSelect = document.getElementById('asymmetric-algo-select');
    const keyExchangeAlgoSelect = document.getElementById('key-exchange-algo-select');
    const visualPanel = document.getElementById('visual-panel'), visualContent = document.getElementById('visual-content');
    const panels = {
        symmetric: document.getElementById('symmetric-panel'),
        rsa: document.getElementById('rsa-panel'),
        ecies: document.getElementById('ecies-panel'),
        dh: document.getElementById('dh-panel'),
        ecdh: document.getElementById('ecdh-panel')
    };
    const keyInputSymm = document.getElementById('key-input-symm');

    // --- Helper to get current algorithm ---
    function getCurrentAlgorithm() {
        const type = document.querySelector('input[name="crypto-type"]:checked').value;
        switch (type) {
            case 'symmetric': return symmetricAlgoSelect.value;
            case 'asymmetric': return asymmetricAlgoSelect.value;
            case 'key-exchange': return keyExchangeAlgoSelect.value;
            default: return null;
        }
    }

    // --- UI Logic ---
    function updatePanels() {
        const type = document.querySelector('input[name="crypto-type"]:checked').value;
        const algorithm = getCurrentAlgorithm();
        console.log(`UI Update for: ${type} -> ${algorithm}`);
        
        // Add loading animation
        document.body.classList.add('loading');
        setTimeout(() => document.body.classList.remove('loading'), 300);
        
        Object.values(panels).forEach(p => p.style.display = 'none');
        visualPanel.style.display = 'none';

        const placeholders = {
            aes: 'Enter a 16-character ASCII key',
            des: 'Enter an 8-character ASCII key',
            vigenere: 'Enter an alphabetic keyword (e.g., LEMON)',
            playfair: 'Enter an alphabetic keyword (e.g., PLAYFAIR)',
            railfence: 'Enter a number for the rails (e.g., 3)'
        };
        keyInputSymm.placeholder = placeholders[algorithm] || 'Enter key...';

        if (type === 'symmetric' || (type === 'asymmetric' && algorithm !== 'ecies')) {
            visualPanel.style.display = 'block';
        }

        if (type === 'symmetric') {
            panels.symmetric.style.display = 'block';
        } else if (type === 'asymmetric') {
            if (algorithm === 'rsa') panels.rsa.style.display = 'block';
            else if (algorithm === 'ecies') panels.ecies.style.display = 'block';
        } else if (type === 'key-exchange') {
            if (algorithm === 'dh') panels.dh.style.display = 'block';
            else if (algorithm === 'ecdh') panels.ecdh.style.display = 'block';
        }
    }
    
    // --- Visualizer ---
    function visualizeRailFence(text, rails) {
        if (!text || rails < 2) { visualContent.innerHTML = ''; return; }
        let matrix = Array(rails).fill(null).map(() => Array(text.length).fill(' '));
        let row = 0, col = 0, dir = -1;
        for (const char of text) {
            matrix[row][col] = char;
            if (row === 0 || row === rails - 1) dir *= -1;
            row += dir;
            col++;
        }
        let visualHtml = '<pre style="color: var(--accent-tertiary); font-size: 1.2rem; line-height: 2;">';
        for (const r of matrix) visualHtml += r.join('').replace(/\s/g, ' ') + '\n';
        visualHtml += '</pre>';
        visualContent.innerHTML = `<h3 style="color: var(--accent-primary); margin-bottom: 1rem;">🚂 Rail Fence Pattern (Rails: ${rails})</h3>${visualHtml}`;
    }

    // --- Crypto Handlers ---
    async function handleCryptoAction(action) {
        const algorithm = getCurrentAlgorithm();
        let text, key, outputTextElem;

        if (panels.symmetric.style.display !== 'none') {
            text = document.getElementById('input-text-symm').value;
            key = document.getElementById('key-input-symm').value;
            outputTextElem = document.getElementById('output-text-symm');
        } else if (panels.rsa.style.display !== 'none') {
            text = document.getElementById('input-text-rsa').value;
            outputTextElem = document.getElementById('output-text-rsa');
        } else if (panels.ecies.style.display !== 'none') {
            text = document.getElementById('input-text-ecies').value;
            outputTextElem = document.getElementById('output-text-ecies');
        } else { return; }

        if (!text) { alert('Please provide input text.'); return; }
        if (action === 'encrypt') visualContent.innerHTML = '';

        try {
            const Module = await loadWasmModule(algorithm);
            let result = '';

            if (algorithm === 'railfence') {
                if (!key) { alert('Please provide a key.'); return; }
                const rails = parseInt(key);
                if (isNaN(rails) || rails < 2) { alert('Invalid key: Rail Fence key must be a number greater than 1.'); return; }
                if (action === 'encrypt') visualizeRailFence(text, rails);
                const c_encrypt = Module.cwrap('encrypt', 'string', ['string', 'number']);
                const c_decrypt = Module.cwrap('decrypt', 'string', ['string', 'number']);
                result = (action === 'encrypt') ? c_encrypt(text, rails) : c_decrypt(text, rails);
            } else if (algorithm === 'vigenere' || algorithm === 'playfair') {
                if (!key) { alert('Please provide a key.'); return; }
                if (!/^[a-zA-Z]+$/.test(key)) { alert('Invalid key: Must be an alphabetic keyword with no spaces or numbers.'); return; }
                const c_encrypt = Module.cwrap('encrypt', 'string', ['string', 'string']);
                const c_decrypt = Module.cwrap('decrypt', 'string', ['string', 'string']);
                result = (action === 'encrypt') ? c_encrypt(text, key) : c_decrypt(text, key);
            } else if (algorithm === 'aes' || algorithm === 'des') {
                if (!key) { alert('Please provide a key.'); return; }
                const blockSize = (algorithm === 'aes') ? 16 : 8;
                if (key.length !== blockSize) { alert(`Invalid key: ${algorithm.toUpperCase()} key must be exactly ${blockSize} characters long.`); return; }
                
                // Both AES and DES now return error codes
                let c_process;
                if (algorithm === 'aes') {
                    c_process = Module.cwrap('process_aes', 'number', ['number', 'number', 'number', 'number', 'boolean']);
                } else {
                    c_process = Module.cwrap('process_des', 'number', ['number', 'number', 'number', 'number', 'boolean']);
                }
                
                const encoder = new TextEncoder(), decoder = new TextDecoder();
                const keyBytes = encoder.encode(key.padEnd(blockSize, '\0')).slice(0, blockSize);
                let dataBytes;
                if (action === 'encrypt') {
                    const originalBytes = encoder.encode(text);
                    const paddingValue = blockSize - (originalBytes.length % blockSize);
                    dataBytes = new Uint8Array(originalBytes.length + paddingValue);
                    dataBytes.set(originalBytes); dataBytes.fill(paddingValue, originalBytes.length);
                } else {
                    try {
                        dataBytes = Uint8Array.from(atob(text), c => c.charCodeAt(0));
                        if (dataBytes.length % blockSize !== 0) { alert(`Invalid ciphertext. Length must be a multiple of ${blockSize}.`); return; }
                    } catch (e) { alert('Invalid Base64 input for decryption.'); return; }
                }
                
                const dataPtr = Module._malloc(dataBytes.length), keyPtr = Module._malloc(keyBytes.length), outputPtr = Module._malloc(dataBytes.length);
                if (!dataPtr || !keyPtr || !outputPtr) {
                    alert('Memory allocation failed. Please try again.');
                    if (dataPtr) Module._free(dataPtr);
                    if (keyPtr) Module._free(keyPtr);
                    if (outputPtr) Module._free(outputPtr);
                    return;
                }
                
                try {
                    // Additional validation
                    if (dataBytes.length === 0) {
                        alert('Invalid input data length.');
                        Module._free(dataPtr); Module._free(keyPtr); Module._free(outputPtr);
            return;
        }

                    Module.HEAPU8.set(dataBytes, dataPtr); 
                    Module.HEAPU8.set(keyBytes, keyPtr);
                    
                    let success = c_process(dataPtr, dataBytes.length, keyPtr, outputPtr, (action === 'encrypt'));
                    
                    if (!success) {
                        alert(`${algorithm.toUpperCase()} processing failed. Please check your input and try again.`);
                        Module._free(dataPtr); Module._free(keyPtr); Module._free(outputPtr);
                return;
            }

                    let resultBytes = Module.HEAPU8.slice(outputPtr, outputPtr + dataBytes.length);
                    
                    // Validate result
                    if (!resultBytes || resultBytes.length === 0) {
                        alert('Invalid output from encryption/decryption.');
                        Module._free(dataPtr); Module._free(keyPtr); Module._free(outputPtr);
                return;
            }
            
                    Module._free(dataPtr); Module._free(keyPtr); Module._free(outputPtr);
                    
            if (action === 'encrypt') {
                        result = btoa(String.fromCharCode.apply(null, resultBytes));
            } else {
                        const paddingValue = resultBytes[resultBytes.length - 1];
                        if (paddingValue > 0 && paddingValue <= blockSize && paddingValue <= resultBytes.length) {
                            resultBytes = resultBytes.slice(0, resultBytes.length - paddingValue);
                        }
                        result = decoder.decode(resultBytes);
                    }
                } catch (error) {
                    console.error('Crypto processing error:', error);
                    alert(`Error during ${algorithm.toUpperCase()} processing: ${error.message}`);
                    // Ensure cleanup even if there's an error
                    try {
                        Module._free(dataPtr); 
                        Module._free(keyPtr); 
                        Module._free(outputPtr);
                    } catch (cleanupError) {
                        console.error('Cleanup error:', cleanupError);
                    }
                    return;
                }
            } else if (algorithm === 'rsa') {
                const n = document.getElementById('rsa-n').value, e = document.getElementById('rsa-e').value, d = document.getElementById('rsa-d').value;
                if (!n || !e || (action === 'decrypt' && !d)) { alert('For RSA, please generate or provide n, e, and d values.'); return; }
                if (isNaN(n) || isNaN(e) || (action === 'decrypt' && isNaN(d))) { alert('Invalid key: RSA keys must be numbers.'); return; }
                const c_process_rsa = Module.cwrap('process_rsa', 'string', ['string', 'number', 'number']);
                if (action === 'encrypt') result = c_process_rsa(text, BigInt(n), -BigInt(e));
                else result = c_process_rsa(text, BigInt(n), BigInt(d));
            } else if (algorithm === 'ecies') {
                if (action === 'encrypt') {
                    const pubKeyStr = document.getElementById('ecies-pub').value;
                    if (!pubKeyStr) { alert("Please generate a key pair first."); return; }
                    const c_encrypt_ecies = Module.cwrap('encrypt_ecies', 'string', ['string', 'number', 'number']);
                    const pubKey = pubKeyStr.replace(/[() ]/g, '').split(',');
                    result = c_encrypt_ecies(text, BigInt(pubKey[0]), BigInt(pubKey[1]));
        } else {
                    const privKeyStr = document.getElementById('ecies-priv').value;
                    if (!privKeyStr) { alert("Please generate your key pair to get a private key for decryption."); return; }
                    const c_decrypt_ecies = Module.cwrap('decrypt_ecies', 'string', ['string', 'number']);
                    result = c_decrypt_ecies(text, BigInt(privKeyStr));
                }
            }
            if (outputTextElem) outputTextElem.value = result;
        } catch (e) { console.error("Error during crypto action:", e); alert(`An error occurred while processing '${algorithm}'. Check console.`); }
    }
    
    // --- Add visual feedback for button clicks ---
    function addButtonFeedback() {
        document.querySelectorAll('button').forEach(btn => {
            btn.addEventListener('click', function(e) {
                // Create ripple effect
                const ripple = document.createElement('span');
                const rect = this.getBoundingClientRect();
                const size = Math.max(rect.width, rect.height);
                const x = e.clientX - rect.left - size / 2;
                const y = e.clientY - rect.top - size / 2;
                
                ripple.style.cssText = `
                    position: absolute;
                    width: ${size}px;
                    height: ${size}px;
                    left: ${x}px;
                    top: ${y}px;
                    background: rgba(255, 255, 255, 0.3);
                    border-radius: 50%;
                    transform: scale(0);
                    animation: ripple 0.6s linear;
                    pointer-events: none;
                `;
                
                this.appendChild(ripple);
                setTimeout(() => ripple.remove(), 600);
            });
        });
    }
    
    // --- Key Generation and Exchange Handlers ---
    async function generateRsaKeys() { try { const Module = await loadWasmModule('rsa'); const c_generate_keys = Module.cwrap('generate_keys', 'string', []); const keys = c_generate_keys().split(','); const [n_val, e_val, d_val] = keys; document.getElementById('rsa-n').value = n_val; document.getElementById('rsa-e').value = e_val; document.getElementById('rsa-d').value = d_val; document.getElementById('rsa-public-key').value = `(${e_val}, ${n_val})`; document.getElementById('rsa-private-key').value = `(${d_val}, ${n_val})`; } catch (e) { console.error("Error generating RSA keys:", e); } }
    async function generateEciesKeys() { try { const Module = await loadWasmModule('ecc'); const c_generate_keys = Module.cwrap('generate_ecc_keys', 'string', []); const keys = c_generate_keys().split(','); const [priv_val, pub_x, pub_y] = keys; document.getElementById('ecies-priv').value = priv_val; document.getElementById('ecies-pub').value = `(${pub_x}, ${pub_y})`; } catch (e) { console.error("Error generating ECIES keys:", e); } }
    async function generateDhPublicKey(party) { try { const Module = await loadWasmModule('dh'); const c_generate_key = Module.cwrap('generate_dh_public_key', 'number', ['number', 'number', 'number']); const randomPrivateKey = Math.floor(Math.random() * 200) + 50; const privKeyInput = document.getElementById(`dh-priv-${party}`); privKeyInput.value = randomPrivateKey; const p = BigInt(document.getElementById('dh-p').value), g = BigInt(document.getElementById('dh-g').value), privKey = BigInt(randomPrivateKey); document.getElementById(`dh-pub-${party}`).value = c_generate_key(g, p, privKey); } catch (e) { console.error("Error generating DH public key:", e); } }
    async function calculateDhSharedSecret() { try { const Module = await loadWasmModule('dh'); const c_calculate_secret = Module.cwrap('calculate_dh_shared_secret', 'number', ['number', 'number', 'number']); const p = BigInt(document.getElementById('dh-p').value), privA = BigInt(document.getElementById('dh-priv-a').value), pubB = BigInt(document.getElementById('dh-pub-b').value), privB = BigInt(document.getElementById('dh-priv-b').value), pubA = BigInt(document.getElementById('dh-pub-a').value); if (!pubA || !pubB) { alert("Please generate public keys for both parties first."); return; } document.getElementById('dh-secret-a').value = c_calculate_secret(pubB, p, privA); document.getElementById('dh-secret-b').value = c_calculate_secret(pubA, p, privB); } catch (e) { console.error("Error calculating DH shared secret:", e); } }
    async function generateEcdhKeys(party) { try { const Module = await loadWasmModule('ecc'); const c_generate_keys = Module.cwrap('generate_ecc_keys', 'string', []); const keys = c_generate_keys().split(','); document.getElementById(`ecdh-priv-${party}`).value = keys[0]; document.getElementById(`ecdh-pub-${party}`).value = `(${keys[1]}, ${keys[2]})`; } catch (e) { console.error("Error generating ECDH keys:", e); } }
    async function calculateEcdhSharedSecret() { try { const Module = await loadWasmModule('ecc'); const c_calculate_secret = Module.cwrap('calculate_shared_secret', 'string', ['number', 'number', 'number']); const privA = BigInt(document.getElementById('ecdh-priv-a').value), pubB_str = document.getElementById('ecdh-pub-b').value.replace(/[() ]/g, '').split(','), privB = BigInt(document.getElementById('ecdh-priv-b').value), pubA_str = document.getElementById('ecdh-pub-a').value.replace(/[() ]/g, '').split(','); if (pubA_str.length < 2 || pubB_str.length < 2) { alert("Please generate keys for both parties first."); return; } document.getElementById('ecdh-secret-a').value = c_calculate_secret(privA, BigInt(pubB_str[0]), BigInt(pubB_str[1])); document.getElementById('ecdh-secret-b').value = c_calculate_secret(privB, BigInt(pubA_str[0]), BigInt(pubA_str[1])); } catch (e) { console.error("Error calculating ECDH shared secret:", e); } }

    // --- Event Listeners ---
    cryptoTypeRadios.forEach(radio => { radio.addEventListener('change', () => { const type = radio.value; symmetricControls.style.display = (type === 'symmetric') ? 'block' : 'none'; asymmetricControls.style.display = (type === 'asymmetric') ? 'block' : 'none'; keyExchangeControls.style.display = (type === 'key-exchange') ? 'block' : 'none'; updatePanels(); loadWasmModule(getCurrentAlgorithm()).catch(err => console.error(err)); }); });
    [symmetricAlgoSelect, asymmetricAlgoSelect, keyExchangeAlgoSelect].forEach(select => { select.addEventListener('change', () => { updatePanels(); loadWasmModule(getCurrentAlgorithm()).catch(err => console.error(err)); }); });
    document.querySelectorAll('.encrypt-btn').forEach(btn => btn.addEventListener('click', () => handleCryptoAction('encrypt')));
    document.querySelectorAll('.decrypt-btn').forEach(btn => btn.addEventListener('click', () => handleCryptoAction('decrypt')));
    document.getElementById('generate-rsa-btn').addEventListener('click', generateRsaKeys);
    document.getElementById('generate-ecies-btn').addEventListener('click', generateEciesKeys);
    document.querySelectorAll('.generate-dh-btn').forEach(btn => btn.addEventListener('click', (e) => generateDhPublicKey(e.target.dataset.party)));
    document.getElementById('calculate-dh-secret-btn').addEventListener('click', calculateDhSharedSecret);
    document.querySelectorAll('.generate-ecdh-btn').forEach(btn => btn.addEventListener('click', (e) => generateEcdhKeys(e.target.dataset.party)));
    document.getElementById('calculate-ecdh-secret-btn').addEventListener('click', calculateEcdhSharedSecret);

    // Add CSS for ripple animation
    const style = document.createElement('style');
    style.textContent = `
        @keyframes ripple {
            to {
                transform: scale(4);
                opacity: 0;
            }
        }
        button {
            position: relative;
            overflow: hidden;
        }
    `;
    document.head.appendChild(style);
    
    // --- Initial Page Setup ---
    addButtonFeedback();
    document.querySelector('input[name="crypto-type"]:checked').dispatchEvent(new Event('change'));
});