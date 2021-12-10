# Pipeline-Simulator

## Objective
The objective for this assignment is to make sure
1. You are familiar with the pipelining process on a MIPS processor.
2. You gain some experience with the principles of pipelining and hazard mitigation techniques including stalling, forwarding and branch prediction.
3. You are comfortable working with C, including I/O, parsing and memory management.

## Realized functions:
1. Reading and parsing
2. Setting up the required pipeline registers
3. Populating the data memory and maintaining the register file 
4. Handling NOOP and HALT  
5. Handling a program without hazards
6. Handling Data Hazards through forwarding
7. Stalling when forwarding is not possible 

For the control hazard:
1. Handling branches by assuming branch is not taken and correcting later
2. Handling branches using prediction
I just write down the code and  test with some "bne" instruction. But I cannot get the correct result but I don't know whether they are totally correct or not.
