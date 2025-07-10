#!/bin/bash
node build/poseidon2_js/generate_witness.js build/poseidon2_js/poseidon2.wasm inputs/input.json build/witness.wtns
snarkjs groth16 prove build/poseidon2_final.zkey build/witness.wtns build/proof.json build/public.json
