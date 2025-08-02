from sm2 import OptimizedSM2, SM2Signature
import secrets
from math import gcd

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

    print(f"解密:{'成功' if plaintext == message else '失败'}\n")
    
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

def recover_private_key_from_known_k():
    '''POC:从已知的 k 恢复私钥'''

    print("泄露 k 后恢复私钥：")
    sm2 = SM2Signature()

    # 生成密钥对
    d, P = sm2.generate_keypair()
    print(f"真实私钥: 0x{d:x}")

    # 固定 k
    known_k = secrets.randbelow(sm2.n - 1) + 1
    msg = "secret operation"

    # 签名
    signature = sm2.sign(msg, d, k=known_k)
    r, s = signature
    print(f"签名 r: 0x{r:x}, s: 0x{s:x}")
    print(f"泄露的 k: 0x{known_k:x}")

    r, s = signature
    n = sm2.n
    denominator = (s + r) % n
    if gcd(denominator, n) != 1:
        raise ValueError("分母不可逆，攻击失败")
    recovered_d = (sm2._fast_mod_inverse(denominator, n) * (known_k - s)) % n

    print(f"恢复私钥: 0x{recovered_d:x}")
    print("攻击成功\n" if recovered_d == d else "攻击失败\n")

def recover_private_key_from_fixed_k():
    '''POC:从固定的 k 恢复私钥'''

    print("两次签名使用固定的 k 恢复私钥：")
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





if __name__ == "__main__":
    #正确性测试
    test_sm2_encryption()
    test_sm2_signature()
    # POC 测试
    recover_private_key_from_known_k()
    recover_private_key_from_fixed_k()



