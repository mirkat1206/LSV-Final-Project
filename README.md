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

## Experiments


