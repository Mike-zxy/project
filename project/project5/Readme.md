# project5 SM2的软件实现优化

该项目实现了 SM2 椭圆曲线密码算法中使用的各种辅助函数，包括数据类型转换、KDF 密钥派生函数、模逆运算以及椭圆曲线加法等，适用于国密 SM2 签名与加解密实验。

---

## 项目结构

- `int_to_bytes` / `bytes_to_int`：整数与字节串互转
- `bits_to_bytes` / `bytes_to_bits`：比特串与字节串互转
- `fielde_to_bytes` / `bytes_to_fielde`：域元素与字节串互转
- `point_to_bytes` / `bytes_to_point`：椭圆曲线点与字节串互转（未压缩格式）
- `KDF`：密钥派生函数（基于 SM3）
- `calc_inverse`：模逆运算（扩展欧几里得算法）
- `frac_to_int`：分式模运算变换为整数
- `add_point` / `double_point`：椭圆曲线上的点加与倍点计算
- 多种辅助函数：如 `bytes_to_hex`, `hex_to_bytes`, `fielde_to_bits`, `point_to_bits`, `int_to_bits` 等

---


##  依赖

* Python 3.6+
* [`gmssl`](https://github.com/duanhongyi/gmssl)：用于调用 SM3 哈希函数

安装：

```bash
pip install gmssl
```

---

##  椭圆曲线参数

```text
    p = eval('0x' + '8542D69E 4C044F18 E8B92435 BF6FF7DE 45728391 5C45517D 722EDB8B 08F1DFC3'.replace(' ', ''))
    a = eval('0x' + '787968B4 FA32C3FD 2417842E 73BBFEFF 2F3C848B 6831D7E0 EC65228B 3937E498'.replace(' ', ''))
    b = eval('0x' + '63E4C6D3 B23B0C84 9CF84241 484BFE48 F61D59A5 B16BA06E 6E12D1DA 27C5249A'.replace(' ', ''))
    h = 1
    xG = eval('0x' + '421DEBD6 1B62EAB6 746434EB C3CC315E 32220B3B ADD50BDC 4C4E6C14 7FEDD43D'.replace(' ', ''))
    yG = eval('0x' + '0680512B CBB42C07 D47349D2 153B70C4 E5D7FDFC BFA36EA1 A85841B9 E46E09A2'.replace(' ', ''))
    G = (xG, yG)            # G 是基点
    n = eval('0x' + '8542D69E 4C044F18 E8B92435 BF6FF7DD 29772063 0485628D 5AE74EE7 C32E79B7'.replace(' ', ''))
    args = (p, a, b, h, G, n)           
```


---
##  使用示例
算法首先获取椭圆曲线系统参数
```python
# 椭圆曲线系统参数args(p, a, b, h, G, n)的获取。
def get_args():
    p = eval('0x' + '8542D69E 4C044F18 E8B92435 BF6FF7DE 45728391 5C45517D 722EDB8B 08F1DFC3'.replace(' ', ''))
    a = eval('0x' + '787968B4 FA32C3FD 2417842E 73BBFEFF 2F3C848B 6831D7E0 EC65228B 3937E498'.replace(' ', ''))
    b = eval('0x' + '63E4C6D3 B23B0C84 9CF84241 484BFE48 F61D59A5 B16BA06E 6E12D1DA 27C5249A'.replace(' ', ''))
    h = 1
    xG = eval('0x' + '421DEBD6 1B62EAB6 746434EB C3CC315E 32220B3B ADD50BDC 4C4E6C14 7FEDD43D'.replace(' ', ''))
    yG = eval('0x' + '0680512B CBB42C07 D47349D2 153B70C4 E5D7FDFC BFA36EA1 A85841B9 E46E09A2'.replace(' ', ''))
    G = (xG, yG)            # G 是基点
    n = eval('0x' + '8542D69E 4C044F18 E8B92435 BF6FF7DD 29772063 0485628D 5AE74EE7 C32E79B7'.replace(' ', ''))
    args = (p, a, b, h, G, n)           # args是存储椭圆曲线参数的元组。
    return args


# 密钥获取。本程序中主要是消息接收方B的公私钥的获取。
def get_key():
    xB = eval('0x' + '435B39CC A8F3B508 C1488AFC 67BE491A 0F7BA07E 581A0E48 49A5CF70 628A7E0A'.replace(' ', ''))
    yB = eval('0x' + '75DDBA78 F15FEECB 4C7895E2 C1CDF5FE 01DEBB2C DBADF453 99CCF77B BA076A42'.replace(' ', ''))
    PB = (xB, yB)           # PB是B的公钥
    dB = eval('0x' + '1649AB77 A00637BD 5E2EFE28 3FBF3535 34AA7F7C B89463F2 08DDBC29 20BB0DA0'.replace(' ', ''))
    # dB是B的私钥
    key_B = (PB, dB)
    return key_B
```
<img width="1202" height="414" alt="image" src="https://github.com/user-attachments/assets/06cb18a7-9c49-47d9-8136-42763edc4a37" />

然后输入待加密的明文，例如123456789
<img width="1217" height="105" alt="image" src="https://github.com/user-attachments/assets/ceffc378-bdbf-44e7-a151-f420ec173a90" />

对明文进行加密
```python
def encry_sm2(args, PB, M):
    p, a, b, h, G, n = args         # 序列解包
    M_bytes = bytes(M, encoding='ascii')
    #print("A1：用随机数发生器产生随机数k∈[1,n-1]")
    k = random.randint(1, n-1)
    k_hex = hex(k)[2:]          # k_hex 是k的十六进制串形式
    #print("生成的随机数是：", k_hex)
    #print("\nA2:计算椭圆曲线点C1=[k]G=(x1,y1)，将C1的数据类型转换为比特串")
    C1 = mult_point(G, k, p, a)
    #print("椭圆曲线点C1=[k]G=(x1,y1)的坐标是:", tuple(map(hex, C1)))
    C1_bits = point_to_bits(C1)
    #print("椭圆曲线点C1=[k]G=(x1,y1)的坐标的比特串形式是:", C1_bits)
    #print("\nA3：计算椭圆曲线点S = [h]PB")
    S = mult_point(PB, h, p, a)
    if S == 0:
        raise Exception("计算得到的S是无穷远点")
    #print("A3椭圆曲线点S = [h]PB的坐标是:", tuple(map(hex, S)))
    #print("\nA4：计算椭圆曲线点[k]PB=(x2,y2)，将坐标x2、y2 的数据类型转换为比特串")
    x2, y2 = mult_point(PB, k, p, a)
    #print("椭圆曲线点[k]PB=(x2,y2)的坐标是:", tuple(map(hex, (x2, y2))))
    x2_bits = fielde_to_bits(x2)
    #print("x2的比特串形式是：", x2_bits)
    y2_bits = fielde_to_bits(y2)
    #print("y2的比特串形式是：", y2_bits)
    #print("\nA5：计算t=KDF(x2 ∥ y2, klen)，若t为全0比特串，则返回A1")
    M_hex = bytes_to_hex(M_bytes)
    klen = 4 * len(M_hex)
    #print("明文消息的比特串长度klen是：", klen)
    t = KDF(x2_bits + y2_bits, klen)
    #print("通过KDF算法计算得到的t=KDF(x2 ∥ y2, klen)是：", t)
    if eval('0b' + t) == 0:
        raise Exception("KDF返回了全零串，请检查KDF算法！")
    t_hex = bits_to_hex(t)
    #print("t的十六进制表示形式是：", t_hex)
    #print("\nA6：计算计算C2 = M ⊕ t")
    C2 = eval('0x' + M_hex + '^' + '0b' + t)
    #print("计算的C2是：", hex(C2)[2:])
    #print("\nA7：计算C3 = Hash(x2 ∥ M ∥ y2)")
    x2_bytes = bits_to_bytes(x2_bits)
    y2_bytes = bits_to_bytes(y2_bits)
    hash_list = [i for i in x2_bytes + M_bytes + y2_bytes]
    C3 = sm3.sm3_hash(hash_list)
    #print("计算的C3 = Hash(x2 ∥ M ∥ y2)是：", C3)
    #print("\nA8：输出密文C = C1 ∥ C2 ∥ C3")
    C1_hex = bits_to_hex(C1_bits)
    C2_hex = hex(C2)[2:]
    C3_hex = C3
    C_hex = C1_hex + C2_hex + C3_hex
    print("加密得到的密文是：", C_hex)
    return C_hex
```
对密文进行解密
```python
# 解密算法。接收的参数为椭圆曲线系统参数args(p, a, b, h, G, n)。dB是B的私钥，C是密文消息。
def decry_sm2(args, dB, C):
    p, a, b, h, G, n = args
    #print("B1：从C中取出比特串C1，将C1的数据类型转换为椭圆曲线上的点，验证C1是否满足椭圆曲线方程，若不满足则报错并退出；")
    l = ceil(log(p, 2)/8)         # l是一个域元素（比如一个点的横坐标）转换为字节串后的字节长度.则未压缩的形式下秘闻第一部分C1长度为2l+1
    bytes_l1 = 2*l+1
    #print("计算得到的C1的字节串长度是：", bytes_l1)
    hex_l1 = bytes_l1 * 2            # hex_l1是密文第一部分C1的十六进制串的长度
    C_bytes = hex_to_bytes(C)
    #print("将十六进制密文串转换为字节串是：", C_bytes)
    C1_bytes = C_bytes[0:2*l+1]
    #print("从密文字节串中取出的C1的字节串是：", C1_bytes)
    C1 = bytes_to_point(C1_bytes)
    #print("将C1字节串转换为椭圆曲线上的点是：", C1)
    if not on_curve(args, C1):          # 检验C1是否在椭圆曲线上
        raise Exception("在解密算法B1中，取得的C1不在椭圆曲线上！")
    x1, y1 = C1[0], C1[1]
    x1_hex, y1_hex = fielde_to_hex(x1), fielde_to_hex(y1)
    #print("C1坐标用的十六进串形式表示是：", (x1_hex, y1_hex))
    #print("\nB2：计算椭圆曲线点S=[h]C1，若S是无穷远点，则报错并退出；")
    S = mult_point(C1, h, p, a)
    #print("计算得到的S是：", S)
    if S == 0:
        raise Exception("在解密算法B2中，S是无穷远点！")
    xS, yS = S[0], S[1]
    xS_hex, yS_hex = fielde_to_hex(xS), fielde_to_hex(yS)
    #print("S的坐标用十六进制串形式表示是：", (xS_hex, yS_hex))
    #print("\nB3：计算[dB]C1=(x2,y2)，将坐标x2、y2的数据类型转换为比特串；")
    temp = mult_point(C1, dB, p, a)
    x2, y2 = temp[0], temp[1]
    x2_hex, y2_hex = fielde_to_hex(x2), fielde_to_hex(y2)
    #print("解密得到的[dB]C1=(x2,y2)的十六进制串形式是：", (x2_hex, y2_hex))
    #print("\nB4:计算t=KDF(x2 ∥ y2, klen)，若t为全0比特串，则报错并退出；")
    hex_l3 = 64           # hex_l3是密文第三部分C3的十六进制串的长度。C3是通过SM3得到的hash值，是64位十六进制串。
    hex_l2 = len(C) - hex_l1 - hex_l3           # hex_l2是密文第二部分C2的十六进制串的长度。
    klen = hex_l2 * 4           # klen是密文C2中比特串的长度
    #print("计算的C2的比特串长度klen是：", klen)
    x2_bits, y2_bits = hex_to_bits(x2_hex), hex_to_bits(y2_hex)
    t = KDF(x2_bits + y2_bits, klen)
    #print("计算的t=KDF(x2 ∥ y2, klen)是：", t)
    if eval('0b' + t) == 0:
        raise Exception("在解密算法B4中，得到的t是全0串！")
    t_hex = bits_to_hex(t)
    #print("t的十六进制串形式是：", t_hex)
    #print("\nB5：从C中取出比特串C2，计算M′ = C2 ⊕ t；")
    C2_hex = C[hex_l1: -hex_l3]
    #print("C2的十六进制串形式是：", C2_hex)
    M1 = eval('0x' + C2_hex + '^' + '0x' + t_hex)           # M1是M'，M′ = C2 ⊕ t
    M1_hex = hex(M1)[2:].rjust(hex_l2, '0')         # 注意位数要一致
    #print("计算的M′ = C2 ⊕ t是：", M1_hex)
    #print("\nB6：计算u = Hash(x2 ∥ M′ ∥ y2)，从C中取出比特串C3，若u != C3，则报错并退出；")
    M1_bits = hex_to_bits(M1_hex)
    cmp_bits = x2_bits + M1_bits + y2_bits          # cmp_bits存储用于计算哈希值以对比C3的二进制串
    cmp_bytes = bits_to_bytes(cmp_bits)
    cmp_list = [i for i in cmp_bytes]
    u = sm3.sm3_hash(cmp_list)          # u中存储
    #print("计算的u = Hash(x2 ∥ M′ ∥ y2)是：", u)
    C3_hex = C[-hex_l3:]
    #print("从C中取出的C3的十六进制形式是：", C3_hex)
    if u != C3_hex:
        raise Exception("在解密算法B6中，计算的u与C3不同！")
    #print("\nB7：输出明文M′")
    M_bytes = hex_to_bytes(M1_hex)
    M = str(M_bytes, encoding='ascii')
    print("解密出的明文是：", M)
    return M
```
比较明文密文是否一致
结果来看加解密一致
<img width="1223" height="263" alt="image" src="https://github.com/user-attachments/assets/2fef311a-11a8-4b39-a47e-cab60f1a30d7" />

---
