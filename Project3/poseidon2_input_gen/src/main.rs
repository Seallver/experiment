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

macro_rules! INPUT1 {
    () => {
        "123456789"
    };
}

macro_rules! INPUT2 {
    () => {
        "987654321"
    };
}

fn main() {
    let inputs = vec![Fr::from_str(INPUT1!()).unwrap(), Fr::from_str(INPUT2!()).unwrap()];
    let poseidon = Poseidon::new();
    let hash = poseidon.hash(inputs.clone()).unwrap();

    let input_strings = vec![
        INPUT1!().to_string(),
        INPUT2!().to_string(),
    ];

    let hash_str = hash.to_string();
    let clean_hash = hash_str.trim_start_matches("Fr(").trim_end_matches(')');

    let input_struct = Input {
        x: input_strings,
        hash: clean_hash.to_string(),
    };

    let json_str = serde_json::to_string_pretty(&input_struct).unwrap();
    fs::create_dir_all("../inputs").unwrap();
    let mut file = File::create("../inputs/input.json").unwrap();
    file.write_all(json_str.as_bytes()).unwrap();

    println!("输入与哈希结果已写入 inputs/input.json");
}
