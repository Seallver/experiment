# 项目介绍

本项目实现了一个基于同态加密与椭圆曲线的隐私集合求交求和协议（PISP），用于在不泄露非交集元素的前提下，计算两个集合交集对应值的和。

## 应用场景

两个参与方：

- **P1** 拥有集合 V = {v₁, v₂, ..., vₙ}
- **P2** 拥有集合 W = {(w₁, t₁), (w₂, t₂), ..., (wₘ, tₘ)}，其中每个元素都绑定了一个值 `tⱼ`

目标是：
计算集合交集 $V\cup W$中元素在 P2 中的值的总和，但不泄露各自的集合内容及非交集数据。

## 协议流程（3轮）

令：

- 参与方 $P_1$ 拥有集合 $V = \{v_1, v_2, \ldots, v_n\}$
- 参与方 $P_2$ 拥有集合及对应权重（payload）

$$
W = \{(w_1, t_1), (w_2, t_2), \ldots, (w_m, t_m)\}
$$

- 选用椭圆曲线群 $\mathbb{G}$ 及其生成元 $G$，阶为 $q$
- 哈希函数 $H: \{0,1\}^* \to \mathbb{G}$，满足随机预言机模型
- $P_1,P_2$ 各自随机选取私钥 $k_1, k_2 \in \mathbb{Z}_q^*$
- $\mathsf{Enc}(\cdot), \mathsf{Dec}(\cdot)$ 表示 Paillier 公钥加密和私钥解密函数，支持加法同态

---

### Round 1: $P_1 \to P_2$

$P_1$计算：

$$
S_1 = \{ k_1 \cdot H(v_i) \mid v_i \in V \} \subseteq \mathbb{G}
$$

并对集合元素顺序做随机置换（Shuffle）后发送给 $P_2$。

---

### Round 2: $P_2 \to P_1$

$P_2$ 接收 $S_1$ 后，计算：

$$
Z = \{ k_2 \cdot P \mid P \in S_1 \} = \{ k_2 k_1 \cdot H(v_i) \mid v_i \in V \} \subseteq \mathbb{G}
$$

同时，对自身集合中每个元素计算：

$$
E = \{ (k_2 \cdot H(w_j), \mathsf{Enc}(t_j)) \mid (w_j, t_j) \in W \}
$$

对 $Z$ 和 $E$ 进行随机置换后发送给 $P_1$。

---

### Round 3: $P_1$ 本地计算

$P_1$ 接收 $Z,E$ 后，对每个 $(k_2 \cdot H(w_j), \mathsf{Enc}(t_j)) \in E$ 计算：

$$
Q_j = k_1 \cdot (k_2 \cdot H(w_j)) = k_1 k_2 \cdot H(w_j)
$$

然后判断：

$$
Q_j \in Z \quad \Longleftrightarrow \quad w_j \in V
$$

若成立，则将对应密文累加：

$$
C = \sum_{j: Q_j \in Z} \mathsf{Enc}(t_j)
$$

最终，$P_1$使用私钥解密：

$$
\sum_{j: w_j \in V} t_j = \mathsf{Dec}(C)
$$

得到交集元素对应权重的和。

---

### 协议核心安全性说明

- $k_1, k_2$ 私钥随机且保密，保证点乘结果对外不可逆推原元素；
- 哈希函数 $H$ 满足随机预言机，映射元素到椭圆曲线群保证碰撞抵抗；
- Paillier 加密确保权重值 $t_j$ 的隐私，非交集元素不被解密；
- 元素顺序随机打乱避免通过顺序推断信息。


# 运行测试

项目提供了测试单元，即 `test_PISP.py` 文件

## 数据示例

- P1 的集合：`["alice", "bob", "charlie"]`
- P2 的集合及值：`[("alice", 100), ("bob", 400), ("dave", 300)]`

交集为 `{"alice", "bob"}`，对应值为 100 + 400 = **500**

## 安装依赖
```bash
pip install petlib phe
```

## 运行测试
```bash
python3 test_PISP.py
```

## 输出结果
```bash
Intersection sum: 500
```