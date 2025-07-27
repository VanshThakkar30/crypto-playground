# Cryptography Playground 🛡️

A full-stack web application designed as an educational tool to demonstrate and analyze the workings of various classical, symmetric, asymmetric, and key exchange cryptographic protocols. The backend logic for the ciphers is written in **C++** and compiled to high-performance **WebAssembly (WASM)** to run efficiently and securely in the browser.

This project was built to showcase a deep understanding of software engineering principles, including full-stack architecture, low-level programming with C++, modern web technologies, and core cybersecurity concepts.

---

## Features ✨

The application is organized into three main categories:

#### ⚔️ Symmetric Ciphers
- **AES (Advanced Encryption Standard):** A modern block cipher with padding.
- **DES (Data Encryption Standard):** A historically significant block cipher.
- **Vigenère Cipher:** A classical polyalphabetic substitution cipher.
- **Playfair Cipher:** A classical digraph substitution cipher.
- **Rail Fence Cipher:** A classical transposition cipher.

####  asymmetric Asymmetric Ciphers
- **RSA:** Full implementation of key generation (public/private pair), encryption, and decryption.
- **ECC Encryption (ECIES):** Demonstrates a modern hybrid encryption scheme using Elliptic Curve Cryptography.

#### 🤝 Key Exchange Protocols
- **Classical Diffie-Hellman (DH):** An interactive two-party simulation (Alice & Bob) to establish a shared secret over an insecure channel.
- **Elliptic Curve Diffie-Hellman (ECDH):** The modern, more secure version of the key exchange protocol, also simulated between two parties.

#### ⚙️ Core Technology & UX
- **High-Performance Backend:** C++ cryptographic logic compiled to WebAssembly (WASM) for near-native speed in the browser.
- **Dynamic UI**: The interface intelligently adapts to the requirements of each selected algorithm, providing dedicated panels for symmetric, asymmetric, and key-exchange operations.
- **Input Validation**: User-friendly checks to ensure keys and input are in the correct format for each algorithm.
- **Canvas Visualizer**: A panel that graphically demonstrates the internal workings of algorithms, with a working example for the Rail Fence cipher.

---

## Technology Stack 💻

- **Backend Logic**: C++
- **WebAssembly Compiler**: Emscripten
- **Web Server**: Python with Flask
- **Frontend**: HTML5, CSS3, Vanilla JavaScript (ES6 Modules), HTML Canvas

---

## Setup and Installation 🚀

To run this project locally, please follow these steps:

1.  **Clone the repository:**
    ```bash
    git clone [https://github.com/your-username/your-repo-name.git](https://github.com/your-username/your-repo-name.git)
    cd your-repo-name
    ```

2.  **Set up the Python Environment:**
    ```bash
    # Create a virtual environment
    python -m venv venv

    # Activate it
    # Windows
    venv\Scripts\activate
    # macOS/Linux
    source venv/bin/activate

    # Install dependencies
    pip install -r requirements.txt
    ```

3.  **Compile the WebAssembly Modules:**
    - Make sure you have the [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html) installed and activated in your terminal.
    - Run the build script to compile all C++ modules:
      ```bash
      # On macOS/Linux
      chmod +x build_wasm.sh
      ./build_wasm.sh
      
      # On Windows, you may need to run the commands inside the build_wasm.sh script one by one.
      ```

4.  **Run the Application:**
    ```bash
    python run.py
    ```
    The application will be available at `http://12.0.0.1:5000`.