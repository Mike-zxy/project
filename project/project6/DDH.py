# -*- coding: utf-8 -*-
from phe import paillier
from hashlib import sha256
from ecdsa import SECP256k1, ellipticcurve
import random

# 群设置（DDH 组）
curve = SECP256k1
G = curve.generator
order = G.order()

def hash_to_group(val: str) -> ellipticcurve.Point:
    """ Hash string to group element (curve point) """
    while True:
        h = sha256(val.encode()).hexdigest()
        x = int(h, 16) % order
        try:
            point = x * G
            return point
        except Exception:
            val += "_"

#  P2 密钥设置Paillier
pk, sk = paillier.generate_paillier_keypair()

# P1 实现
class Party1:
    def __init__(self, V):
        self.k1 = random.randint(1, order - 1)
        self.V = V  # 字符串集合

    def round1_send(self):
        """ 发送 k1 * H(v_i) """
        return [hash_to_group(v) * self.k1 for v in self.V]

    def round3_compute_intersection_sum(self, Z_set, encrypted_pairs):
        J = []
        for point, ciphertext in encrypted_pairs:
            double_exp = point * self.k1
            if any(double_exp == z for z in Z_set):
                J.append(ciphertext)

        if J:
            result = J[0]
            for c in J[1:]:
                result += c
            return result
        else:
            return pk.encrypt(0)

# P2 实现 
class Party2:
    def __init__(self, W_T):
        self.k2 = random.randint(1, order - 1)
        self.W_T = W_T  # (w_j, t_j) 列表

    def round2_compute(self, received_set):
        Z = [pt * self.k2 for pt in received_set]
        encrypted_pairs = []
        for w, t in self.W_T:
            point = hash_to_group(w) * self.k2
            enc_t = pk.encrypt(t)
            encrypted_pairs.append((point, enc_t))
        return Z, encrypted_pairs

    def output_decrypt(self, result_ciphertext):
        return sk.decrypt(result_ciphertext)

if __name__ == "__main__":
    # 输入集合
    P1_set = ['alice', 'bob', 'charlie']
    P2_set = [('bob', 10), ('charlie', 20), ('david', 30)]

    # 实例化参与方
    p1 = Party1(P1_set)
    p2 = Party2(P2_set)

    # Round 1: P1 -> P2
    round1_out = p1.round1_send()

    # Round 2: P2 -> P1
    Z, encrypted_pairs = p2.round2_compute(round1_out)

    # Round 3: P1 -> P2
    cipher_result = p1.round3_compute_intersection_sum(Z, encrypted_pairs)

    # 解密输出
    intersection_sum = p2.output_decrypt(cipher_result)
    print(f"Intersection sum: {intersection_sum}")
