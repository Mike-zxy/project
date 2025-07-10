#!/bin/bash
mkdir -p verifier
snarkjs zkey export solidityverifier build/poseidon2_final.zkey verifier/verifier.sol
