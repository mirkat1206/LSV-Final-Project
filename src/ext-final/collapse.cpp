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
        if (v->weights[i] < 0)  min += v->weights[i];
        // 0-weight node    //TODO: why???
        if (v->weights[i] == 0)
            return true;
    }
    return (max < v->value || min >= v->value);
}

Th_Node* Lsv_copy(Th_Node* u) {
    Th_Node* new_u = new Th_Node();
    new_u->id = u->id;
    new_u->ref = u->ref;
    new_u->type = u->type;
    int size;
    size = u->weights.size();
    for (int i = 0; i < size; ++i) {
        new_u->weights.push_back(u->weights[i]);
    }
    size = u->fanins.size();
    for (int i = 0; i < size; ++i) {
        new_u->fanins.push_back(u->fanins[i]);
    }
    size = u->fanouts.size();
    for (int i = 0; i < size; ++i) {
        new_u->fanouts.push_back(u->fanouts[i]);
    }
    new_u->value = u->value;
    return new_u;
}

int Lsv_get_fanin_num(Th_Node* u, Th_Node* v) {
    /* u: fanin <--> v: fanout */
    int size = u->fanins.size();
    for (int i = 0; i < size; ++i) {
        if (u->fanins[i] == v)
            return i;
    }
    assert(0);
    return -1;
}

Th_Node* Lsv_invert(Th_Node* u) {
    // make a inverted copy
    Th_Node* inv_u = Lsv_copy(u);
    // invert the copy
    int size = inv_u->weights.size();
    for (int i = 0; i < size; ++i) 
        inv_u->weights[i] *= -1;
    inv_u->value = 1 - inv_u->value;
    return inv_u;
}

int find_int(int numerator, int denominator) {

}

// ------------------
KL_Pair* Lsv_calculateKL(Th_Node* u, Th_Node* v,int n_fanin, int weight, bool f_invert) {
    assert(!Lsv_skip_node(u));
    assert(!Lsv_skip_node(v));
    // 1. check conditions
    bool condition[2];
    condition[0] = false; // 
    condition[1] = false;
    
    int Tv = (f_invert) ? (v->value + weight) : (v->value);
    int max = 0, min = 0, size = v->fanins.size();
    for (int i = 0; i < size; ++i) {
        if (i == n_fanin)   
            continue;
        if (v->weights[i] > 0)  max += v->weights[i];
        if (v->weights[i] < 0)  min += v->weights[i];
    }
    if (min <= Tv - weight - 1)
        condition[0] = true;
    if (max >= Tv)
        condition[1] = true;

    // ===== coefficients ===== //
    int i;
    int max_fu = 0, min_fu = 0;
    // max{fu+} & min{fu+}
    for (i = 0; i < u->weights.size(); i++) {
        if (u->weights[i] > 0) max_fu += u->weights[i];
        else min_fu += u->weights[i]; // <=0
    } 
    int b1 = v->weights[n_fanin];

    // 3. compute K and L
    int Tu_min_fu = u->value - min_fu;
    int max_fu_Tu = max_fu;
    KL_Pair* pair = new KL_Pair();
    if (condition[0] && condition[1]) { // inq1~3
        
    } else if (condition[0]) { // inq1,3
        if (1 > max_fu_Tu*(b1-1)) {
            pair->l = find_int((max_fu_Tu+1), (1-(max_fu_Tu*(b1-1))));
            pair->k = pair->l*(b1-1)+1;
        }
    } else if (condition[1]) { // inq 2,3
        if (b1 > (b1-1)*Tu_min_fu) {
            pair->l = find_int(Tu_min_fu, (b1-((b1-1)*Tu_min_fu)));
            pair->k = pair->l*(b1-1)+1;
        } else {
            printf("condition weird!");
        }
    } else { // inq 3 only
        pair->l = 1;
        pair->k = b1;
    }
}

// ------------------

bool Lsv_is_pair_collapsable(Th_Node* u, Th_Node* v) {  //TODO
    /* u: fanin <--> v: fanout */
    int n_fanin = Lsv_get_fanin_num(u, v);
    int weight  = v->weights[n_fanin];
    bool f_invert  = false;
    if (weight < 0) {
        u = Lsv_invert(u);
        weight *= -1;
        f_invert = true;
    }
    KL_Pair* kl_pair = Lsv_calculateKL(u, v, n_fanin, weight, f_invert);
}

bool Lsv_is_collapsable(Th_Node* u, int bound) {
    /* u: fanin <--> v: fanout */
    if (bound == -1);   // no limit
    else if (u->fanouts.size() > bound)
        return false;
    
    Th_Node* v;
    int size = u->fanouts.size();
    for (int i = 0; i < size; ++i) {
        v = u->fanouts[i];
        // some special node cannot be merged
        if (Lsv_skip_node(v))
            return false;
        if (!Lsv_is_pair_collapsable(u, v)) {
            return false;
        }
    }
    return true;
}

bool Lsv_collapse2fanouts(Th_Node* u, int bound) {
    // 06: if u can be collapsed to all of its fanouts
    if (Lsv_is_collapsable(u, bound) == false)
        return false;    
    // 07: foreach fanout t of u
    int index;
    Th_Node* t;
    int size = u->fanouts.size();
    KL_Pair* pair;
    for (int i = 0; i < size; ++i) {
        t = u->fanouts[i];
        // ===== 08: w := CollapseNode(u,t) ===== //
        //TODO: may invert node
        // calc KL
        index = Lsv_get_fanin_num(t, u);
        pair = Lsv_calculateKL(u, t, index, t->weights[index], 0);
        // multiple l to all weight except fanin of u
        for (int j = 0; j < t->weights.size(); j++) {
            t->weights[j] *= pair->l;
        }
        // update new value T
        t->value = pair->k * u->value + pair->l * (t->value - t->weights[index]);
        // remove origin fanin of u
        t->fanins.erase(t->fanins.begin() + index);
        t->weights.erase(t->weights.begin() + index);
        // connect u's fanin to t's fanin and weight*=k
        for (int j = 0; j < u->fanins.size(); j++) {
            t->weights.push_back(u->weights[j] * pair->k);
            t->fanins.push_back(u->fanins[j]);
        }
        // ===== 09:unmark w(t) ===== //
        t->ref = 1 - globalref;
        // 10: V := V \ {t} U {w} -> maybe redundant?
    }
    return true;
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
            v->ref = 1 - globalref;
        }
        // 02: while some v of V is unmarked
        bool f_has_collapsed;
        do {
            f_has_collapsed = false;
            // 03: foreach v of V
            for (int i = 0; i < size; ++i) {
                v = th_list[i];
                // some special node cannot be merged
                if (Lsv_skip_node(v))  continue;
                // 04: foreach fanin u of v
                for (int j = 0; j < v->fanins.size(); ++j) {
                    u = v->fanins[j];
                    // some special node cannot be merged
                    if (Lsv_skip_node(u))  continue;
                    // 05: if |fanouts(u)| <= B
                    if (u->fanouts.size() > bound)
                        continue;
                    // 06 ~ 11
                    int v_ori_size = v->fanins.size()-1; // -1 for u
                    if (Lsv_collapse2fanouts(u, bound)) {
                        f_has_collapsed = true;
                        // 11: V := V \ {u} -> redundant?
                        for (int k = 0; k < th_list.size(); k++) {
                            if (th_list[k] == u) {
                                delete u;
                                th_list.erase(th_list.begin()+k);
                            }
                        }
                        // Note: consider nondisjoint fanins for v
                        for (int k = v_ori_size; k < v->fanins.size(); k++) {
                            for (int l = 0; l < v_ori_size; l++) {
                                if (v->fanins[k] == v->fanins[l]) { // merge k to l
                                    v->weights[l] += v->weights[k]; // summed up
                                    v->weights.erase(v->weights.begin()+k); // remove fanin of k
                                    v->fanins.erase(v->fanins.begin()+k);
                                    k--; break;
                                }
                            }
                        }
                        // 12: continue to next v
                        break;
                    }
                    // 13: if u is the last fanin of v
                    if (j == size - 1) {
                        // 14: mark v
                        v->ref = globalref;
                    }                    
                }
            }
        } while(f_has_collapsed);        
    }
    // 15: return (V,E)
}