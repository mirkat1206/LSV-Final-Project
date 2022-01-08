#include "threshold.h"

bool Lsv_skip_node(Th_Node* p) {
    if (p == NULL)  // NULL node
        return true;
    if (p->type != TH_NODE) // PI, PO, CONST
        return true;
    if (p->ref == th_globalref) // marked
        return true;
    // const node
    int max = 0, min = 0, size = p->fanins.size();
    for (int i = 0; i < size; ++i) {
        if (p->weights[i] > 0)  max += p->weights[i];
        if (p->weights[i] < 0)  max += p->weights[i];
        // 0-weight node    //TODO: why???
        if (p->weights[i] == 0)
            return true;
    }
    return (max < p->value || min >= p->value);
}

bool Lsv_is_collapsable(Th_Node* p) {
    //TODO
    return true;
}

void Lsv_collapse2fanouts(Th_Node* u) {
    // 07: foreach fanout t of  u
    // 08: w := CollapseNode(u,t)
    // 09: unmark w
    // 10: V := V \ {t} U {w}
}

void Lsv_collapse(int max_bound) {
    // input: a TL circuit G = (V, E) and a bound B
    // output: a collapsed TL circuit G' = (V', E')    
    // *** set a bound to the fanout size and iteratively increases the bound ***
    Th_Node* v;
    Th_Node* u;
    Th_Node* temp;
    for (int bound = 1; bound <= max_bound; ++bound) {
        // 01: unmark every v of V;
        int size = th_list.size();
        for (int i = 0; i < size; ++i)
            v = th_list[i];
            // 
            v->ref = 1 - th_globalref;
        // 02: while somve v of V is unmarked
        bool flag;
        do {
            flag = false;
            // 03: foreach v of V
            for (int i = 0; i < size; ++i) {
                // some special node cannot be merged
                if (Lsv_skip_node(v))  continue;
                // 04: foreach fanin u of v
                int size1 = v->fanins.size();
                for (int j = 0; j < size1; ++j) {
                    u = v->fanins[j];
                    // 05: if |fanouts(u)| <= B
                    if (u->fanouts.size() > bound)
                        continue;
                    // 06: if u can be collapsed to all of its fanouts
                    if (Lsv_is_collapsable(u)) {
                        Lsv_collapse2fanouts(u);
                    }
                    // 11: V := V \ {u}
                    temp = th_list[j];
                    th_list[j] = 
                }

            }
        } while(flag);
        
        
        // 12: continue
        // 13: if u is the last fanin of v
        // 14: mark v
        // 15: return (V,E)
    }
}