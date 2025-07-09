pragma circom 2.0.0;

include "node_modules/circomlib/circuits/bitify.circom";
include "node_modules/circomlib/circuits/comparators.circom";

// 预定义的轮常数 (为简化只展示前5轮，实际需要完整64轮)
const ARK = [
    [ // Round 0
        0x123456789abcdef0123456789abcdef0,
            0xfedcba9876543210fedcba9876543210,
            0xabcdef0123456789abcdef0123456789
    ],
        [ // Round 1
            0x56789abcdef0123456789abcdef01234,
                0x0fedcba9876543210fedcba987654321,
                0x9abcdef0123456789abcdef012345678
        ],
        // ... 完整实现需要64轮常数
];

// MDS矩阵 (3×3)
const MDS = [
    [0x1ec7b2a4d6d9f3b1, 0x1e2b3c4d5e6f7a8b, 0x1c3d5f7b9a2c4e6f],
        [0x1d2e3f4a5b6c7d8e, 0x1f2e3d4c5b6a7988, 0x1a2b3c4d5e6f7890],
        [0x1928374655aabbcc, 0x1a9b8c7d6e5f4a3b, 0x1fc7d9e8b6a5c4d3]
];

template FullRound(round) {
    signal input state[3];
    signal output out[3];

    // 1. 加轮常数
    signal addRC[3];
    for (var i = 0; i < 3; i++) {
        addRC[i] <= = state[i] + ARK[round][i];
    }

    // 2. S-box (x^5)
    signal sbox[3];
    for (var i = 0; i < 3; i++) {
        signal s0;
        signal s1;
        s0 <= = addRC[i] * addRC[i];         // x²
        s1 <= = s0 * s0;                     // x⁴
        sbox[i] <= = s1 * addRC[i];          // x⁵
    }

    // 3. MDS矩阵乘法
    for (var i = 0; i < 3; i++) {
        out[i] <= = MDS[i][0] * sbox[0] + MDS[i][1] * sbox[1] + MDS[i][2] * sbox[2];
    }
}

template PartialRound(round) {
    signal input state[3];
    signal output out[3];

    // 1. 加轮常数
    signal addRC[3];
    for (var i = 0; i < 3; i++) {
        addRC[i] <= = state[i] + ARK[round][i];
    }

    // 2. 部分S-box (只对第一个元素)
    signal sbox[3];
    signal s0;
    signal s1;
    s0 <= = addRC[0] * addRC[0];          // x²
    s1 <= = s0 * s0;                      // x⁴
    sbox[0] <= = s1 * addRC[0];           // x⁵
    sbox[1] <= = addRC[1];
    sbox[2] <= = addRC[2];

    // 3. MDS矩阵乘法
    for (var i = 0; i < 3; i++) {
        out[i] <= = MDS[i][0] * sbox[0] + MDS[i][1] * sbox[1] + MDS[i][2] * sbox[2];
    }
}

template Poseidon2() {
    // 输入：256位隐私输入（哈希原象）
    signal input in;

    // 输出：256位公开输出（哈希值）
    signal output out[2];

    // 将256位输入分割为两个128位块
    component splitter = Num2Bits(256);
    splitter.in <= = in;

    signal highBits <= = splitter.out[128] +
        (splitter.out[129] << 1) +
        (splitter.out[130] << 2); // 实际需要循环展开128位

    signal lowBits <= = splitter.out[0] +
        (splitter.out[1] << 1) +
        (splitter.out[2] << 2); // 实际需要循环展开128位

    // 初始状态设置 [0, in_high, in_low]
    signal state[3];
    state[0] <= = 0;
    state[1] <= = highBits;
    state[2] <= = lowBits;

    // 轮函数处理 (4 full + 4 partial + 4 full)
    // 实际需要12轮完整实现，这里展示前2轮
    component fullRound0 = FullRound(0);
    fullRound0.state[0] <= = state[0];
    fullRound0.state[1] <= = state[1];
    fullRound0.state[2] <= = state[2];

    component fullRound1 = FullRound(1);
    fullRound1.state[0] <= = fullRound0.out[0];
    fullRound1.state[1] <= = fullRound0.out[1];
    fullRound1.state[2] <= = fullRound0.out[2];

    // ... 中间省略8轮

    // 最终状态的前两个元素作为输出
    out[0] <= = fullRound1.out[1]; // 高位
    out[1] <= = fullRound1.out[2]; // 低位
}

component main = Poseidon2();