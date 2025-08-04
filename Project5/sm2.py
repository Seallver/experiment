import secrets
import struct
from math import gcd, ceil, log
from typing import Tuple, Optional, List
from gmssl import sm3

# SM2 标准参数
SM2_P = 0x8542D69E4C044F18E8B92435BF6FF7DE457283915C45517D722EDB8B08F1DFC3
SM2_A = 0x787968B4FA32C3FD2417842E73BBFEFF2F3C848B6831D7E0EC65228B3937E498
SM2_B = 0x63E4C6D3B23B0C849CF84241484BFE48F61D59A5B16BA06E6E12D1DA27C5249A
SM2_N = 0x8542D69E4C044F18E8B92435BF6FF7DD297720630485628D5AE74EE7C32E79B7
SM2_GX = 0x421DEBD61B62EAB6746434EBC3CC315E32220B3BADD50BDC4C4E6C147FEDD43D
SM2_GY = 0x0680512BCBB42C07D47349D2153B70C4E5D7FDFCBFA36EA1A85841B9E46E09A2

_FIELD_BYTES = ceil(ceil(log(SM2_P, 2)) / 8)
_POINT_BYTES = 2 * _FIELD_BYTES + 1


class OptimizedSM2:
    """SM2算法优化实现"""
    def __init__(self):
        self.p = SM2_P
        self.a = SM2_A
        self.b = SM2_B
        self.n = SM2_N
        self.G = (SM2_GX, SM2_GY)
        self._precompute_values()

    def _precompute_values(self):
        self._inv_cache = {}
        self._pow2_cache = [1]
        for _ in range(1, 256):
            self._pow2_cache.append((self._pow2_cache[-1] * 2) % self.p)

    def _fast_mod_inverse(self, a: int, m: int) -> int:
        if a in self._inv_cache:
            return self._inv_cache[a]

        def extended_gcd(a, b):
            if a == 0:
                return b, 0, 1
            g, x, y = extended_gcd(b % a, a)
            return g, y - (b // a) * x, x

        g, x, _ = extended_gcd(a, m)
        if g != 1:
            raise ValueError("模逆不存在")

        result = x % m
        self._inv_cache[a] = result
        return result

    def _fast_mod_mul(self, a, b): return (a * b) % self.p
    def _fast_mod_add(self, a, b): return (a + b) % self.p
    def _fast_mod_sub(self, a, b): return (a - b) % self.p

    def _optimized_int_to_bytes(self, x, length=None):
        if length is None:
            length = max(1, (x.bit_length() + 7) // 8)
        return x.to_bytes(length, 'big')

    def _optimized_bytes_to_int(self, data): return int.from_bytes(data, 'big')
    def _optimized_hex_to_bytes(self, hex_str): return bytes.fromhex(hex_str)
    def _optimized_bytes_to_hex(self, data): return data.hex()

    def _naf_decomposition(self, k: int) -> List[int]:
        naf = []
        while k > 0:
            if k % 2 == 1:
                ki = 2 - (k % 4)
                k -= ki
            else:
                ki = 0
            naf.append(ki)
            k //= 2
        return naf

    def _point_add_optimized(self, P, Q):
        if P == (0, 0): return Q
        if Q == (0, 0): return P

        x1, y1 = P
        x2, y2 = Q

        if x1 == x2:
            if (y1 + y2) % self.p == 0:
                return (0, 0)
            else:
                return self._point_double_optimized(P)

        lmbd = self._fast_mod_mul(
            self._fast_mod_sub(y2, y1),
            self._fast_mod_inverse(self._fast_mod_sub(x2, x1), self.p)
        )
        x3 = self._fast_mod_sub(self._fast_mod_sub(self._fast_mod_mul(lmbd, lmbd), x1), x2)
        y3 = self._fast_mod_sub(self._fast_mod_mul(lmbd, self._fast_mod_sub(x1, x3)), y1)
        return (x3, y3)

    def _point_double_optimized(self, P):
        if P == (0, 0): return P
        x1, y1 = P
        if y1 == 0: return (0, 0)

        lmbd = self._fast_mod_mul(
            self._fast_mod_add(self._fast_mod_mul(3, self._fast_mod_mul(x1, x1)), self.a),
            self._fast_mod_inverse(self._fast_mod_mul(2, y1), self.p)
        )
        x3 = self._fast_mod_sub(self._fast_mod_sub(self._fast_mod_mul(lmbd, lmbd), x1), x1)
        y3 = self._fast_mod_sub(self._fast_mod_mul(lmbd, self._fast_mod_sub(x1, x3)), y1)
        return (x3, y3)

    def _scalar_multiply_naf(self, P, k):
        if k == 0: return (0, 0)
        naf = self._naf_decomposition(k)
        P_neg = (P[0], (-P[1]) % self.p)
        result = (0, 0)
        for i in range(len(naf) - 1, -1, -1):
            result = self._point_double_optimized(result)
            if naf[i] == 1:
                result = self._point_add_optimized(result, P)
            elif naf[i] == -1:
                result = self._point_add_optimized(result, P_neg)
        return result

    def _optimized_kdf(self, Z: bytes, klen: int) -> bytes:
        v = 256
        if klen >= (2**32 - 1) * v:
            raise ValueError("密钥长度超出限制")

        ct = 1
        k = b''
        while len(k) < klen:
            input_data = Z + struct.pack('>I', ct)
            hash_result = sm3.sm3_hash(list(input_data))
            k += bytes.fromhex(hash_result)
            ct += 1
        return k[:klen]

    def _sm3_hash_bytes(self, data: bytes) -> bytes:
        return bytes.fromhex(sm3.sm3_hash(list(data)))

    def generate_keypair(self) -> Tuple[int, Tuple[int, int]]:
        d = secrets.randbelow(self.n - 1) + 1
        P = self._scalar_multiply_naf(self.G, d)
        return d, P

    def encrypt(self, message: str, public_key: Tuple[int, int]) -> str:
        k = secrets.randbelow(self.n - 1) + 1
        C1 = self._scalar_multiply_naf(self.G, k)
        kP = self._scalar_multiply_naf(public_key, k)
        x2, y2 = kP
        message_bytes = message.encode()
        klen = len(message_bytes)

        t = self._optimized_kdf(
            self._optimized_int_to_bytes(x2, _FIELD_BYTES) +
            self._optimized_int_to_bytes(y2, _FIELD_BYTES),
            klen
        )
        if all(b == 0 for b in t):
            return self.encrypt(message, public_key)

        C2 = bytes(a ^ b for a, b in zip(message_bytes, t))
        C3 = self._sm3_hash_bytes(
            self._optimized_int_to_bytes(x2, _FIELD_BYTES) +
            message_bytes +
            self._optimized_int_to_bytes(y2, _FIELD_BYTES)
        )
        C1_bytes = self._optimized_int_to_bytes(0x04, 1) + \
                   self._optimized_int_to_bytes(C1[0], _FIELD_BYTES) + \
                   self._optimized_int_to_bytes(C1[1], _FIELD_BYTES)

        return self._optimized_bytes_to_hex(C1_bytes + C2 + C3)

    def decrypt(self, ciphertext: str, private_key: int) -> str:
        cipher_bytes = self._optimized_hex_to_bytes(ciphertext)
        if len(cipher_bytes) < _POINT_BYTES + 32:
            raise ValueError("密文长度不足")

        C1_bytes = cipher_bytes[:_POINT_BYTES]
        if C1_bytes[0] != 0x04:
            raise ValueError("无效的点格式")

        x1 = self._optimized_bytes_to_int(C1_bytes[1:_FIELD_BYTES+1])
        y1 = self._optimized_bytes_to_int(C1_bytes[_FIELD_BYTES+1:_POINT_BYTES])
        C1 = (x1, y1)

        if not self._is_on_curve(C1):
            raise ValueError("C1不在椭圆曲线上")

        dC1 = self._scalar_multiply_naf(C1, private_key)
        x2, y2 = dC1

        C2_len = len(cipher_bytes) - _POINT_BYTES - 32
        t = self._optimized_kdf(
            self._optimized_int_to_bytes(x2, _FIELD_BYTES) +
            self._optimized_int_to_bytes(y2, _FIELD_BYTES),
            C2_len
        )
        if all(b == 0 for b in t):
            raise ValueError("解密失败：t全为零")

        C2 = cipher_bytes[_POINT_BYTES:_POINT_BYTES+C2_len]
        C3 = cipher_bytes[_POINT_BYTES+C2_len:]
        message_bytes = bytes(a ^ b for a, b in zip(C2, t))

        expected_C3 = self._sm3_hash_bytes(
            self._optimized_int_to_bytes(x2, _FIELD_BYTES) +
            message_bytes +
            self._optimized_int_to_bytes(y2, _FIELD_BYTES)
        )
        if C3 != expected_C3:
            raise ValueError("解密失败：哈希验证失败")

        return message_bytes.decode('utf-8')

    def _is_on_curve(self, P: Tuple[int, int]) -> bool:
        if P == (0, 0):
            return True
        x, y = P
        left = self._fast_mod_mul(y, y)
        right = self._fast_mod_add(
            self._fast_mod_add(
                self._fast_mod_mul(self._fast_mod_mul(x, x), x),
                self._fast_mod_mul(self.a, x)
            ),
            self.b
        )
        return left == right

class SM2Signature(OptimizedSM2):
    def sign(self, msg: str, private_key: int, k: int = None) -> tuple[int, int]:
        """SM2签名算法"""
        e = int.from_bytes(self._sm3_hash_bytes(msg.encode()), 'big')
        d = private_key
        
        if k is None:
            k = secrets.randbelow(self.n - 1) + 1
        else:  # 用于测试固定k值的情况
            if k <= 0 or k >= self.n:
                raise ValueError("无效的k值")
        
        P = self._scalar_multiply_naf(self.G, k)
        r = (e + P[0]) % self.n
        if r == 0 or r + k == self.n:
            return self.sign(msg, private_key, k)
        
        s = (self._fast_mod_inverse(1 + d, self.n) * (k - r * d)) % self.n
        if s == 0:
            return self.sign(msg, private_key, k)
        
        return (r, s)
    
    def verify(self, msg: str, signature: tuple[int, int], public_key: tuple[int, int]) -> bool:
        """SM2签名验证"""
        r, s = signature
        if not (1 <= r < self.n and 1 <= s < self.n):
            return False
        
        e = int.from_bytes(self._sm3_hash_bytes(msg.encode()), 'big')
        t = (r + s) % self.n
        if t == 0:
            return False
        
        P1 = self._scalar_multiply_naf(self.G, s)
        P2 = self._scalar_multiply_naf(public_key, t)
        P = self._point_add_optimized(P1, P2)
        
        return (e + P[0]) % self.n == r