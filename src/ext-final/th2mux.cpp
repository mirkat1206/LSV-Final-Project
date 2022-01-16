#include "threshold.h"

#include <iostream>
#include <queue>
#include <map>

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

Abc_Obj_t* convertTLG2MUX(Th_Node* v, Abc_Ntk_t* pNtk_th2mux) {
    // line 01~04 : special case
    // line 05 : find max abs weight input
    // line 06 : create a mux gate 
    // line 07 : set controlling input
    // line 08 : data zero input
    // line 09 : data one input
    // line 10 : set primary output
}

void Lsv_th2mux() {
    map<Th_Node*, Abc_Obj_t*> th2aigNode;
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
            th2aigNode[th_list[i]] = convertTLG2MUX(th_list[i], pNtk_th2mux);
        }
    }

    // connect Po

    
}