# 项目介绍
本项目是对国密 SM2 椭圆曲线公钥密码算法的优化实现与安全性分析。内容涵盖 SM2 的加解密、基于 SM2 的签名与验证，同时实现了多种攻击场景的POC，展示了在不同参数复用情况下的私钥恢复技术。

## 项目结构
```bash
.
├── sm2.py # SM2 加密、签名实现
└── test.py # 正确性测试与攻击 PoC 实现
```

- 实现了 **SM2 加密与解密**
- 对 SM2 算法进行了软件层面的性能优化
- 实现了 **SM2 签名与验证**
- 演示了以下几种针对签名的攻击模式：
  - 泄露随机数 `k` 后恢复私钥
  - 两次签名复用相同 `k` 恢复私钥
  - 不同用户复用相同 `k` 恢复私钥
  - SM2 与 ECDSA 协议间复用同一 `d` 和 `k` 造成的密钥泄露

## 项目使用说明

### 环境依赖

```bash
pip install gmssl ecdsa
```

### 运行测试
```bash
python3 test.py
```

# 算法原理

SM2 是中国国家密码管理局发布的国家椭圆曲线密码标准，基于 ECC ，主要用于加密、数字签名等场景

SM2 使用的椭圆曲线形式为：

$$
y^2 = x^3 + ax + b (\bmod p)
$$

其中，所用的参数包括：

- 素数模 `p`（256 位）
- 曲线系数 `a` 和 `b`
- 基点 `G`、阶 `n`
- 余因子 `h = 1`


## SM2 加密算法实现与优化

### 密钥生成

1. 随机生成一个私钥 $d$ ，满足： $1 \leq d \leq n-1$
2. 根据椭圆曲线生成对应的公钥 $P = d\cdot G$

- **私钥（Private Key）**：大整数 $d$
- **公钥（Public Key）**：椭圆曲线点 $P = (x, y)$

### 加密流程

1. 随机生成 $k$
2. 计算 

$$
C_1 = k \cdot G
$$

3. 计算共享点 

$$
(x_2, y_2) = k\cdot P
$$

4. 使用 KDF 函数派生加密密钥 $t$

5. 用 $t$ 加密明文

$$
C_2 = M \oplus t
$$

6. 生成摘要

$$
C_3 = Hash(x_2 || M || y_2)
$$

7. 生成密文

$$
C = C_1 || C_2 || C_3
$$

### 解密流程

1. 提取 $C_1$ 并验证其是否有效
2. 计算

$$
 (x_2, y_2) = d \cdot C_1
$$

3. 使用 KDF 函数生成解密密钥 $t$
4. 解密密文

$$
M' = C_2 \oplus t
$$

5. 验证摘要是否一致

$$
Hash(x_2 || M' || y_2) == C_3
$$

6. 若一致，输出明文 $M'$ ；否则解密失败

### 软件优化

#### 1. 使用 NAF 加速标量乘法

标量乘法是椭圆曲线密码算法的核心计算之一，即计算 $k \cdot G$ 。直接采用二进制展开法会导致较多的点加操作。

**非相邻形式（NAF, Non-Adjacent Form）** 是一种稀疏表示，它通过将整数 $k$ 表示为：

$$
k = \sum (ki · 2^i)
$$

其中 $ki \in \{0, ±1\}$，且任意两个非零 $ki$ 之间至少间隔一个 0

由于非零系数的数量显著减少，可降低椭圆曲线加法操作，从而提升标量乘法性能。

#### 2. 预计算与缓存提升模运算效率

在椭圆曲线计算中，点乘、模逆、模乘、幂运算频繁出现。通过提前计算和缓存常用值，可以显著降低重复运算的开销，是一种空间换时间的思想。

常见缓存策略包括：

- **模逆缓存表**：缓存计算过的值，避免重复求某些值的模逆
- **幂值缓存表**：提前计算 $2^i \bmod p (0 \leq i < 256)$，供后续快速查表使用
- **点乘缓存表**: 提前计算 $G,2\cdot G, \cdots , (2^n-1) \cdot G$，将椭圆曲线上的点乘操作转化为查表操作

这类缓存可加快椭圆曲线上点运算、标量分解、NAF 编码等操作。

## 基于 SM2 的签名

### 预计算

$$
Z_A = H(ENTL_A||ID_A||a||b||x_G||y_G||x_A||y_A)
$$

### 密钥生成

$$
P = d\cdot G
$$

$P$ 为公钥、 $d$ 为私钥

### 签名流程

1. 对消息 $M$ 计算杂凑值：

$$
e = \text{Hash}(Z_A||M)
$$

2. 随机选取整数 $k$，满足： $1 \leq k \leq n - 1$ 

3. 计算点：

$$
(x_1, y_1) = k \cdot G
$$

4. 计算 $r$：

$$
r = (e + x_1) \bmod n
$$

- 如果 $r = 0$ 或 $r + k = n$，则返回步骤 2 重新选择 $k$

5. 计算 $s$：

$$
s = \left[ (1 + d)^{-1} \cdot (k - r \cdot d) \right] \bmod n
$$

- 如果 $s = 0$，重新选择 $k$

6. 签名结果为： $(r, s)$ 

### 验证流程

给定消息 $M$，签名 $(r, s)$，公钥 $P$，验证如下：

1. 验证签名参数是否满足：

$$
1 \leq r < n, \quad 1 \leq s < n
$$

2. 计算杂凑值：

$$
e = \text{Hash}(M)
$$

3. 计算中间值：

$$
t = (r + s) \bmod n
$$

- 若 $t = 0$，则验证失败

4. 计算椭圆曲线点：

$$
(x_1', y_1') = s \cdot G + t \cdot P
$$

5. 验证是否满足：

$$
r \stackrel{?}{=} (e + x_1') \bmod n
$$

- 若相等，则验证成功；否则失败


## SM2 签名算法误用攻击

### 1. 泄露 k ，可恢复私钥 d
#### 私钥恢复流程

1. 已知签名 $\sigma = (r, s)$ 和对应的随机数 $k$。
2. 计算中间值 $s + r$ 并求模逆：

$$
(s + r)^{-1} \bmod n
$$

3. 计算私钥 $d$：

$$
d = \left[ (s + r)^{-1} \cdot (k - s) \right] \bmod n
$$

#### 关键推导过程

从签名方程出发：

$$
s = \left[ (1 + d_A)^{-1} \cdot (k - r \cdot d_A) \right] \bmod n
$$

两边乘以 $(1 + d_A)$：

$$
s(1 + d_A) \equiv (k - r \cdot d_A) \pmod{n}
$$

展开并整理关于 $d_A$ 的线性方程：

$$
s + s \cdot d_A \equiv k - r \cdot d_A \pmod{n}
$$

$$
(s \cdot d_A + r \cdot d_A) \equiv (k - s) \pmod{n}
$$

$$
d_A \cdot (s + r) \equiv (k - s) \pmod{n}
$$

最终解得：

$$
d_A \equiv (s + r)^{-1} \cdot (k - s) \pmod{n}
$$

---


### 2. 同一用户两次签名用同一个 k ，可恢复私钥 d

#### 签名消息 $M_1$
1. 随机选取整数 $k$，满足： $1 \leq k \leq n - 1$ 

2. 计算椭圆曲线点：

$$
kG = (x, y)
$$

3. 计算$r_1$：

$$
r_1 = (\text{Hash}(Z_A||M_1) + x) \bmod n
$$

4. 计算$s_1$：

$$
s_1 = \left[(1 + d_A)^{-1} \cdot (k - r_1 \cdot d_A)\right] \bmod n
$$

#### 签名消息 $M_2$（重用相同k值）
1. 使用相同的$k$值，计算椭圆曲线点：
   
$$
kG = (x, y)
$$

2. 计算$r_2$：

$$
r_2 = (\text{Hash}(Z_A||M_2) + x) \bmod n
$$

3. 计算$s_2$：

$$
s_s = \left[(1 + d_A)^{-1} \cdot (k - r_2 \cdot d_A)\right] \bmod n
$$

#### 私钥恢复流程（通过两个签名恢复 $d_A$ ）

1. 建立方程组：

$$
\begin{cases}
s_1(1 + d_A) \equiv (k - r_1 \cdot d_A) \pmod{n} \\
s_2(1 + d_A) \equiv (k - r_2 \cdot d_A) \pmod{n}
\end{cases}
$$

2. 消去$k$得到：

$$
s_1 - s_2 \equiv (r_2 - r_1) \cdot d_A \pmod{n}
$$

3. 解出私钥$d_A$：

$$
d_A = \frac{s_2 - s_1}{s_1 - s_2 + r_1 - r_2} \bmod n
$$

#### 关键推导过程

从两个签名方程出发：

$$
\begin{aligned}
s_1(1 + d_A) &\equiv k - r_1 d_A \pmod{n} \\
s_2(1 + d_A) &\equiv k - r_2 d_A \pmod{n}
\end{aligned}
$$

将两式相减消去$k$：

$$
(s_1 - s_2)(1 + d_A) \equiv (r_2 - r_1)d_A \pmod{n}
$$

展开整理：

$$
s_1 - s_2 + (s_1 - s_2)d_A \equiv (r_2 - r_1)d_A \pmod{n}
$$

将含 $d_A$ 项移到左边：

$$
(s_1 - s_2 + r_1 - r_2)d_A \equiv s_2 - s_1 \pmod{n}
$$

最终解得：

$$
d_A \equiv (s_2 - s_1)(s_1 - s_2 + r_1 - r_2)^{-1} \pmod{n}
$$

---

### 3. 两个用户用了相同的 k 进行签名，可互相推导对方的私钥

#### Alice签名消息 $M_1$ 
1. 随机选取整数 $k$，满足： $1 \leq k \leq n - 1$ 
2. 计算椭圆曲线点：

$$
kG = (x, y)
$$

3. 计算$r_1$：

$$
r_1 = (\text{Hash}(Z_A||M_1) + x) \bmod n
$$

4. 计算$s_1$：

$$
s_1 = \left[(1 + d_A)^{-1} \cdot (k - r_1 \cdot d_A)\right] \bmod n
$$

5. 签名结果： $\sigma_A = (r_1, s_1)$

#### Bob签名消息 $M_2$ （重用Alice的k值）
1. 使用相同的$k$值：

$$
kG = (x, y)
$$

2. 计算$r_2$：

$$
r_2 = (\text{Hash}(Z_B||M_2) + x) \bmod n
$$

3. 计算$s_2$：

$$
s_2 = \left[(1 + d_B)^{-1} \cdot (k - r_2 \cdot d_B)\right] \bmod n
$$

4. 签名结果： $\sigma_B = (r_2, s_2)$

#### Alice推导Bob的私钥 $d_B$ 
1. 从Bob的签名方程：

$$
s_2 \equiv (1 + d_B)^{-1}(k - r_2 d_B) \pmod{n}
$$

2. 变形得到：

$$
d_B \equiv \frac{k - s_2}{s_2 + r_2} \bmod n
$$

#### Bob推导Alice的私钥 $d_A$ 
1. 从Alice的签名方程：

$$
s_1 \equiv (1 + d_A)^{-1}(k - r_1 d_A) \pmod{n}
$$

2. 变形得到：

$$
d_A \equiv \frac{k - s_1}{s_1 + r_1} \bmod n
$$

---

### 4. ECDSA和SM2签名共用私钥 d 和随机数 k，可恢复 d

#### ECDSA签名流程
1. 随机选取整数 $k$，满足： $1 \leq k \leq n - 1$ 
2. 计算曲线点：

$$
R = kG = (x, y)
$$

3. 计算消息哈希：
   
$$
e_1 = \text{hash}(m)
$$

4. 计算签名分量：
   
$$
r_1 = x \bmod n 
$$

$$
s_1 = (e_1 + r_1d)k^{-1} \bmod n
$$

5. 签名结果： $(r_1, s_1)$

#### SM2签名流程（重用相同k值）
1. 使用相同的 $k$ 值： $(x, y) = kG$ 

2. 计算SM2哈希：

$$
e_2 = h(Z_A||m)
$$

3. 计算签名分量：

$$
r_2 = (e_2 + x) \bmod n
$$

$$
s_2 = (1 + d)^{-1} \cdot (k - r_2d) \bmod n
$$

4. 签名结果： $(r_2, s_2)$

### 私钥恢复攻击

#### 建立方程组
1. 从ECDSA签名：
   
$$
d \cdot r_1 \equiv ks_1 - e_1 \pmod{n}
$$

2. 从SM2签名：
   
$$
d \cdot (s_2 + r_2) \equiv k - s_2 \pmod{n}
$$

#### 私钥推导过程
1. 将ECDSA方程表示为：

$$
k \equiv (dr_1 + e_1)s_1^{-1} \pmod{n}
$$

2. 代入SM2方程：
   
$$
d(s_2 + r_2) \equiv (dr_1 + e_1)s_1^{-1} - s_2 \pmod{n}
$$

3. 整理方程：
   
$$
d(s_2 + r_2 - r_1s_1^{-1}) \equiv e_1s_1^{-1} - s_2 \pmod{n}
$$

4. 最终解得私钥：

$$
d = \frac{s_1s_2 - e_1}{r_1 - s_1s_2 - s_1r_2} \bmod n
$$