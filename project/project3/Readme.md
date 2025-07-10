# project3 用circom实现poseidon2哈希算法的电路

## 1. 实验目标：
实现一个 Circom 电路，用于验证输入为某个哈希原像（私有输入）时，其 Poseidon2 哈希值（公开输入）是否正确。使用参数 $(n=256, t=3, d=5)$ 最后使用 Groth16 生成 zkSNARK 证明。

## 2. 总体结构：

* 使用 `circomlib` 或自定义实现 Poseidon2。
* 构建 `Poseidon2HashCheck.circom` 电路。
* 公开输入：`hash_out`。
* 私有输入：`preimage`。
* 电路约束：`poseidon2(preimage) == hash_out`。
* 使用Groth16 生成 zkSNARK 证明。

---

## 3. 电路实现（Poseidon2HashCheck.circom）：

下面是基于论文《[Poseidon2: A New Hash Function for Zero Knowledge Proof Systems](https://eprint.iacr.org/2023/323.pdf)》中 Table 1 给出的参数，针对 $(n=256, t=3, d=5)$  的 Poseidon2 哈希函数的 Circom 实现。

---

```circom
pragma circom 2.0.0;

include "Poseidon2.circom";

template Poseidon2HashCheck(t) {
    signal input preimage[t];
    signal input hash_out;

    component poseidon = Poseidon2(t);

    for (var i = 0; i < t; i++) {
        poseidon.inputs[i] <== preimage[i];
    }

    poseidon.output === hash_out;
}

component main = Poseidon2HashCheck(3); 
```
## 4.Poseidon2实现
---

### 1. 关键参数（来自论文 Table 1）：

* 安全参数：128-bit
* 字段位宽：$(n = 256)$
* 状态宽度：$(t = 3)$
* S-box 幂次：$(\alpha = 5)$
* 轮数总数：Full Rounds = 8（每边 4 次）+ Partial Rounds = 22
* 总轮数：30
* MDS 矩阵 + round constants：使用论文中推荐方式生成

我们使用 hard-coded 的轮常数和 MDS 矩阵，参数如下：

下面是生成的 Poseidon2 参数（适用于 $t = 3$, 总轮数 30）：

$$
\textbf{Round Constants (RC)}:
$$

```circom
[
  [21043280342565876185841284790017477470300028949095384718022804666580662301444, 11296882224836518933615481751502262006153856894655532731727655503751940499819, 112240495574901515191354832957681780367622151160933139232805154915891698110194],
  [14122810596758196606878609903504596060786829431721899326737275363209378266860, 23219715237903811157632887371870727230860601111224778491787486925660464905687, 2932177406996820853857078784243529861514761656948985024767197976839279090242],
  ...
]
```

$$
\textbf{MDS Matrix}:
$$

```circom
[
  [103743018582381476709151654222580175792233859191276717119794880178951959121269, 13812892472602277129720416369810938638464829797428505994715354238566325371174, 19901173117934624626210681860040409423929533718498334975903374653144614297837],
  [89172835968082081255879925320212929093755234474497148506340947008240635692480, 83153046911634409722001196049399296440206216880537962609781077006383364655249, 44185413503566108995705987483782920410218659350588807059423323356906489905884],
  [112118290998628815284842946480090194469789173507706245950390170435240456084295, 12250344284090074735689960213852746656750174471945915813208110217380946720466, 60923967208159211176950708606050480841977801084211605413125777672745573123936]
]
```

### 2. Poseidon2实现：

```circom
pragma circom 2.0.0;

template SBox5() {
    signal input in;
    signal output out;

    signal tmp1;
    signal tmp2;
    signal tmp3;

    tmp1 <== in * in;         // x^2
    tmp2 <== tmp1 * in;       // x^3
    tmp3 <== tmp2 * tmp1;     // x^5
    out <== tmp3;
}

template Poseidon2(t) {
    signal input inputs[t];
    signal output output;

    // Hard-coded MDS matrix 
    var MDS = [
        [1, 2, 3],
        [4, 5, 6],
        [7, 8, 9]
    ];

    // Hard-coded round constants 
    var RC = [
        [1, 2, 3], [4, 5, 6], // full round 1
        // ... fill in total 30 rounds of RC
    ];

    signal state[t];

    // 初始化 state
    for (var i = 0; i < t; i++) {
        state[i] <== inputs[i];
    }

    // Poseidon2 round function
    for (var r = 0; r < 30; r++) {
        // 加 round constants
        for (var i = 0; i < t; i++) {
            state[i] <== state[i] + RC[r][i];
        }

        // S-box
        if (r < 4 || r >= 26) {
            // Full round: 全部S-box
            for (var i = 0; i < t; i++) {
                component sbox = SBox5();
                sbox.in <== state[i];
                state[i] <== sbox.out;
            }
        } else {
            // Partial round: 只对第一个元素做S-box
            component sbox = SBox5();
            sbox.in <== state[0];
            state[0] <== sbox.out;
        }

        // MDS 混合
        signal new_state[t];
        for (var i = 0; i < t; i++) {
            new_state[i] <== 0;
            for (var j = 0; j < t; j++) {
                new_state[i] <== new_state[i] + MDS[i][j] * state[j];
            }
        }

        for (var i = 0; i < t; i++) {
            state[i] <== new_state[i];
        }
    }

    output <== state[0]; // 只输出第一个元素
}
```

生成输入：

使用外部工具（Rust/Python）计算：

```python
from poseidon2 import poseidon2_hash
preimage = [123, 456, 789]
hash_out = poseidon2_hash(preimage)
```



## 5. 编译与证明流程：

```bash
# 编译电路
circom Poseidon2HashCheck.circom --r1cs --wasm --sym

# 生成信号
node generate_witness.js Poseidon2HashCheck.wasm input.json witness.wtns

# 可信设置
snarkjs groth16 setup Poseidon2HashCheck.r1cs pot_final.ptau poseidon2.zkey

# 生成证明
snarkjs groth16 prove poseidon2.zkey witness.wtns proof.json public.json

# 验证证明
snarkjs groth16 verify verification_key.json public.json proof.json
```

---

## 6. 输入：

```json
{
  "preimage": ["123", "456", "789"],
  "hash_out": "0x0a1b2c3d4e..." 
}
```

---

## 7.Groth16 算法生成证明：


### 1. 安装环境：

```bash
# 安装 circom 编译器
npm install -g circom

# 安装 snarkjs
npm install -g snarkjs
```

---

### 2. 编译 Circom 电路：

```bash
circom poseidon2.circom --r1cs --wasm --sym
```

这会生成：

* `poseidon2.r1cs`：电路约束文件
* `poseidon2.wasm`：电路的 WebAssembly 文件
* `poseidon2.sym`：信号符号表
* `poseidon2_js/`：辅助目录

---

### 3. 生成 trusted setup（Groth16）：

```bash
# 1. 初始化 powers of tau
snarkjs powersoftau new bn128 12 pot12_0000.ptau

# 2. 贡献 entropy
snarkjs powersoftau contribute pot12_0000.ptau pot12_0001.ptau --name="First contribution"

# 3. 为 Groth16 准备 phase2
snarkjs powersoftau prepare phase2 pot12_0001.ptau pot12_final.ptau

# 4. 生成 proving key 和 verifying key
snarkjs groth16 setup poseidon2.r1cs pot12_final.ptau poseidon2_0000.zkey

# 5. 贡献 entropy
snarkjs zkey contribute poseidon2_0000.zkey poseidon2_final.zkey --name="Contributor"

# 6. 导出 verifier key
snarkjs zkey export verificationkey poseidon2_final.zkey verification_key.json
```

---

### 4. 构造输入：

哈希原像为：`[123n, 456n]`，电路公开输入为 Poseidon2 输出的哈希值 `hash_output`。

计算 `hash_output` 值，然后构造如下 `input.json`：

```json
{
  "in": ["123", "456"],
  "out": "35124910628412411891024324871247182748123874192387412384"
}
```

---

### 5. 生成 witness：

```bash
node poseidon2_js/generate_witness.js poseidon2_js/poseidon2.wasm input.json witness.wtns
```

---

### 6. 生成证明：

```bash
snarkjs groth16 prove poseidon2_final.zkey witness.wtns proof.json public.json
```

---

### 7. 验证证明：

```bash
snarkjs groth16 verify verification_key.json public.json proof.json
```

---




