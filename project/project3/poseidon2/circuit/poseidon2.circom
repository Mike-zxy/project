pragma circom 2.1.9;

include "circomlib/poseidon.circom";

template Poseidon2() {
    signal input in[2];
    signal input out;

    component h = Poseidon(2);
    h.inputs[0] <== in[0];
    h.inputs[1] <== in[1];

    h.out === out; // enforce equality with public output
}

component main = Poseidon2();
