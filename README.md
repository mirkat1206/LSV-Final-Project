# LSV-Final-Project
ABC: System for Sequential Logic Synthesis and Formal Verification

Re-implement “Constraint Solving for Synthesis and Veriﬁcation of Threshold Logic Circuits”


## Introduction


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
2. Start with `./abc` and run `rbe` to produce comparison result.
2. Start with `./abc` and run `raf` to produce experimental result and verify..

## Experiments


