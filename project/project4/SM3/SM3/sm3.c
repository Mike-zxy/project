#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define ROTL(x,n) (((x) << (n)) | ((x) >> (32 - (n))))

#define T(j) ((j) < 16 ? 0x79cc4519 : 0x7a879d8a)

#define FF(j, x, y, z) ((j) < 16 ? ((x) ^ (y) ^ (z)) : (((x) & (y)) | ((x) & (z)) | ((y) & (z))))
#define GG(j, x, y, z) ((j) < 16 ? ((x) ^ (y) ^ (z)) : (((x) & (y)) | ((~(x)) & (z))))

#define P0(x) ((x) ^ ROTL((x), 9) ^ ROTL((x), 17))
#define P1(x) ((x) ^ ROTL((x), 15) ^ ROTL((x), 23))

typedef struct {
    uint8_t buf[64];
    uint32_t state[8];
    uint64_t count;  // bits
} SM3_CTX;

const uint32_t IV[8] = {
    0x7380166f, 0x4914b2b9,
    0x172442d7, 0xda8a0600,
    0xa96f30bc, 0x163138aa,
    0xe38dee4d, 0xb0fb0e4e
};

void sm3_init(SM3_CTX* ctx) {
    memcpy(ctx->state, IV, sizeof(IV));
    ctx->count = 0;
}

void sm3_compress(SM3_CTX* ctx, const uint8_t* block) {
    uint32_t W[68], W1[64];
    uint32_t A, B, C, D, E, F, G, H;

    for (int i = 0; i < 16; i++) {
        W[i] = ((uint32_t)block[4 * i] << 24) |
            ((uint32_t)block[4 * i + 1] << 16) |
            ((uint32_t)block[4 * i + 2] << 8) |
            ((uint32_t)block[4 * i + 3]);
    }

    for (int i = 16; i < 68; i++) {
        W[i] = P1(W[i - 16] ^ W[i - 9] ^ ROTL(W[i - 3], 15)) ^
            ROTL(W[i - 13], 7) ^ W[i - 6];
    }

    for (int i = 0; i < 64; i++) {
        W1[i] = W[i] ^ W[i + 4];
    }

    A = ctx->state[0]; B = ctx->state[1]; C = ctx->state[2]; D = ctx->state[3];
    E = ctx->state[4]; F = ctx->state[5]; G = ctx->state[6]; H = ctx->state[7];

    for (int j = 0; j < 64; j++) {
        uint32_t SS1 = ROTL((ROTL(A, 12) + E + ROTL(T(j), j % 32)) & 0xFFFFFFFF, 7);
        uint32_t SS2 = SS1 ^ ROTL(A, 12);
        uint32_t TT1 = (FF(j, A, B, C) + D + SS2 + W1[j]) & 0xFFFFFFFF;
        uint32_t TT2 = (GG(j, E, F, G) + H + SS1 + W[j]) & 0xFFFFFFFF;
        D = C;
        C = ROTL(B, 9);
        B = A;
        A = TT1;
        H = G;
        G = ROTL(F, 19);
        F = E;
        E = P0(TT2);
    }

    ctx->state[0] ^= A;
    ctx->state[1] ^= B;
    ctx->state[2] ^= C;
    ctx->state[3] ^= D;
    ctx->state[4] ^= E;
    ctx->state[5] ^= F;
    ctx->state[6] ^= G;
    ctx->state[7] ^= H;
}

void sm3_update(SM3_CTX* ctx, const uint8_t* data, size_t len) {
    size_t i = 0, fill = ctx->count / 8 % 64;
    ctx->count += len * 8;

    if (fill) {
        size_t left = 64 - fill;
        if (len < left) {
            memcpy(ctx->buf + fill, data, len);
            return;
        }
        else {
            memcpy(ctx->buf + fill, data, left);
            sm3_compress(ctx, ctx->buf);
            i += left;
        }
    }

    for (; i + 64 <= len; i += 64) {
        sm3_compress(ctx, data + i);
    }

    if (i < len) {
        memcpy(ctx->buf, data + i, len - i);
    }
}

void sm3_final(SM3_CTX* ctx, uint8_t digest[32]) {
    size_t fill = ctx->count / 8 % 64;
    ctx->buf[fill++] = 0x80;
    if (fill > 56) {
        memset(ctx->buf + fill, 0, 64 - fill);
        sm3_compress(ctx, ctx->buf);
        fill = 0;
    }
    memset(ctx->buf + fill, 0, 56 - fill);

    uint64_t bitlen = ctx->count;
    for (int i = 0; i < 8; i++) {
        ctx->buf[63 - i] = (uint8_t)(bitlen >> (8 * i));
    }

    sm3_compress(ctx, ctx->buf);

    for (int i = 0; i < 8; i++) {
        digest[4 * i] = (ctx->state[i] >> 24) & 0xFF;
        digest[4 * i + 1] = (ctx->state[i] >> 16) & 0xFF;
        digest[4 * i + 2] = (ctx->state[i] >> 8) & 0xFF;
        digest[4 * i + 3] = (ctx->state[i]) & 0xFF;
    }
}

// 测试函数
void sm3(const uint8_t* data, size_t len, uint8_t hash[32]) {
    SM3_CTX ctx;
    sm3_init(&ctx);
    sm3_update(&ctx, data, len);
    sm3_final(&ctx, hash);
}

// 打印16进制
void print_hex(const uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
}


int main() {
    const char* msg = "abc";
    uint8_t digest[32];
    sm3((const uint8_t*)msg, strlen(msg), digest);
    printf("SM3(\"abc\") = ");
    print_hex(digest, 32);
    return 0;
}
