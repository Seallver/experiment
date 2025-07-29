use poseidon_rs::poseidon2::{Poseidon2, Fr};
use std::fs::File;
use std::io::Write;
use serde::Serialize;

#[derive(Serialize)]
struct PoseidonInput {
    values: Vec<Fr>,
    hash_result: String,
}

fn main() -> std::io::Result<()> {
    let inputs = vec![Fr::from(123_456_789u64), Fr::from(987_654_321u64)];
    let poseidon = Poseidon2::new(inputs.len());
    let hash = poseidon.hash(inputs.clone());

    let data = PoseidonInput {
        values: inputs,
        hash_result: format!("{}", hash),
    };

    let json = serde_json::to_string_pretty(&data).expect("JSON 序列化失败");
    std::fs::create_dir_all("inputs")?;
    let mut file = File::create("inputs/input.json")?;
    file.write_all(json.as_bytes())?;

    println!("输入和哈希结果已成功写入 inputs/input.json");
    Ok(())
}
