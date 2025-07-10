#!/bin/bash
snarkjs powersoftau new bn128 12 build/pot12_0000.ptau -v
snarkjs powersoftau contribute build/pot12_0000.ptau build/pot12_0001.ptau --name="1st Contributor" -v
snarkjs powersoftau prepare phase2 build/pot12_0001.ptau build/pot12_final.ptau -v
snarkjs groth16 setup build/poseidon2.r1cs build/pot12_final.ptau build/poseidon2_0000.zkey
snarkjs zkey contribute build/poseidon2_0000.zkey build/poseidon2_final.zkey --name="Final Contributor"
snarkjs zkey export verificationkey build/poseidon2_final.zkey build/verification_key.json
