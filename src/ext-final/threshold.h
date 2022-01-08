#ifndef THRESHOLD_H_
#define THRESHOLD_H_

#include "base/abc/abc.h"
#include <vector>

using namespace std;

typedef enum {
    TH_UNKNOWN,
    TH_CONST1,
    TH_PI,
    TH_PO,
    TH_NODE
} Th_Node_Type;

typedef struct Th_Node_ Th_Node;

struct Th_Node_
{
    int id;
    int ref;
    Th_Node_Type type;
    vector<int> weights;
    vector<Th_Node_*> fanins;
    vector<Th_Node_*> fanouts;
    int value;
};

extern vector<Th_Node*> th_list;
extern int globalref;

extern void Lsv_aig2th(Abc_Ntk_t*);
extern void Lsv_collapse(int max_bound);

#endif