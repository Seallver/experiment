# 项目介绍
本项目使用 Circom 实现了 t=2 的 Poseidon2 哈希函数的零知识电路，用 Groth16 生成证明。

以下是项目结构说明

```bash 
.
├── Makefile                        # 一键构建工具，包含编译、生成证明等命令
├── README.md                       # 本文件：项目说明文档
├── build/                          # 构建生成的文件目录
│   └── ...              
├── circuits/
│   └── poseidon2.circom            # Circom 编写的 Poseidon2 哈希电路定义
├── inputs/
│   └── input.json                  # 输入文件（原像数据与哈希值），由 Rust 程序生成
├── poseidon2_input_gen/           # Rust 项目目录，用于生成输入 JSON
│   ├── Cargo.toml                  # Rust 项目配置
│   └── src/
│       └── main.rs                 # Rust 主程序：构造输入并计算 Poseidon2 哈希值
└── powersOfTau28_hez_final_14.ptau# 通用信任设置文件（Powers of Tau Phase 1）
```