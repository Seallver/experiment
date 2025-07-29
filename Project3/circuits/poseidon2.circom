pragma circom 2.0.0;

include "circomlib/circuits/poseidon.circom";  

template Poseidon2Hash() {
    signal input x[2];       // 输入信号
    signal input hash;      

    component poseidon = Poseidon(2);  // 实例化 Poseidon(2)
    poseidon.inputs[0] <== x[0];      // 连接输入
    poseidon.inputs[1] <== x[1];
    
    poseidon.out === hash;
}

component main = Poseidon2Hash();