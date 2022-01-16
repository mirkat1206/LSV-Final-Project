#include "threshold.h"
#include <iostream>

void print_node(Th_Node* node) {
    int i;
    cout << "======================" << endl;
    cout << "Print node: " << node->id << ((node->ref == globalref) ? " (marked)" : " (unmarked)")<< endl;
    switch (node->type)
    {
        case TH_PI: 
            cout << "Type: PI" << endl;
            break;
        case TH_PO: 
            cout << "Type: PO" << endl;
            break;
        case TH_NODE: 
            cout << "Type: Node" << endl;
            break;
        case TH_CONST1: 
            cout << "Type: Const1" << endl;
            break;
        default: cout << "unknown" << endl;
            break;
    }
    cout << "Number of fanins: " << node->fanins.size();
    if (node->fanins.size() <= 10) {
        cout << "(" ;
        for (i = 0; i < node->fanins.size(); i++) {
            cout << " " << node->fanins[i]->id << "(w: " << node->weights[i] << ")";
        }
        cout << " )";
    }
    cout << endl;
    cout << "Number of fanouts: " << node->fanouts.size();
    if (node->fanouts.size() < 10) {
        cout << "(" ;
        for (i = 0; i < node->fanouts.size(); i++) {
            cout << " " << node->fanouts[i]->id;
        }
        cout << " )";
    }
    cout << endl;
    cout << "======================" << endl;
}

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
    // cout << "Lsv_get_fanin_num" << endl;
    // print_node(u); print_node(v);
    int size = v->fanins.size();
    for (int i = 0; i < size; ++i) {
        if (v->fanins[i] == u)
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
    return (numerator/denominator + (numerator%denominator != 0));
}

// ------------------
KL_Pair* Lsv_calculateKL(Th_Node* u, Th_Node* v,int n_fanin, int weight, bool f_invert) {
    assert(!Lsv_skip_node(u));
    assert(!Lsv_skip_node(v));
    // 1. check conditions
    bool condition[2];
    condition[0] = false;
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
        else min_fu += u->weights[i];
    } 
    int b1 = v->weights[n_fanin];

    // 3. compute K and L
    int Tu_min_fu = u->value - min_fu;
    int max_fu_Tu = max_fu - u->value;
    bool flag1 = (1 > max_fu_Tu*(b1-1)) ? 1 : 0;
    bool flag2 = (b1 > (b1-1)*Tu_min_fu) ? 1 : 0;
    KL_Pair* pair = new KL_Pair();
    pair->k = -1;
    pair->l = -1;
    if (condition[0] && condition[1]) { // inq1~3
        if (flag1 && flag2) {
            pair->l = std::max(find_int((max_fu_Tu + 1), (1-(max_fu_Tu*(b1 - 1)))), find_int(Tu_min_fu, (b1 - ((b1 - 1)*Tu_min_fu))));
            pair->k = pair->l*(b1 - 1) + 1;
        } 
    } else if (condition[0]) { // inq1,3
        if (flag1) {
            pair->l = find_int((max_fu_Tu + 1), (1 - (max_fu_Tu*(b1 - 1))));
            pair->k = pair->l*(b1 - 1) + 1;
        } 
        // else {
        //     printf("calc KL condition weird!");
        // }
    } else if (condition[1]) { // inq 2,3
        if (flag2) {
            pair->l = find_int(Tu_min_fu, (b1 - ((b1 - 1)*Tu_min_fu)));
            pair->k = pair->l*(b1 - 1) + 1;
        } 
        // else {
        //     printf("calc KL condition weird!");
        // }
    } else { // inq 3 only
        pair->l = 1;
        pair->k = b1;
    }
    return pair;
}

// ------------------

bool Lsv_is_pair_collapsable(Th_Node* u, Th_Node* v) {
    assert(!Lsv_skip_node(u));
    assert(!Lsv_skip_node(v));

    /* u: fanin <--> v: fanout */
    int n_fanin = Lsv_get_fanin_num(u, v);
    int weight  = v->weights[n_fanin];
    bool f_invert  = false;
    Th_Node* new_u;
    if (weight < 0) {
        new_u = Lsv_invert(u);
        weight *= -1;
        f_invert = true;
    }
    else new_u = u;
    KL_Pair* kl_pair = Lsv_calculateKL(new_u, v, n_fanin, weight, f_invert);
    if (f_invert) delete new_u;
    if (kl_pair->l == -1 && kl_pair->k == -1) return false;
    else return true;
}

bool Lsv_is_collapsable(Th_Node* u, int bound) {
    /* u: fanin <--> v: fanout */
    if (bound == -1);   // no limit
    else if (u->fanouts.size() >= bound)
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

int get_fanout_num(Th_Node* in, Th_Node* out) {
    for (int i=0; i < in->fanouts.size(); i++) {
        if (in->fanouts[i] == out) return i;
    }
    assert(0);
    return -1;
}

bool Lsv_collapse2fanouts(Th_Node* u, int bound) {
    assert(!Lsv_skip_node(u));
    // 06: if u can be collapsed to all of its fanouts
    if (Lsv_is_collapsable(u, bound) == false)
        return false;  
    // 07: foreach fanout t of u
    int index;
    Th_Node *t;
    bool f_invert, dup;
    KL_Pair* pair;
    int i, j, k;
    Th_Node *invert_u = Lsv_invert(u);

    int size = u->fanouts.size();
    for (i = 0; i < size; ++i) { // u->t
        t = u->fanouts[i];
        // cout << i << ": merge u(" << u->id << ") to t(" << t->id << ")" << endl;
        // print_node(u); print_node(t);
        // ===== 08: w := CollapseNode(u,t) ===== //
        // calc KL (need to check invert condition) and decide u invert or not
        index = Lsv_get_fanin_num(u, t);
        f_invert  = false;
        if (t->weights[index] < 0) {
            t->value -= t->weights[index];
            t->weights[index] *= -1;
            f_invert = true;
        }
        // cout << "line 256: " << f_invert << endl;
        if (f_invert) {
            assert(!Lsv_skip_node(t));
            pair = Lsv_calculateKL(invert_u, t, index, t->weights[index], f_invert);
        }
        else pair = Lsv_calculateKL(u, t, index, t->weights[index], f_invert);
        
        // multiple l to all weight except fanin of u
        for (j = 0; j < t->weights.size(); j++) {
            t->weights[j] *= pair->l;
        }

        // update new value T
        if (f_invert) t->value = pair->k * invert_u->value + pair->l * t->value;
        else t->value = pair->k * u->value + pair->l * t->value;
        
        // remove origin fanin of u
        t->fanins.erase(t->fanins.begin() + index);
        t->weights.erase(t->weights.begin() + index);
        // u's fanin's fanout add t
        // Note: need to check if t is already u's fanin's fanout
        for (j = 0; j < u->fanins.size(); j++) {
            dup = false;
            for (k = 0; k < u->fanins[j]->fanouts.size(); k++) {
                if (u->fanins[j]->fanouts[k] == t) {
                    dup=true;
                    break;
                }
            }
            if (!dup) u->fanins[j]->fanouts.push_back(t);
        }
        // connect u's fanin <- t's fanin and weight*=k
        for (j = 0; j < u->fanins.size(); j++) {
            t->weights.push_back(u->weights[j] * pair->k);
            t->fanins.push_back(u->fanins[j]);
        }
        // ===== 09:unmark w(t) ===== //
        t->ref = 1 - globalref;
        // 10: V := V \ {t} U {w} -> maybe redundant?
        delete pair;
    }
    // remove all u->fanins->fanout u
    for (i = 0; i < u->fanins.size(); i++) {
        for (j = 0; j < u->fanins[i]->fanouts.size(); j++) {
            if (u->fanins[i]->fanouts[j] == u) {
                u->fanins[i]->fanouts.erase(u->fanins[i]->fanouts.begin()+j);
                break;
            }
        }
    }
    delete invert_u;
    return true;
}

void Lsv_collapse(int max_bound) {
    // input: a TL circuit G = (V, E) and a bound B
    // output: a collapsed TL circuit G' = (V', E')    
    // *** set a bound to the fanout size and iteratively increases the bound ***
    Th_Node* v;
    Th_Node* u;
    Lsv_PrintTh(0);
    for (int bound = 1; bound <= max_bound; ++bound) {
        cout << "bound: " << bound << endl;
        // 01: unmark every v of V;
        for (int i = 0; i < th_list.size(); ++i) {
            v = th_list[i];
            v->ref = 1 - globalref;
        }
        // 02: while some v of V is unmarked
        bool f_has_collapsed;
        do {
            f_has_collapsed = false;
            // 03: foreach v of V
            for (int i = 0; i < th_list.size(); ++i) {
                v = th_list[i];
                // some special node cannot be merged
                if (Lsv_skip_node(v))  continue;
                // 04: foreach fanin u of v
                for (int j = 0; j < v->fanins.size(); ++j) {
                    u = v->fanins[j];
                    // some special node cannot be merged
                    if (Lsv_skip_node(u))  continue;
                    // 05: if |fanouts(u)| <= B
                    if (u->fanouts.size() >= bound)
                        continue;
                    // 06 ~ 11
                    int v_ori_size = v->fanins.size()-1; // -1 for u
                    // cout << "line 267" << endl;
                    if (Lsv_collapse2fanouts(u, bound)) {
                        f_has_collapsed = true;
                        // cout << "remove in bound: " << bound << endl;
                        // print_node(u);
                        // if (u->fanins.size() < 3) {
                        //     cout << "print node: " << bound << endl;
                        //     for (int a = 0 ; a < u->fanins.size(); a++) {
                        //         print_node(u->fanins[a]);
                        //     }
                        // }
                        // Lsv_PrintTh(0);
                        // 11: V := V \ {u} -> redundant?
                        for (int k = 0; k < th_list.size(); k++) {
                            if (th_list[k] == u) {
                                th_list.erase(th_list.begin()+k);
                                delete u;
                                break;
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
                        // 12: continue to next v -> redundant
                    }
                    // 13: if u is the last fanin of v
                    if (j == v->fanins.size()- 1) {
                        // 14: mark v
                        v->ref = globalref;
                    }                    
                }
            }
        } while(f_has_collapsed);   
        Lsv_PrintTh(0);     
    }
    // 15: return (V,E)
}