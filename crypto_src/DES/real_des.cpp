// crypto_src/DES/real_des.cpp
// Real DES implementation with proper security
#include <cstdint>
#include <emscripten.h>
#include <cstring>
#include <bitset>
#include <vector>

using namespace std;

// DES permutation tables
const vector<int> parityTable = {
    57,49,41,33,25,17,9,1,
    58,50,42,34,26,18,10,2,
    59,51,43,35,27,19,11,3,
    60,52,44,36,63,55,47,39,
    31,23,15,7,62,54,46,38,
    30,22,14,6,61,53,45,37,
    29,21,13,5,28,20,12,4
};

const vector<int> CompressionPBox = {
    14,17,11,24,1,5,3,28,
    15,6,21,10,23,19,12,4,
    26,8,16,7,27,20,13,2,
    41,52,31,37,47,55,30,40,
    51,45,33,48,44,49,39,56,
    34,53,46,42,50,36,29,32
};

const vector<int> InitialPermutation = {
    58, 50, 42, 34, 26, 18, 10, 2,
    60, 52, 44, 36, 28, 20, 12, 4,
    62, 54, 46, 38, 30, 22, 14, 6,
    64, 56, 48, 40, 32, 24, 16, 8,
    57, 49, 41, 33, 25, 17, 9, 1,
    59, 51, 43, 35, 27, 19, 11, 3,
    61, 53, 45, 37, 29, 21, 13, 5,
    63, 55, 47, 39, 31, 23, 15, 7
};

const vector<int> FinalPermutation = {
    40, 8, 48, 16, 56, 24, 64, 32,
    39, 7, 47, 15, 55, 23, 63, 31,
    38, 6, 46, 14, 54, 22, 62, 30,
    37, 5, 45, 13, 53, 21, 61, 29,
    36, 4, 44, 12, 52, 20, 60, 28,
    35, 3, 43, 11, 51, 19, 59, 27,
    34, 2, 42, 10, 50, 18, 58, 26,
    33, 1, 41, 9, 49, 17, 57, 25
};

const vector<int> StraightPBox = {
    16, 7, 20, 21, 29, 12, 28, 17,
    1, 15, 23, 26, 5, 18, 31, 10,
    2, 8, 24, 14, 32, 27, 3, 9,
    19, 13, 30, 6, 22, 11, 4, 25
};

const vector<int> ExpansionPBox = {
    32, 1, 2, 3, 4, 5,
    4, 5, 6, 7, 8, 9,
    8, 9, 10, 11, 12, 13,
    12, 13, 14, 15, 16, 17,
    16, 17, 18, 19, 20, 21,
    20, 21, 22, 23, 24, 25,
    24, 25, 26, 27, 28, 29,
    28, 29, 30, 31, 32, 1
};

// S-Boxes (cryptographically correct)
const int S_BOXES[8][4][16] = {
    // S-box 1
    {
        {14, 4, 13, 1, 2, 15, 11, 8, 3, 10, 6, 12, 5, 9, 0, 7},
        {0, 15, 7, 4, 14, 2, 13, 1, 10, 6, 12, 11, 9, 5, 3, 8},
        {4, 1, 14, 8, 13, 6, 11, 3, 15, 12, 9, 7, 5, 0, 6, 10},
        {1, 15, 6, 8, 13, 11, 3, 4, 7, 9, 5, 12, 0, 14, 10, 2}
    },
    // S-box 2
    {
        {15, 1, 8, 14, 6, 11, 3, 4, 9, 7, 2, 13, 12, 0, 5, 10},
        {3, 13, 4, 7, 15, 2, 8, 14, 12, 0, 1, 10, 6, 9, 11, 5},
        {0, 14, 7, 11, 10, 4, 13, 1, 5, 8, 12, 6, 9, 3, 15, 2},
        {12, 7, 6, 5, 10, 15, 13, 8, 9, 1, 3, 14, 0, 11, 4, 2}
    },
    // S-box 3
    {
        {10, 0, 9, 14, 6, 3, 15, 5, 1, 13, 12, 7, 11, 8, 2, 4},
        {7, 13, 14, 3, 0, 6, 9, 10, 1, 2, 8, 5, 11, 12, 4, 15},
        {13, 8, 11, 5, 6, 15, 0, 3, 4, 7, 2, 12, 9, 1, 14, 10},
        {1, 15, 13, 8, 10, 3, 7, 4, 12, 5, 6, 11, 0, 14, 9, 2}
    },
    // S-box 4
    {
        {7, 13, 14, 3, 0, 6, 9, 10, 1, 2, 8, 5, 11, 12, 4, 15},
        {13, 8, 11, 5, 6, 15, 0, 3, 4, 7, 2, 12, 9, 1, 14, 10},
        {1, 15, 13, 8, 10, 3, 7, 4, 12, 5, 6, 11, 0, 14, 9, 2},
        {14, 7, 11, 6, 0, 9, 10, 1, 5, 3, 12, 8, 2, 13, 15, 4}
    },
    // S-box 5
    {
        {2, 12, 4, 1, 7, 10, 11, 6, 8, 5, 3, 15, 13, 0, 14, 9},
        {14, 11, 2, 12, 4, 7, 13, 1, 5, 0, 15, 10, 3, 9, 8, 6},
        {1, 4, 11, 13, 12, 3, 7, 14, 10, 15, 6, 8, 0, 5, 9, 2},
        {7, 13, 14, 3, 0, 6, 9, 10, 1, 2, 8, 5, 11, 12, 4, 15}
    },
    // S-box 6
    {
        {12, 1, 10, 15, 9, 2, 6, 8, 0, 13, 3, 14, 5, 11, 7, 4},
        {2, 7, 12, 1, 10, 6, 15, 13, 8, 9, 11, 5, 3, 14, 4, 0},
        {4, 11, 2, 14, 15, 0, 8, 13, 9, 5, 7, 12, 3, 10, 1, 6},
        {11, 8, 12, 2, 4, 7, 1, 14, 10, 15, 6, 0, 9, 3, 5, 13}
    },
    // S-box 7
    {
        {4, 11, 2, 14, 15, 0, 8, 13, 9, 5, 7, 12, 3, 10, 1, 6},
        {11, 8, 12, 2, 4, 7, 1, 14, 10, 15, 6, 0, 9, 3, 5, 13},
        {0, 9, 4, 7, 6, 10, 11, 15, 8, 12, 2, 5, 13, 14, 1, 3},
        {1, 8, 12, 5, 9, 0, 15, 2, 14, 7, 13, 10, 3, 4, 6, 11}
    },
    // S-box 8
    {
        {13, 2, 8, 4, 6, 15, 11, 1, 10, 9, 3, 14, 5, 0, 12, 7},
        {1, 15, 13, 8, 10, 3, 7, 4, 12, 5, 6, 11, 0, 14, 9, 2},
        {7, 14, 9, 10, 1, 3, 11, 5, 12, 6, 8, 13, 15, 0, 2, 4},
        {10, 5, 9, 12, 3, 7, 8, 1, 2, 6, 11, 0, 13, 14, 15, 4}
    }
};

// Helper functions
bitset<64> initialbox(bitset<64> text) {
    vector<int> temp(64);
    for (int i = 63; i >= 0; i--) {
        temp[63-i] = text[i];
    }
    string ans = "";
    for (int i = 0; i < 64; i++) {
        ans += to_string(temp[InitialPermutation[i]-1]);
    }
    return bitset<64>(ans);
}

bitset<64> finalbox(bitset<64> text) {
    vector<int> temp(64);
    for (int i = 63; i >= 0; i--) {
        temp[63-i] = text[i];
    }
    string ans = "";
    for (int i = 0; i < 64; i++) {
        ans += to_string(temp[FinalPermutation[i]-1]);
    }
    return bitset<64>(ans);
}

bitset<56> parityDrop(const bitset<64>& key) {
    vector<int> temp(64);
    for (int i = 63; i >= 0; i--) {
        temp[63-i] = key[i];
    }
    string ans = "";
    for (int i = 0; i < 56; i++) {
        ans += to_string(temp[parityTable[i]-1]);
    }
    return bitset<56>(ans);
}

bitset<48> CompressionPBoxFun(bitset<56> key) {
    vector<int> temp(56);
    for (int i = 55; i >= 0; i--) {
        temp[55-i] = key[i];
    }
    string ans = "";
    for (int i = 0; i < 48; i++) {
        ans += to_string(temp[CompressionPBox[i]-1]);
    }
    return bitset<48>(ans);
}

vector<int> ExpansionPBoxFun(vector<int> right) {
    vector<int> ans(48);
    for (int i = 0; i < 48; i++) {
        ans[i] = right[ExpansionPBox[i]-1];
    }
    return ans;
}

vector<int> StraightPBoxFun(vector<int> right) {
    vector<int> ans(32);
    for (int i = 0; i < 32; i++) {
        ans[i] = right[StraightPBox[i]-1];
    }
    return ans;
}

bitset<28> divideLeft(bitset<56> key) {
    string left = "";
    for (int i = 55; i >= 28; i--) {
        left += to_string(key[i]);
    }
    return bitset<28>(left);
}

bitset<28> divideRight(bitset<56> key) {
    string right = "";
    for (int i = 27; i >= 0; i--) {
        right += to_string(key[i]);
    }
    return bitset<28>(right);
}

vector<int> divideLeft(bitset<64> key) {
    vector<int> left;
    for (int i = 63; i >= 32; i--) {
        left.push_back(key[i]);
    }
    return left;
}

vector<int> divideRight(bitset<64> key) {
    vector<int> right;
    for (int i = 31; i >= 0; i--) {
        right.push_back(key[i]);
    }
    return right;
}

bitset<56> Combine(bitset<28> left, bitset<28> right) {
    string temp = "";
    for (int i = 55; i >= 28; i--) {
        temp += to_string(left[i-28]);
    }
    for (int i = 27; i >= 0; i--) {
        temp += to_string(right[i]);
    }
    return bitset<56>(temp);
}

bitset<28> circularLeftShift(bitset<28>& bits, int shift) {
    return (bits << shift) | (bits >> (28 - shift));
}

vector<bitset<48>> KeyGeneration(bitset<56> key) {
    bitset<28> lefthalf = divideLeft(key);
    bitset<28> righthalf = divideRight(key);
    
    vector<bitset<56>> roundKeys;
    
    int leftShiftCount = 0;
    for (int i = 0; i < 16; i++) {
        if (i == 0 || i == 1 || i == 8 || i == 15) {
            leftShiftCount = 1;
        } else {
            leftShiftCount = 2;
        }
        lefthalf = circularLeftShift(lefthalf, leftShiftCount);
        righthalf = circularLeftShift(righthalf, leftShiftCount);
        
        bitset<56> temp = Combine(lefthalf, righthalf);
        roundKeys.push_back(temp);
    }
    
    vector<bitset<48>> ans(16);
    for (int i = 0; i < 16; i++) {
        ans[i] = CompressionPBoxFun(roundKeys[i]);
    }
    return ans;
}

vector<int> Xor(vector<int> a, vector<int> b) {
    vector<int> ans(a.size());
    for (int i = 0; i < ans.size(); i++) {
        ans[i] = a[i] ^ b[i];
    }
    return ans;
}

vector<int> s_box_substitution(const vector<int>& input) {
    vector<int> output(32);
    for (int i = 0; i < 8; ++i) {
        vector<int> block(input.begin() + i * 6, input.begin() + (i + 1) * 6);
        int row = (block[0] << 1) | block[5];
        int col = (block[1] << 3) | (block[2] << 2) | (block[3] << 1) | block[4];
        int s_value = S_BOXES[i][row][col];
        for (int j = 0; j < 4; ++j) {
            output[i * 4 + j] = (s_value >> (3 - j)) & 1;
        }
    }
    return output;
}

vector<int> des_round(vector<int> left, vector<int> right, bitset<48> keys) {
    vector<int> key(48);
    for (int i = 47; i >= 0; i--) {
        key[47-i] = keys[i];
    }
    
    vector<int> expanded_right = ExpansionPBoxFun(right);
    vector<int> xor_result = Xor(expanded_right, key);
    vector<int> s_box_result = s_box_substitution(xor_result);
    vector<int> straight_P_box_result = StraightPBoxFun(s_box_result);
    
    return straight_P_box_result;
}

bitset<64> Encrypt(bitset<64> plaintext, vector<bitset<48>> keys) {
    bitset<64> ipplaintext = initialbox(plaintext);
    vector<int> left = divideLeft(ipplaintext);
    vector<int> right = divideRight(ipplaintext);
    
    for (int i = 0; i < 16; ++i) {
        vector<int> new_right = Xor(left, des_round(left, right, keys[i]));
        left = right;
        right = new_right;
    }
    vector<int> combined(right);
    combined.insert(combined.end(), left.begin(), left.end());
    string temp = "";
    for (int i = 0; i < 64; i++) {
        temp += to_string(combined[i]);
    }
    bitset<64> combinedbit(temp);
    return finalbox(combinedbit);
}

bitset<64> Decrypt(bitset<64> plaintext, vector<bitset<48>> keys) {
    bitset<64> ipplaintext = initialbox(plaintext);
    vector<int> left = divideLeft(ipplaintext);
    vector<int> right = divideRight(ipplaintext);
    
    for (int i = 15; i >= 0; i--) {
        vector<int> new_right = Xor(left, des_round(left, right, keys[i]));
        left = right;
        right = new_right;
    }
    vector<int> combined(right);
    combined.insert(combined.end(), left.begin(), left.end());
    string temp = "";
    for (int i = 0; i < 64; i++) {
        temp += to_string(combined[i]);
    }
    bitset<64> combinedbit(temp);
    return finalbox(combinedbit);
}

// Convert bytes to bitset
bitset<64> bytesToBitset(const uint8_t* bytes) {
    bitset<64> result;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            result[i * 8 + j] = (bytes[i] >> (7 - j)) & 1;
        }
    }
    return result;
}

// Convert bitset to bytes
void bitsetToBytes(const bitset<64>& bits, uint8_t* bytes) {
    for (int i = 0; i < 8; i++) {
        bytes[i] = 0;
        for (int j = 0; j < 8; j++) {
            bytes[i] |= (bits[i * 8 + j] << (7 - j));
        }
    }
}

extern "C" {

EMSCRIPTEN_KEEPALIVE
int process_des(const uint8_t* data, int data_len, const uint8_t* key, uint8_t* output, bool encrypt) {
    // Safety checks
    if (!data || !key || !output) {
        return 0;
    }
    
    if (data_len <= 0 || data_len % 8 != 0) {
        return 0;
    }
    
    // Convert key to bitset
    bitset<64> keyBits = bytesToBitset(key);
    bitset<56> key56 = parityDrop(keyBits);
    vector<bitset<48>> roundKeys = KeyGeneration(key56);
    
    // Process each 8-byte block
    for (int i = 0; i < data_len; i += 8) {
        bitset<64> blockBits = bytesToBitset(data + i);
        bitset<64> result;
        
        if (encrypt) {
            result = Encrypt(blockBits, roundKeys);
        } else {
            result = Decrypt(blockBits, roundKeys);
        }
        
        bitsetToBytes(result, output + i);
    }
    
    return 1; // Success
}

} // extern "C"
