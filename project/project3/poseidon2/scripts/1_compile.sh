#!/bin/bash
mkdir -p build
circom circuit/poseidon2.circom --r1cs --wasm --sym -o build/
