from petlib.ec import EcGroup
from phe import paillier
import random

# 初始化椭圆曲线组（secp256r1）
group = EcGroup(nid=713)
order = group.order()

# 哈希函数 H: str -> EC点
H = lambda x: group.hash_to_point(x.encode())

### Paillier 同态加密封装 ###
class AHE:
    def __init__(self):
        self.pubkey, self.privkey = paillier.generate_paillier_keypair()

    def encrypt(self, m):
        return self.pubkey.encrypt(m)

    def decrypt(self, c):
        return self.privkey.decrypt(c)

### Party1 ###
class Party1:
    def __init__(self, V, ahe):
        self.V = V
        self.k1 = order.random()
        self.ahe = ahe

    def round1(self):
        # H(v)^k1
        hashed = [self.k1 * H(v) for v in self.V]
        random.shuffle(hashed)
        return hashed

    def round3(self, Z, encrypted_pairs):
        intersection_sum_cipher = None
        Z_bytes = set([z.export() for z in Z])
        
        for (Hw_k2, enc_t) in encrypted_pairs:
            Hw_k1k2 = self.k1 * Hw_k2
            if Hw_k1k2.export() in Z_bytes:
                if intersection_sum_cipher is None:
                    intersection_sum_cipher = enc_t
                else:
                    intersection_sum_cipher += enc_t  # Paillier 同态加法
        if intersection_sum_cipher is None:
            return 0
        return self.ahe.decrypt(intersection_sum_cipher)

### Party2 ###
class Party2:
    def __init__(self, W, ahe):
        self.W = W
        self.k2 = order.random()
        self.ahe = ahe

    def round2(self, received_Hv_k1):
        # Z = {H(v)^(k1k2)}
        Z = [self.k2 * hv_k1 for hv_k1 in received_Hv_k1]
        random.shuffle(Z)

        # 加密t_j & blind H(w_j)^k2
        encrypted_pairs = [
            (self.k2 * H(w), self.ahe.encrypt(t)) for (w, t) in self.W
        ]
        random.shuffle(encrypted_pairs)
        return Z, encrypted_pairs

### 测试主函数 ###
if __name__ == "__main__":
    # 数据准备
    V = ["alice", "bob", "charlie"]
    W = [("alice", 100), ("bob", 400), ("dave", 300)]

    ahe = AHE()
    P1 = Party1(V, ahe)
    P2 = Party2(W, ahe)

    # Round 1
    Hv_k1 = P1.round1()

    # Round 2
    Z, encrypted_pairs = P2.round2(Hv_k1)

    # Round 3
    sum_J = P1.round3(Z, encrypted_pairs)

    # 输出结果
    assert sum_J == 500  # 只包含 "alice" 和 "bob" 的 t 值之和
    print(f"Intersection sum: {sum_J}") 