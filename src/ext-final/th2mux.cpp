#include "threshold.h"

#include <iostream>
#include <queue>
#include <map>

map<Th_Node*, Abc_Obj_t*> th2aigNode;

void sort_th() {
    vector<Th_Node*> th_list_copy;
    th_list_copy.assign(th_list.begin(), th_list.end());
    th_list.clear();
    cout << "copy size: " << th_list_copy.size() << endl;

    int i, j;
    queue<Th_Node*> pr_queue;
    th_list_copy[0]->printref = true; // const1 Node
    th_list.push_back(th_list_copy[0]); // push const1
    for (i = 0; i < th_list_copy[0]->fanouts.size(); i++) {
        pr_queue.push(th_list_copy[0]->fanouts[i]);
    }
    // reset ref -> Note: using printref
    for (i = 1; i < th_list_copy.size(); i++) {
        th_list_copy[i]->printref = false;
    }

    // push PI's fanout
    for (i = 0; i < th_PI_list.size(); i++) {
        th_PI_list[i]->printref = true;
        th_list.push_back(th_PI_list[i]); // push PI
        for (j = 0; j < th_PI_list[i]->fanouts.size(); j++) {
            pr_queue.push(th_PI_list[i]->fanouts[j]);
        }
    }
    
    // push to th_list in topological order
    Th_Node* temp;
    bool ready;
    while (!pr_queue.empty()) {
        temp = pr_queue.front();
        pr_queue.pop();
        if (!temp->printref) {
            // check if all its fanin are already print
            ready = true;
            for (i = 0; i < temp->fanins.size(); i++) {
                if (!temp->fanins[i]->printref) {
                    ready = false; break;
                }
            }
            if (ready) {
                assert(temp->type == TH_PO || temp->type == TH_NODE);
                if (temp->type == TH_NODE) { 
                    for (j = 0; j < temp->fanouts.size(); j++) {
                        pr_queue.push(temp->fanouts[j]);
                    }
                }
                th_list.push_back(temp);
                temp->printref = true;
            } else {
                pr_queue.push(temp);
            }
        } 
    }
    cout << "ori size: " << th_list.size() << endl;
    return;
}

Th_Node* createTempNode() {
    Th_Node *thObj    = new Th_Node();
    thObj->id         = 0;
	thObj->type       = TH_UNKNOWN;
	thObj->weights    = vector<int>();
	thObj->fanins     = vector<Th_Node*>();
	thObj->fanouts    = vector<Th_Node*>();
	thObj->value      = 0;
    thObj->printref   = false;
	return thObj;
}

Abc_Obj_t* thg2mux_recur(Th_Node* v, Abc_Ntk_t* pNtk_th2mux) {
    // special case: inverter or buffer
    if (v->fanins.size() == 1) {
        if (v->weights[0] == 1 && v->value == 1) { // buffer
            assert(th2aigNode.find(v->fanins[0]) != th2aigNode.end());
            return th2aigNode.find(v->fanins[0])->second;
        } else if (v->weights[0] == -1 && !v->value) { // inverter
            
        }
    }

    // line 01~04 : special case -> const1 & const2
    // line 05 : find max abs weight input
    Th_Node* max_weight_node = v->fanins[0];
    int max_weight = v->weights[0];
    int max_weight_index = 0;

    int max = 0, min = 0, size = v->fanins.size();
    int i;
    for (i = 0; i < size; ++i) {
        if (v->weights[i] > 0)  max += v->weights[i];
        if (v->weights[i] < 0)  min += v->weights[i];

        if (v->weights[i] > max_weight) {
            max_weight_node = v->fanins[i];
            max_weight = v->weights[i];
            max_weight_index = i;
        }
        // TBD: 0-weight node ??
    }
    if (max < v->value) { //const0
        return Abc_ObjNot(Abc_AigConst1(pNtk_th2mux));
    } else if (min > v->value) { // const1
        return Abc_AigConst1(pNtk_th2mux);
    }

    // line 06 : create a mux gate 
    Abc_Obj_t* root;
    // positive & negative cofactor
    Th_Node* pos_v = createTempNode();
    Th_Node* neg_v = createTempNode();
    pos_v->value = v->value-max_weight;
    neg_v->value = v->value;
    for (i = 0; i < v->fanins.size(); i++) {
        if (i != max_weight_index) {
            pos_v->fanins.push_back(v->fanins[i]);
            pos_v->weights.push_back(v->weights[i]);
            neg_v->fanins.push_back(v->fanins[i]);
            neg_v->weights.push_back(v->weights[i]);
        }
    }

    // line 07~09 : set controlling input/data zero input/data one input
    assert(th2aigNode.find(max_weight_node) != th2aigNode.end());
    root = Abc_AigMux(pNtk_th2mux->pManFunc, th2aigNode[max_weight_node], thg2mux_recur(pos_v), thg2mux_recur(neg_v));
    // line 10 : set primary output
    return root;
}

void Lsv_th2mux() {
    int i;
    // sort th_list in topologocal order
    sort_th();

    // create mux network
    Abc_Ntk_t * pNtk_th2mux;
    char buf[1000];
    pNtk_th2mux = Abc_NtkAlloc( ABC_NTK_STRASH , ABC_FUNC_AIG , 1 );
    sprintf(buf , "th2mux");
    pNtk_th2mux->pName = Extra_UtilStrsav(buf);

    // for each PI/PO/const1 create gate
    th2aigNode[th_list[0]] = Abc_AigConst1(pNtk_th2mux);
    for (i = 0; i < th_list.size(); i++) {
        if (th_list[i]->type == TH_PI) {
            th2aigNode[th_list[i]] = Abc_NtkCreatePi(pNtk_th2mux);
        }
        else if (th_list[i]->type == TH_PO) {
            th2aigNode[th_list[i]] = Abc_NtkCreatePo(pNtk_th2mux);
        }
    }

    // for each TLG: call convertTLGMUX
    for (i = 0; i < th_list.size(); i++) {
        if (th_list[i]->type == TH_NODE) {
            th2aigNode[th_list[i]] = thg2mux_recur(th_list[i], pNtk_th2mux);
        }
    }

    // connect Po

    
}