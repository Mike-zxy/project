#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


static const uint32_t IV[8] = {
    0x7380166F, 0x4914B2B9, 0x172442D7, 0xDA8A0600,
    0xA96F30BC, 0x163138AA, 0xE38DEE4D, 0xB0FB0E4E
};

#define ROTL(x, n) (((x) << (n)) | ((x) >> (32 - (n))))
#define P0(x) ((x) ^ ROTL((x), 9) ^ ROTL((x), 17))
#define P1(x) ((x) ^ ROTL((x), 15) ^ ROTL((x), 23))
#define FF0(x, y, z) ((x) ^ (y) ^ (z))
#define FF1(x, y, z) (((x) & (y)) | ((x) & (z)) | ((y) & (z)))
#define GG0(x, y, z) ((x) ^ (y) ^ (z))
#define GG1(x, y, z) (((x) & (y)) | ((~(x)) & (z)))
#define T0 0x79CC4519
#define T1 0x7A879D8A

void sm3_compress(uint32_t digest[8], const uint8_t block[64]) {
    uint32_t W[68], W1[64];
    uint32_t A = digest[0], B = digest[1], C = digest[2], D = digest[3];
    uint32_t E = digest[4], F = digest[5], G = digest[6], H = digest[7];

    for (int i = 0; i < 16; i++) {
        W[i] = ((uint32_t)block[i * 4] << 24) |
            ((uint32_t)block[i * 4 + 1] << 16) |
            ((uint32_t)block[i * 4 + 2] << 8) |
            ((uint32_t)block[i * 4 + 3]);
    }

    for (int i = 16; i < 68; i++) {
        uint32_t tmp = W[i - 16] ^ W[i - 9] ^ ROTL(W[i - 3], 15);
        W[i] = P1(tmp) ^ ROTL(W[i - 13], 7) ^ W[i - 6];
    }

    for (int i = 0; i < 64; i++) {
        W1[i] = W[i] ^ W[i + 4];
    }

    for (int j = 0; j < 64; j++) {
        uint32_t SS1 = ROTL((ROTL(A, 12) + E + ROTL((j < 16 ? T0 : T1), j)) & 0xFFFFFFFF, 7);
        uint32_t SS2 = SS1 ^ ROTL(A, 12);
        uint32_t TT1 = ((j < 16 ? FF0(A, B, C) : FF1(A, B, C)) + D + SS2 + W1[j]) & 0xFFFFFFFF;
        uint32_t TT2 = ((j < 16 ? GG0(E, F, G) : GG1(E, F, G)) + H + SS1 + W[j]) & 0xFFFFFFFF;

        D = C;
        C = ROTL(B, 9);
        B = A;
        A = TT1;

        H = G;
        G = ROTL(F, 19);
        F = E;
        E = P0(TT2);
    }

    digest[0] ^= A;
    digest[1] ^= B;
    digest[2] ^= C;
    digest[3] ^= D;
    digest[4] ^= E;
    digest[5] ^= F;
    digest[6] ^= G;
    digest[7] ^= H;
}

void sm3_hash(const uint8_t* message, size_t len, uint8_t hash[32]) {
    uint64_t bit_len = len * 8;
    size_t pad_len = ((len + 9 + 63) / 64) * 64;
    uint8_t* buffer = (uint8_t*)malloc(pad_len);
    memcpy(buffer, message, len);
    buffer[len] = 0x80;
    memset(buffer + len + 1, 0, pad_len - len - 9);
    for (int i = 0; i < 8; i++) {
        buffer[pad_len - 8 + i] = (bit_len >> ((7 - i) * 8)) & 0xFF;
    }

    uint32_t digest[8];
    memcpy(digest, IV, sizeof(IV));

    for (size_t i = 0; i < pad_len; i += 64) {
        sm3_compress(digest, buffer + i);
    }

    for (int i = 0; i < 8; i++) {
        hash[i * 4] = (digest[i] >> 24) & 0xFF;
        hash[i * 4 + 1] = (digest[i] >> 16) & 0xFF;
        hash[i * 4 + 2] = (digest[i] >> 8) & 0xFF;
        hash[i * 4 + 3] = digest[i] & 0xFF;
    }

    free(buffer);
}

void benchmark_sm3(int repeat) {
    const size_t TEST_SIZE = 1024 * 1024;  // 1MB
    uint8_t* msg = (uint8_t*)malloc(TEST_SIZE);
    memset(msg, 'A', TEST_SIZE);

    uint8_t hash[32];
    clock_t start = clock();

    for (int i = 0; i < repeat; i++) {
        sm3_hash(msg, TEST_SIZE, hash);
    }

    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Benchmark SM3_pro: %d x 1MB = %.2f MB total\n", repeat, (double)repeat);
    printf("Time taken: %.4f seconds\n", elapsed);
    printf("Throughput: %.2f MB/s\n", repeat / elapsed);

    free(msg);
}
int main() {
    // 正确性验证
    const char* msg = "abc";
    uint8_t hash[32];

    printf("测试 SM3_pro(\"abc\")...\n");
    sm3_hash((const uint8_t*)msg, strlen(msg), hash);

    printf("Output: ");
    for (int i = 0; i < 32; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");

    const char* expected = "66c7f0f462eeedd9d1f2d46bdc10e4e24167c4875cf2f7a2297da02b8f4ba8e0";
    printf("Expected: %s\n", expected);

    // 性能测试
    printf("\n=== SM3_pro 性能测试 ===\n");
    benchmark_sm3(100);  // 可调整次数

    return 0;
}
