#include <stdio.h>
#include <stdint.h>
#include <string.h>

// SM4常数定义
static const uint32_t SM4_FK[4] = {
    0xA3B1BAC6, 0x56AA3350, 0x677D9197, 0xB27022DC
};

static const uint32_t SM4_CK[32] = {
    0x00070E15, 0x1C232A31, 0x383F464D, 0x545B6269,
    0x70777E85, 0x8C939AA1, 0xA8AFB6BD, 0xC4CBD2D9,
    0xE0E7EEF5, 0xFC030A11, 0x181F262D, 0x343B4249,
    0x50575E65, 0x6C737A81, 0x888F969D, 0xA4ABB2B9,
    0xC0C7CED5, 0xDCE3EAF1, 0xF8FF060D, 0x141B2229,
    0x30373E45, 0x4C535A61, 0x686F767D, 0x848B9299,
    0xA0A7AEB5, 0xBCC3CAD1, 0xD8DFE6ED, 0xF4FB0209,
    0x10171E25, 0x2C333A41, 0x484F565D, 0x646B7279
};

static const uint8_t SM4_SBOX[256] = {
    0xD6, 0x90, 0xE9, 0xFE, 0xCC, 0xE1, 0x3D, 0xB7, 0x16, 0xB6, 0x14, 0xC2, 0x28, 0xFB, 0x2C, 0x05,
    0x2B, 0x67, 0x9A, 0x76, 0x2A, 0xBE, 0x04, 0xC3, 0xAA, 0x44, 0x13, 0x26, 0x49, 0x86, 0x06, 0x99,
    0x9C, 0x42, 0x50, 0xF4, 0x91, 0xEF, 0x98, 0x7A, 0x33, 0x54, 0x0B, 0x43, 0xED, 0xCF, 0xAC, 0x62,
    0xE4, 0xB3, 0x1C, 0xA9, 0xC9, 0x08, 0xE8, 0x95, 0x80, 0xDF, 0x94, 0xFA, 0x75, 0x8F, 0x3F, 0xA6,
    0x47, 0x07, 0xA7, 0xFC, 0xF3, 0x73, 0x17, 0xBA, 0x83, 0x59, 0x3C, 0x19, 0xE6, 0x85, 0x4F, 0xA8,
    0x68, 0x6B, 0x81, 0xB2, 0x71, 0x64, 0xDA, 0x8B, 0xF8, 0xEB, 0x0F, 0x4B, 0x70, 0x56, 0x9D, 0x35,
    0x1E, 0x24, 0x0E, 0x5E, 0x63, 0x58, 0xD1, 0xA2, 0x25, 0x22, 0x7C, 0x3B, 0x01, 0x21, 0x78, 0x87,
    0xD4, 0x00, 0x46, 0x57, 0x9F, 0xD3, 0x27, 0x52, 0x4C, 0x36, 0x02, 0xE7, 0xA0, 0xC4, 0xC8, 0x9E,
    0xEA, 0xBF, 0x8A, 0xD2, 0x40, 0xC7, 0x38, 0xB5, 0xA3, 0xF7, 0xF2, 0xCE, 0xF9, 0x61, 0x15, 0xA1,
    0xE0, 0xAE, 0x5D, 0xA4, 0x9B, 0x34, 0x1A, 0x55, 0xAD, 0x93, 0x32, 0x30, 0xF5, 0x8C, 0xB1, 0xE3,
    0x1D, 0xF6, 0xE2, 0x2E, 0x82, 0x66, 0xCA, 0x60, 0xC0, 0x29, 0x23, 0xAB, 0x0D, 0x53, 0x4E, 0x6F,
    0xD5, 0xDB, 0x37, 0x45, 0xDE, 0xFD, 0x8E, 0x2F, 0x03, 0xFF, 0x6A, 0x72, 0x6D, 0x6C, 0x5B, 0x51,
    0x8D, 0x1B, 0xAF, 0x92, 0xBB, 0xDD, 0xBC, 0x7F, 0x11, 0xD9, 0x5C, 0x41, 0x1F, 0x10, 0x5A, 0xD8,
    0x0A, 0xC1, 0x31, 0x88, 0xA5, 0xCD, 0x7B, 0xBD, 0x2D, 0x74, 0xD0, 0x12, 0xB8, 0xE5, 0xB4, 0xB0,
    0x89, 0x69, 0x97, 0x4A, 0x0C, 0x96, 0x77, 0x7E, 0x65, 0xB9, 0xF1, 0x09, 0xC5, 0x6E, 0xC6, 0x84,
    0x18, 0xF0, 0x7D, 0xEC, 0x3A, 0xDC, 0x4D, 0x20, 0x79, 0xEE, 0x5F, 0x3E, 0xD7, 0xCB, 0x39, 0x48
};

// 循环左移
static uint32_t rotl(uint32_t x, int n) {
    return (x << n) | (x >> (32 - n));
}

// 线性变换L
static uint32_t l_transform(uint32_t x) {
    return x ^ rotl(x, 2) ^ rotl(x, 10) ^ rotl(x, 18) ^ rotl(x, 24);
}

// 线性变换L'
static uint32_t l_prime_transform(uint32_t x) {
    return x ^ rotl(x, 13) ^ rotl(x, 23);
}

// 非线性变换τ
static uint32_t tau(uint32_t a) {
    uint32_t b = 0;
    b |= SM4_SBOX[(a >> 24) & 0xFF] << 24;
    b |= SM4_SBOX[(a >> 16) & 0xFF] << 16;
    b |= SM4_SBOX[(a >> 8) & 0xFF] << 8;
    b |= SM4_SBOX[a & 0xFF];
    return b;
}

// 轮函数F
static uint32_t sm4_f(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3, uint32_t rk) {
    return x0 ^ l_transform(tau(x1 ^ x2 ^ x3 ^ rk));
}

// 密钥扩展
static void sm4_key_expansion(const uint8_t key[16], uint32_t rk[32]) {
    uint32_t k[36];

    // 初始化轮密钥
    k[0] = ((uint32_t)key[0] << 24) | ((uint32_t)key[1] << 16) |
        ((uint32_t)key[2] << 8) | key[3] ^ SM4_FK[0];
    k[1] = ((uint32_t)key[4] << 24) | ((uint32_t)key[5] << 16) |
        ((uint32_t)key[6] << 8) | key[7] ^ SM4_FK[1];
    k[2] = ((uint32_t)key[8] << 24) | ((uint32_t)key[9] << 16) |
        ((uint32_t)key[10] << 8) | key[11] ^ SM4_FK[2];
    k[3] = ((uint32_t)key[12] << 24) | ((uint32_t)key[13] << 16) |
        ((uint32_t)key[14] << 8) | key[15] ^ SM4_FK[3];

    // 生成轮密钥
    for (int i = 0; i < 32; i++) {
        k[i + 4] = k[i] ^ l_prime_transform(tau(k[i + 1] ^ k[i + 2] ^ k[i + 3] ^ SM4_CK[i]));
        rk[i] = k[i + 4];
    }
}

// SM4加密单个分组
static void sm4_encrypt_block(const uint8_t plaintext[16], const uint8_t key[16], uint8_t ciphertext[16]) {
    uint32_t rk[32];
    uint32_t x[36];  // 32轮 + 4个初始值

    sm4_key_expansion(key, rk);

    // 初始化状态
    x[0] = ((uint32_t)plaintext[0] << 24) | ((uint32_t)plaintext[1] << 16) |
        ((uint32_t)plaintext[2] << 8) | plaintext[3];
    x[1] = ((uint32_t)plaintext[4] << 24) | ((uint32_t)plaintext[5] << 16) |
        ((uint32_t)plaintext[6] << 8) | plaintext[7];
    x[2] = ((uint32_t)plaintext[8] << 24) | ((uint32_t)plaintext[9] << 16) |
        ((uint32_t)plaintext[10] << 8) | plaintext[11];
    x[3] = ((uint32_t)plaintext[12] << 24) | ((uint32_t)plaintext[13] << 16) |
        ((uint32_t)plaintext[14] << 8) | plaintext[15];

    // 32轮迭代
    for (int i = 0; i < 32; i++) {
        x[i + 4] = sm4_f(x[i], x[i + 1], x[i + 2], x[i + 3], rk[i]);
    }

    // 输出结果 (反序)
    ciphertext[0] = x[35] >> 24; ciphertext[1] = x[35] >> 16;
    ciphertext[2] = x[35] >> 8;  ciphertext[3] = x[35];
    ciphertext[4] = x[34] >> 24; ciphertext[5] = x[34] >> 16;
    ciphertext[6] = x[34] >> 8;  ciphertext[7] = x[34];
    ciphertext[8] = x[33] >> 24; ciphertext[9] = x[33] >> 16;
    ciphertext[10] = x[33] >> 8; ciphertext[11] = x[33];
    ciphertext[12] = x[32] >> 24; ciphertext[13] = x[32] >> 16;
    ciphertext[14] = x[32] >> 8; ciphertext[15] = x[32];
}

// SM4解密单个分组
static void sm4_decrypt_block(const uint8_t ciphertext[16], const uint8_t key[16], uint8_t decrypted[16]) {
    uint32_t rk[32];
    uint32_t x[36];

    sm4_key_expansion(key, rk);

    // 逆向轮密钥
    uint32_t reverse_rk[32];
    for (int i = 0; i < 32; i++) {
        reverse_rk[i] = rk[31 - i];
    }

    // 初始化状态
    x[0] = ((uint32_t)ciphertext[0] << 24) | ((uint32_t)ciphertext[1] << 16) |
        ((uint32_t)ciphertext[2] << 8) | ciphertext[3];
    x[1] = ((uint32_t)ciphertext[4] << 24) | ((uint32_t)ciphertext[5] << 16) |
        ((uint32_t)ciphertext[6] << 8) | ciphertext[7];
    x[2] = ((uint32_t)ciphertext[8] << 24) | ((uint32_t)ciphertext[9] << 16) |
        ((uint32_t)ciphertext[10] << 8) | ciphertext[11];
    x[3] = ((uint32_t)ciphertext[12] << 24) | ((uint32_t)ciphertext[13] << 16) |
        ((uint32_t)ciphertext[14] << 8) | ciphertext[15];

    // 32轮迭代(使用逆序轮密钥)
    for (int i = 0; i < 32; i++) {
        x[i + 4] = sm4_f(x[i], x[i + 1], x[i + 2], x[i + 3], reverse_rk[i]);
    }

    // 输出结果 (反序)
    decrypted[0] = x[35] >> 24; decrypted[1] = x[35] >> 16;
    decrypted[2] = x[35] >> 8;  decrypted[3] = x[35];
    decrypted[4] = x[34] >> 24; decrypted[5] = x[34] >> 16;
    decrypted[6] = x[34] >> 8;  decrypted[7] = x[34];
    decrypted[8] = x[33] >> 24; decrypted[9] = x[33] >> 16;
    decrypted[10] = x[33] >> 8; decrypted[11] = x[33];
    decrypted[12] = x[32] >> 24; decrypted[13] = x[32] >> 16;
    decrypted[14] = x[32] >> 8; decrypted[15] = x[32];
}

// 打印十六进制数组
void print_hex(const uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%02X", data[i]);
    }
    printf("\n");
}

int main() {
    uint8_t key[16] = {
        0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10,
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF
    };

    uint8_t plaintext[16] = {
        
        0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10,
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF
    };

    uint8_t ciphertext[16];
    uint8_t decrypted[16];

    // 加密
    sm4_encrypt_block(plaintext, key, ciphertext);
    printf("Ciphertext: ");
    print_hex(ciphertext, 16);

    // 解密
    sm4_decrypt_block(ciphertext, key, decrypted);
    printf("Decrypted:  ");
    print_hex(decrypted, 16);

    // 验证解密结果
    if (memcmp(plaintext, decrypted, sizeof(plaintext))) {
        printf("SM4 test FAILED!\n");
        return 1;
    }

    printf("SM4 encryption/decryption test PASSED!\n");

    return 0;
}