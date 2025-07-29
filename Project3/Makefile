PTAU = powersOfTau28_hez_final_14.ptau
CIRCUIT_FILE = circuits/poseidon2.circom
OUT_DIR = build
CIRCUIT_BASE = poseidon2

.PHONY: all clean compile witness setup prove verify

all: compile witness setup prove verify

compile:
	mkdir -p $(OUT_DIR)
	circom $(CIRCUIT_FILE) --r1cs --wasm --sym -o $(OUT_DIR) -l node_modules

witness:
	node $(OUT_DIR)/$(CIRCUIT_BASE)_js/generate_witness.js $(OUT_DIR)/$(CIRCUIT_BASE)_js/$(CIRCUIT_BASE).wasm inputs/input.json $(OUT_DIR)/witness.wtns

setup:
	snarkjs groth16 setup $(OUT_DIR)/$(CIRCUIT_BASE).r1cs $(PTAU) $(OUT_DIR)/$(CIRCUIT_BASE).zkey
	snarkjs zkey export verificationkey $(OUT_DIR)/$(CIRCUIT_BASE).zkey $(OUT_DIR)/verification_key.json

prove:
	snarkjs groth16 prove $(OUT_DIR)/$(CIRCUIT_BASE).zkey $(OUT_DIR)/witness.wtns $(OUT_DIR)/proof.json $(OUT_DIR)/public.json

verify:
	snarkjs groth16 verify $(OUT_DIR)/verification_key.json $(OUT_DIR)/public.json $(OUT_DIR)/proof.json

clean:
	rm -rf $(OUT_DIR)
