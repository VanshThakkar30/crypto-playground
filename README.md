# Cryptography Playground 🛡️

A full-stack web application designed as an educational tool to demonstrate the workings of various classical, symmetric, and asymmetric cryptographic algorithms. The backend logic for the ciphers is written in C++ and compiled to WebAssembly to run efficiently in the browser.

This project was built to showcase a deep understanding of software engineering principles, including full-stack architecture, low-level programming with C++, and modern web technologies.

## Features ✨

- **Algorithm Selection**: Choose between symmetric and asymmetric cryptography.
- **Symmetric Ciphers**:
  - AES & DES (Block Ciphers)
  - Vigenère, Playfair, Rail Fence (Classical Ciphers)
- **Asymmetric Ciphers**:
  - RSA Key Generation, Encryption, and Decryption.
  - Elliptic Curve Cryptography (ECC) demonstrating ECDH Key Exchange.
- **Dynamic UI**: The interface intelligently adapts to the requirements of each selected algorithm.
- **High-Performance Backend**: C++ cryptographic logic compiled to WebAssembly for near-native speed in the browser.

## Technology Stack ⚙️

- **Backend Logic**: C++
- **WebAssembly Compiler**: Emscripten
- **Web Server**: Python with Flask
- **Frontend**: HTML5, CSS3, Vanilla JavaScript (ES6 Modules)

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
    - Run the build script:
      ```bash
      # On macOS/Linux
      chmod +x build_wasm.sh
      ./build_wasm.sh
      
      # On Windows, you may need to run the commands inside the script one by one.
      ```

4.  **Run the Application:**
    ```bash
    python run.py
    ```
    The application will be available at `http://127.0.0.1:5000`.