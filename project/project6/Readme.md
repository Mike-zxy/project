 **Project6.实现DDH-based Private Intersection-Sum Protocol**



实验要求：

1. 使用满足 DDH 假设的群 G\mathbb{G}，如椭圆曲线群或素数阶有限域上的群。
2. 使用加性同态加密（如 Paillier）来支持密文求和。
3. 实现多轮协议模拟两方 P1P_1 和 P2P_2 的交互过程。

------

> 

这个协议是一个基于 DDH 假设的**私密交集求和协议**，它的目标是：

------

### ✅ 协议目标：

在不泄露集合内容的前提下，**两个参与方（P1 和 P2）计算它们集合交集对应值的加和总和**。

------

### ✅ 双方输入和输出：

- **P1（Party 1）输入：**

  - 一个集合 V={v1,v2,…,vn}V = \{v_1, v_2, \dots, v_n\}，元素为字符串（如名字）。

- **P2（Party 2）输入：**

  - 一个键值对集合 W={(w1,t1),(w2,t2),…,(wm,tm)}W = \{(w_1, t_1), (w_2, t_2), \dots, (w_m, t_m)\}，键为字符串（如名字），值为整数（如得分、余额等）。

- **输出结果（仅给 P2）：**

  - 所有 vi=wjv_i = w_j 的情况，将对应的 tjt_j 相加后输出：

    ∑vi=wjtj\sum_{v_i = w_j} t_j

------

### ✅ 协议流程简述（三轮交互）：

1. **Round 1（P1 → P2）**:
   - P1 随机选择密钥 k1k_1
   - 对每个 viv_i，计算 k1⋅H(vi)k_1 \cdot H(v_i)，并发送给 P2
      （其中 HH 是 hash-to-curve 函数）
2. **Round 2（P2 → P1）**:
   - P2 随机选择密钥 k2k_2
   - 对收到的点 k1⋅H(vi)k_1 \cdot H(v_i)，再计算 k1k2⋅H(vi)k_1 k_2 \cdot H(v_i)，形成集合 ZZ
   - 对于每个 (wj,tj)(w_j, t_j)，也计算 k2⋅H(wj)k_2 \cdot H(w_j)
   - 同时将 tjt_j 用 Paillier 加密后附在该点后，形成 (k2⋅H(wj),Enc(tj))(k_2 \cdot H(w_j), Enc(t_j)) 对发送回 P1
3. **Round 3（P1 → P2）**:
   - P1 再对 P2 发来的点做 k1k_1 次幂变换，得到 k1k2⋅H(wj)k_1 k_2 \cdot H(w_j)
   - 找出和自己计算出的 ZZ 中相等的点（即交集）
   - 将这些对应的 Paillier 密文加和后返回给 P2
4. **P2 解密结果**，得到交集对应的 tjt_j 总和。

------

### ✅ 结果说明：

该协议完成了：

- **P1 不知道交集内容和 t 值**
- **P2 不知道 P1 的集合内容**
- 只有 **交集部分的 t 值总和** 会被 P2 知道

------

### 📌 实际应用场景：

- 两个公司要统计共同客户的消费总额，但都不想泄露各自客户名单
- 医疗隐私数据分析：两家医院想知道共同病人的总治疗费用
- 广告投放交叉分析：品牌和平台分析目标用户交叉消费总额

------

如你想我进一步将这个协议与 PSI（Private Set Intersection）或 PSI-Sum（交集求和）类协议进行比较，我也可以详细解释。是否还需要我将流程转成文字版说明或图解？

###  第一步：安装依赖

```bash
pip install phe ecdsa
```

------

### 第二步：协议实现代码

```python
from phe import paillier
from hashlib import sha256
from ecdsa import SECP256k1, ellipticcurve, numbertheory
import random

# ------------------------ 群设置 ------------------------
curve = SECP256k1
G = curve.generator
order = G.order()

def hash_to_group(val: str) -> ellipticcurve.Point:
    """ Hash string to group element (curve point) """
    while True:
        h = sha256(val.encode()).hexdigest()
        x = int(h, 16) % order
        try:
            return x * G
        except Exception:
            val = val + "_"

# ------------------------ P2 密钥设置 ------------------------
pk, sk = paillier.generate_paillier_keypair()

# ------------------------ P1：初始化 ------------------------
class Party1:
    def __init__(self, V):
        self.k1 = random.randint(1, order - 1)
        self.V = V  # list of strings

    def round1_send(self):
        return [hash_to_group(v) * self.k1 for v in self.V]

    def round3_compute_intersection_sum(self, Z_set, encrypted_pairs):
        J = []
        for idx, (point, ciphertxt) in enumerate(encrypted_pairs):
            # Exponentiate again with k1
            double_exp = point * self.k1
            if any(double_exp == z for z in Z_set):
                J.append(ciphertxt)

        # Homomorphic addition
        if J:
            result = J[0]
            for c in J[1:]:
                result += c
            return result
        else:
            return pk.encrypt(0)  # 空交集

# ------------------------ P2：初始化 ------------------------
class Party2:
    def __init__(self, W_T):
        self.k2 = random.randint(1, order - 1)
        self.W_T = W_T  # list of (w_j, t_j) pairs

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
```

------

### 示例运行：

```python
# 示例输入
P1_set = ['alice', 'bob', 'charlie']
P2_set = [('bob', 10), ('charlie', 20), ('david', 30)]

# 实例化参与方
p1 = Party1(P1_set)
p2 = Party2(P2_set)

# Round 1
round1_out = p1.round1_send()

# Round 2
Z, encrypted_pairs = p2.round2_compute(round1_out)

# Round 3
cipher_result = p1.round3_compute_intersection_sum(Z, encrypted_pairs)

# Output
intersection_sum = p2.output_decrypt(cipher_result)
print(f"Intersection sum: {intersection_sum}")
```

------

### ✅ 输出示例：

```
Intersection sum: 30
```

说明：这里 `P1 ∩ P2 = {bob, charlie}`，对应 `10 + 20 = 30`。

------

如果你需要：

- 使用真实 ECC 库模拟整个群；
- 更换为素数域群；
- 模拟真实网络通信；
- 实现 security proof 模拟器（SIM1、SIM2）；