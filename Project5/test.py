from sm2 import OptimizedSM2, SM2Signature
import secrets
from math import gcd

from ecdsa import SigningKey, NIST256p
from hashlib import sha256


def test_sm2_encryption():
    '''测试 SM2 加密和解密功能'''

    print("测试 SM2 加解密正确性：")
    sm2 = OptimizedSM2()
    
    private_key, public_key = sm2.generate_keypair()
    print(f"私钥：0x{private_key:x}")
    print(f"公钥：{public_key}")

    message = "Hello World!"
    print(f"明文：{message}")
    
    ciphertext = sm2.encrypt(message, public_key)
    print(f"密文：{ciphertext}")

    plaintext = sm2.decrypt(ciphertext, private_key)
    print(f"解密结果：{plaintext}")

    print(f"解密：{'成功' if plaintext == message else '失败'}\n")
    
def test_sm2_signature():
    '''测试 SM2 签名和验证功能'''

    print("测试 SM2 签名和验证：")
    sm2 = SM2Signature()

    # 生成密钥对
    d, P = sm2.generate_keypair()
    print(f"私钥: 0x{d:x}")
    print(f"公钥: {P}")

    # 签名消息
    msg = "This is a test message."
    signature = sm2.sign(msg, d)
    r, s = signature
    print(f"签名 r: 0x{r:x}, s: 0x{s:x}")

    # 验证签名
    is_valid = sm2.verify(msg, signature, P)
    print(f"签名验证: {'成功' if is_valid else '失败'}\n")

def recover_private_key_from_leak_k():
    '''POC:从已知的 k 恢复私钥'''

    print("===> POC: 泄露 k 后恢复私钥：")
    sm2 = SM2Signature()

    # 生成密钥对
    d, P = sm2.generate_keypair()
    print(f"真实私钥: 0x{d:x}")

    # 泄露 k
    leak_k = secrets.randbelow(sm2.n - 1) + 1
    msg = "secret operation"

    # 签名
    signature = sm2.sign(msg, d, k=leak_k)
    r, s = signature
    print(f"签名 r: 0x{r:x}, s: 0x{s:x}")
    print(f"泄露的 k: 0x{leak_k:x}")

    r, s = signature
    n = sm2.n
    denominator = (s + r) % n
    if gcd(denominator, n) != 1:
        raise ValueError("分母不可逆，攻击失败")
    recovered_d = (sm2._fast_mod_inverse(denominator, n) * (leak_k - s)) % n

    print(f"恢复私钥: 0x{recovered_d:x}")
    print("攻击成功\n" if recovered_d == d else "攻击失败\n")

def recover_private_key_from_fixed_k():
    '''POC:从固定的 k 恢复私钥'''

    print("===> POC: 两次签名使用固定的 k 恢复私钥：")
    sm2 = SM2Signature()

    # 生成密钥对
    d, P = sm2.generate_keypair()
    print(f"真实私钥: 0x{d:x}")

    # 固定 k
    known_k = secrets.randbelow(sm2.n - 1) + 1
    msg1 = "message one"
    msg2 = "message two"

    # 使用相同 k 对两条消息分别签名
    signature1 = sm2.sign(msg1, d, k=known_k)
    signature2 = sm2.sign(msg2, d, k=known_k)
    r1, s1 = signature1
    r2, s2 = signature2

    print(f"签名1 r: 0x{r1:x}, s: 0x{s1:x}")
    print(f"签名2 r: 0x{r2:x}, s: 0x{s2:x}")
    print(f"固定的 k: 0x{known_k:x}")

    n = sm2.n
    numerator = (s2 - s1) % n
    denominator = (s1 - s2 + r1 - r2) % n

    if gcd(denominator, n) != 1:
        raise ValueError("分母不可逆，攻击失败")

    recovered_d = (numerator * sm2._fast_mod_inverse(denominator, n)) % n
    print(f"恢复私钥: 0x{recovered_d:x}")
    print("攻击成功\n" if recovered_d == d else "攻击失败\n")

def recover_private_key_from_reused_k():
    '''POC: 当两个用户复用了相同的 k 时恢复私钥'''

    print("===> POC: 两个用户复用相同的 k 恢复私钥：")
    sm2 = SM2Signature()

    # 用户1和用户2分别生成密钥对
    d1, P1 = sm2.generate_keypair()
    d2, P2 = sm2.generate_keypair()
    print(f"用户1私钥: 0x{d1:x}")
    print(f"用户2私钥: 0x{d2:x}")

    # 固定一个 k（被复用）
    reused_k = secrets.randbelow(sm2.n - 1) + 1
    msg1 = "user1 message"
    msg2 = "user2 message"

    # 用户1签名
    r1, s1 = sm2.sign(msg1, d1, k=reused_k)
    # 用户2签名
    r2, s2 = sm2.sign(msg2, d2, k=reused_k)

    print(f"签名1 r: 0x{r1:x}, s: 0x{s1:x}")
    print(f"签名2 r: 0x{r2:x}, s: 0x{s2:x}")
    print(f"复用的 k: 0x{reused_k:x}")

    # 恢复用户1的私钥（假设我们已知 d2）
    n = sm2.n
    # 解方程：k = s1(1 + d1) + r1 * d1 = s2(1 + d2) + r2 * d2
    # => s1 + s1*d1 + r1*d1 = known_k - r1*d1 = s2(1 + d2) + r2*d2
    known_k = (s2 * (1 + d2) + r2 * d2) % n
    recovered_d1 = ((known_k - s1) * sm2._fast_mod_inverse(s1 + r1, n)) % n

    # 同理恢复用户2的私钥（假设我们已知 d1）
    known_k = (s1 * (1 + d1) + r1 * d1) % n
    recovered_d2 = ((known_k - s2) * sm2._fast_mod_inverse(s2 + r2, n)) % n

    print(f"恢复用户1私钥: 0x{recovered_d1:x} -> 攻击{'成功' if recovered_d1 == d1 else '失败'}")
    print(f"恢复用户2私钥: 0x{recovered_d2:x} -> 攻击{'成功' if recovered_d2 == d2 else '失败'}\n")

def modinv(a, n):
    '''计算 a 在 mod n 下的乘法逆元'''
    if gcd(a, n) != 1:
        raise ValueError("分母不可逆")
    return pow(a, -1, n)

def test_cross_protocol_attack():
    '''POC: ECDSA 与 SM2 复用同一个 k 和 d，恢复私钥'''
    print("===> POC: ECDSA 与 SM2 复用同一个 k 和 d，恢复私钥")

    # 1. 选择统一椭圆曲线 secp256r1 = NIST256p
    curve = NIST256p
    G = curve.generator
    n = curve.order

    # 2. 生成私钥 d，复用
    d = SigningKey.generate(curve=NIST256p)
    d_int = d.privkey.secret_multiplier
    print(f"私钥 d: 0x{d_int:x}")

    # 3. 选择固定 k
    reused_k = secrets.randbelow(n - 1) + 1
    R = reused_k * G
    x = R.x()
    r1 = x % n  # ECDSA 中 r = x mod n

    # 4. 构造 ECDSA 签名
    msg1 = "ecdsa message"
    e1 = int.from_bytes(sha256(msg1.encode()).digest(), 'big') # ECDSA 用 SHA-256 作为哈希
    s1 = (modinv(reused_k, n) * (e1 + r1 * d_int)) % n
    print(f"ECDSA 签名 (r1, s1): (0x{r1:x}, 0x{s1:x})")

    # 5. 构造 SM2 签名
    sm2 = SM2Signature()
    sm2.n = n  # 强制设定为相同曲线阶
    msg2 = "sm2 message"
    r2, s2 = sm2.sign(msg2, d_int, k=reused_k)
    print(f"SM2 签名 (r2, s2): (0x{r2:x}, 0x{s2:x})")

    # 6. 攻击恢复私钥 d
    numerator = (s2 * s1 - e1) % n
    denominator = (r1 - s1 * s2 - s1 * r2) % n
    if gcd(denominator, n) != 1:
        raise ValueError("攻击失败：分母不可逆")
    recovered_d = (numerator * modinv(denominator, n)) % n

    print(f"恢复私钥 d: 0x{recovered_d:x}")
    print("攻击成功\n" if recovered_d == d_int else "攻击失败\n")


if __name__ == "__main__":
    #正确性测试
    test_sm2_encryption()
    test_sm2_signature()
    # POC 测试
    recover_private_key_from_leak_k()
    recover_private_key_from_fixed_k()
    recover_private_key_from_reused_k()
    test_cross_protocol_attack()
    



