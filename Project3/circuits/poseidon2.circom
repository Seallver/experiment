pragma circom 2.0.0;

include "circomlib/poseidon.circom";

// 使用 Poseidon2 哈希，输入长度为 2
template Poseidon2Verifier() {
    signal input preimage[2];    // 私有输入，哈希原文
    signal input expectedHash;   // 公开输入，哈希结果

    component hashFunc = Poseidon(2);
    for (var i = 0; i < 2; i++) {
        hashFunc.inputs[i] <== preimage[i];
    }

    // 断言计算结果等于预期哈希
    hashFunc.output === expectedHash;
}

component main = Poseidon2Verifier();
