from DDH_PISP import AHE, Party1, Party2

def test_pisp():
    V = ["alice", "bob", "charlie"]
    W = [("alice", 100), ("bob", 400), ("dave", 300)]

    ahe = AHE()
    P1 = Party1(V, ahe)
    P2 = Party2(W, ahe)

    Hv_k1 = P1.round1()
    Z, encrypted_pairs = P2.round2(Hv_k1)
    sum_J = P1.round3(Z, encrypted_pairs)

    assert sum_J == 500, f"测试失败，交集求和结果应为500，实际为{sum_J}"
    print(f"测试通过，交集求和结果为: {sum_J}")

if __name__ == "__main__":
    test_pisp()

