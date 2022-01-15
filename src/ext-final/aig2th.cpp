#include "threshold.h"
#include <map>

Th_Node* createNode(Th_Node_Type _type, unsigned _id) {
    Th_Node *thObj    = new Th_Node();
    thObj->id         = _id;
	thObj->type       = _type;
	thObj->weights    = vector<int>();
	thObj->fanins     = vector<Th_Node*>();
	thObj->fanouts    = vector<Th_Node*>();
	thObj->value      = 0;
    thObj->printref   = false;
    th_list.push_back(thObj);
    if (_type == TH_PI) th_PI_list.push_back(thObj);
	return thObj;
}

void Lsv_aig2th(Abc_Ntk_t* pNtk) {
    printf("Convert aig to threshold...\n");
    Th_Node *thNode;
    Abc_Obj_t *pObj, *pFanout, *pConst1;
    map<int,  Th_Node*> _id2ThNode;
    th_list.clear();
	int i, j;

    /* create threshold node */
    // create const1 node
    // printf("create Const1 ...\n");
    pConst1 = Abc_AigConst1(pNtk);
	thNode = createNode(TH_CONST1, Abc_ObjId(pConst1));
    // printf("Const1 id: %d\n", thNode->id);
    _id2ThNode[thNode->id] = thNode;

    // create Co
    // printf("create Ci ...\n");
    Abc_NtkForEachCi(pNtk,pObj,i) {
        thNode = createNode(TH_PI, Abc_ObjId(pObj));
        // printf("Ci id: %d\n", thNode->id);
        _id2ThNode[thNode->id] = thNode;
    }

    // create Co
    // printf("create Co ...\n");
    Abc_NtkForEachCo(pNtk,pObj,i) {
        thNode = createNode(TH_PO, Abc_ObjId(pObj));
        // printf("Co id: %d\n", thNode->id);
        _id2ThNode[thNode->id] = thNode;
    }

    // create Node
    // printf("create Node ...\n");
    Abc_NtkForEachNode(pNtk,pObj,i) {
        thNode = createNode(TH_NODE, Abc_ObjId(pObj));
        // printf("node id: %d\n", thNode->id);
        _id2ThNode[thNode->id] = thNode;
    }

    /* connect fanins & fanouts */
    // printf("Connect ...\n");
    // connect const1 Fanout
    thNode = th_list[0];
    Abc_ObjForEachFanout(pConst1,pFanout,i) {
        thNode->fanouts.push_back(_id2ThNode[Abc_ObjId(pFanout)]);
    }

    // connect Po Fanin
    Abc_NtkForEachPo(pNtk,pObj,i) {
        thNode = _id2ThNode[Abc_ObjId(pObj)];
        thNode->fanins.push_back(_id2ThNode[Abc_ObjFaninId0(pObj)]);

        if (Abc_ObjFaninC0(pObj)) { // with inverter
            thNode->value = 0;
            thNode->weights.push_back(-1);
        } else { // buffer only
            thNode->value = 1;
            thNode->weights.push_back(1);
        }
	}

    // connect Pi Fanout
    Abc_NtkForEachCi( pNtk , pObj , i ) {
	    thNode = _id2ThNode[Abc_ObjId(pObj)];
        Abc_ObjForEachFanout(pObj,pFanout,j) {
            thNode->fanouts.push_back(_id2ThNode[Abc_ObjId(pFanout)]);
        }
	}

    // connect Node Fanin
    Abc_NtkForEachNode(pNtk,pObj,i) {
        thNode = _id2ThNode[Abc_ObjId(pObj)];
        thNode->fanins.push_back(_id2ThNode[Abc_ObjFaninId0(pObj)]);
        thNode->fanins.push_back(_id2ThNode[Abc_ObjFaninId1(pObj)]);

        if (!Abc_ObjFaninC0(pObj) && !Abc_ObjFaninC1(pObj)) { // [1, 1; 2]
            thNode->value = 2;
            thNode->weights.push_back(1);
            thNode->weights.push_back(1);
        } else if (Abc_ObjFaninC0(pObj) && !Abc_ObjFaninC1(pObj)) { // [-1, 1; 1]
            thNode->value = 1;
            thNode->weights.push_back(-1);
            thNode->weights.push_back(1);
        } else if (!Abc_ObjFaninC0(pObj) && Abc_ObjFaninC1(pObj)) { // [1, -1; 1]
            thNode->value = 1;
            thNode->weights.push_back(1);
            thNode->weights.push_back(-1);
        } else if (Abc_ObjFaninC0(pObj) && Abc_ObjFaninC1(pObj)) { // [-1, -1; 0]
            thNode->value = 0;
            thNode->weights.push_back(-1);
            thNode->weights.push_back(-1);
        }  
    }
	
	// connect Node Fanout
	Abc_NtkForEachNode(pNtk,pObj,i) {
	    thNode = _id2ThNode[Abc_ObjId(pObj)];
        Abc_ObjForEachFanout(pObj,pFanout,j) {
            thNode->fanouts.push_back(_id2ThNode[Abc_ObjId(pFanout)]);
        }
	}
    printf("Finish!\n");

    /* determine level */
}