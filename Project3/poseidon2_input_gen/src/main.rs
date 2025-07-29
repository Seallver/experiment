use poseidon_rs::{Fr, Poseidon};
use std::fs::File;
use std::io::Write;
use serde::Serialize;
use std::fs;

#[macro_use]
use ff::*;

#[derive(Serialize)]
struct Input {
    x: Vec<String>, // 注意这里改为 Vec<String>
    hash: String,
}

use num_bigint::BigUint;
use num_traits::Zero;

fn main() {
    let inputs = vec![Fr::from_str("123456789").unwrap(), Fr::from_str("987654321").unwrap()];
    let poseidon = Poseidon::new();
    let hash = poseidon.hash(inputs.clone()).unwrap();

    let input_strings = vec![
        "123456789".to_string(),
        "987654321".to_string(),
    ];

    let input_struct = Input {
        x: input_strings,
        hash: hash.to_string(),
    };

    let json_str = serde_json::to_string_pretty(&input_struct).unwrap();
    fs::create_dir_all("../inputs").unwrap();
    let mut file = File::create("../inputs/input.json").unwrap();
    file.write_all(json_str.as_bytes()).unwrap();

    println!("输入与哈希结果已写入 inputs/input.json");
}
