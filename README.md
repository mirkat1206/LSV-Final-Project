# LSV-Final-Project
ABC: System for Sequential Logic Synthesis and Formal Verification

Re-implement “Constraint Solving for Synthesis and Veriﬁcation of Threshold Logic Circuits”


## Introduction
  In our final project, we tried to re-implement the paper “Constraint Solving for Synthesis and Verification of Threshold Logic Circuits” written by Nian-Ze Lee and Jie-Hong R. Jiang from National Taiwan University, which was published on 2020 IEEE Transactions on Computer-Aided Design of Integrated Circuits and Systems.
	
  Since threshold logic (TL) circuits are gaining increasing attention due to their strong bind to neural network applications, the automatic synthesis and verification of TL circuits are important.
	
  The paper formulated the collapse operation for TL functions and a necessary and sufficient condition for collapsibility, which can achieve an average of 18% gate count reduction on top of synthesized TL circuits. It also proposed 2 ways to verify the collapsed TL circuits: TL-to-MUX tree, TL-to-PB constraints. Both ways can perform equivalence checking of TL circuits, and therefore verify the correctness of collapsed TL circuits.
	
  In our final project, we tried to re-implement the collapse operation for TL functions and the TL-to-MUX tree conversion for verification.


## Installation
Type `make` in `LSV-Final-Project` to complie and the executable file is `abc`

## Inside dir
```
In LSV-Final-Project  
├── src
│   ├── ext-final
│       └── threshold.h: Header file. 
│       └── thresholdcmd.cpp: Construct command function.  
│       └── aig2th.cpp: Implement convert combinational ckt to threshold logic ntk. 
│       └── collapse.cpp: Implement collapse threshold logic ntk.
│       └── th2mux.cpp: Implement convert threshold logic ntk to mux trees.  
├── collapse
│   ├── benchmark: Benchmarks we used. Same as paper.
│   ├── result: Our implementation results.
└── README.md
```

## Commands

### Synthesis
- `lsv_aig2th` (alias as `a2t`): convert combinational ckt to threshold logic ntk.
- `lsv_collapse` (alias as `col`): collapse threshold logic ntk.
### Verification
- `lsv_th2mux`  (alias as `t2m`): convert threshold logic ntk to mux trees.
### Helping function
- `lsv_print_th` (alias as `pth`): print number of PI/PO/TLG.

### Reproduce our experiment
#### Produce comparison result.
1. Read in .blif file. (Abc original command `read`)
```
abc01> read collapse/benchmark/iscas_itc/b19.blif
```
2. Make ckt combinational. (Abc original command `comb`)
```
abc02> comb
```
3. Convert circuit to threshold logic network.
```
abc03> lsv_aig2th
```
4. Convert threshold logic network to mux trees. 
```
abc04> lsv_th2mux
```
5. Write .aig file. (Abc original command `write`)
```
abc05> write collapse/result/b19_before.aig
```

#### Produce experimental result and verify.
1. Read in .blif file. (Abc original command `read`)
```
abc01> read collapse/benchmark/iscas_itc/b19.blif
```
2. Make ckt combinational. (Abc original command `comb`)
```
abc02> comb
```
3. Convert circuit to threshold logic network.
```
abc03> lsv_aig2th
```
4. Collapse threshold logic network. 
```
abc04> lsv_collapse
```
5. Convert threshold logic network to mux trees. 
```
abc05> lsv_th2mux
```
6. Write .aig file. (Abc original command `write`)
```
abc06> write collapse/result/b19_after.aig
```
7. Verify. (Abc original command `cec`)
```
abc07> cec collapse/result/b19_before.aig collapse/result/b19_after.aig
```

#### Using alias for fast run 
1. revise benckmark file path in `abc.rc` for commands.
```
alias read1 "read collapse/benchmark/iscas_itc/b22.blif"
alias wbe "w collapse/result/b19_before.aig"
alias waf "w collapse/result/b19_after.aig"
alias teq "cec collapse/result/b22_before.aig collapse/result/b22_after.aig"
```
2. Start with `./abc` and type `rbe` to produce comparison result.
2. Start with `./abc` and type `raf` to produce experimental result and verify..

## Experiments
Please see our report for detail information.
