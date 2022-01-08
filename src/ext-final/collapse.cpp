#include "threshold.h"

bool Lsv_skip_node(Th_Node* v) {
    if (v == NULL)  // NULL node
        return true;
    if (v->type != TH_NODE) // PI, PO, CONST
        return true;
    if (v->ref == globalref) // marked
        return true;
    // const node
    int max = 0, min = 0, size = v->fanins.size();
    for (int i = 0; i < size; ++i) {
        if (v->weights[i] > 0)  max += v->weights[i];
        if (v->weights[i] < 0)  max += v->weights[i];
        // 0-weight node    //TODO: why???
        if (v->weights[i] == 0)
            return true;
    }
    return (max < v->value || min >= v->value);
}

bool Lsv_is_collapsable(Th_Node* u) {
    //TODO
    return true;
}

int Lsv_get_fanin_num(Th_Node* out, Th_Node* in) {
    int size = out->fanins.size();
    for (int i = 0; i < size; ++i) {
        if (out->fanins[i] == in)
            return i;
    }
    return 0;
}


bool Lsv_collapse2fanouts(Th_Node* u, int bound) {
    // 06: if u can be collapsed to all of its fanouts
    if (Lsv_is_collapsable(u) == false)
        return 0;
    // 07: foreach fanout t of u
    Th_Node* t;
    int size = u->fanouts.size();
    for (int i = 0; i < size; ++i) {
        t = u->fanouts[i];
        //TODO
        // 08: w := CollapseNode(u,t)
        Lsv_get_fanin_num(t, u);
        // 09: unmark w
        // 10: V := V \ {t} U {w}

    }
}

void Lsv_collapse(int max_bound) {
    // input: a TL circuit G = (V, E) and a bound B
    // output: a collapsed TL circuit G' = (V', E')    
    // *** set a bound to the fanout size and iteratively increases the bound ***
    Th_Node* v;
    Th_Node* u;
    for (int bound = 1; bound <= max_bound; ++bound) {
        // 01: unmark every v of V;
        int size = th_list.size();
        for (int i = 0; i < size; ++i) {
            v = th_list[i];
            // 
            v->ref = 1 - globalref;
        }
        // 02: while some v of V is unmarked
        bool flag;
        do {
            flag = false;
            // 03: foreach v of V
            for (int i = 0; i < size; ++i) {
                v = th_list[i];
                // some special node cannot be merged
                if (Lsv_skip_node(v))  continue;
                // 04: foreach fanin u of v
                int size1 = v->fanins.size();
                for (int j = 0; j < size1; ++j) {
                    u = v->fanins[j];
                    // 05: if |fanouts(u)| <= B
                    if (u->fanouts.size() > bound)
                        continue;
                    if (Lsv_collapse2fanouts(u, bound)) {
                        flag = true;
                        //TODO
                    }
                    // 11: V := V \ {u}
                    
                }
            }
        } while(flag);        
        // 12: continue
        // 13: if u is the last fanin of v
        // 14: mark v
        // 15: return (V,E)
    }
}