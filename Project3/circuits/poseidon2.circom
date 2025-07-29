pragma circom 2.0.0;

include "circomlib/circuits/poseidon.circom";  // 确保路径正确

template Poseidon2Hash() {
    signal input x[2];       // 输入信号
    signal output hash;      // 输出信号（计算结果）

    component poseidon = Poseidon(2);  // 实例化 Poseidon(2)
    poseidon.inputs[0] <== x[0];      // 连接输入
    poseidon.inputs[1] <== x[1];

    hash <== poseidon.out;   // 约束输出（注意：Poseidon 的输出可能是 `.out` 而非 `.output`）
}

component main {public [x]} = Poseidon2Hash();  // 声明公开输入