#include "threshold.h"
#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"
#include <queue>

vector<Th_Node*> th_list;
vector<Th_Node*> th_PI_list;

int globalref = 1;

static int Lsv_CommandAig2Th( Abc_Frame_t * pAbc, int argc, char ** argv );
static int Lsv_CommandCollapse( Abc_Frame_t * pAbc, int argc, char ** argv );
static int Lsv_CommandPrintTh( Abc_Frame_t * pAbc, int argc, char ** argv );

void th_init(Abc_Frame_t* pAbc) {
    Cmd_CommandAdd(pAbc, "LSV", "lsv_aig2th", Lsv_CommandAig2Th, 0);
    Cmd_CommandAdd(pAbc, "LSV", "lsv_collapse", Lsv_CommandCollapse, 0);
    Cmd_CommandAdd(pAbc, "LSV", "lsv_print_th", Lsv_CommandPrintTh, 0);
}

void th_destroy(Abc_Frame_t* pAbc) {}

Abc_FrameInitializer_t frame_initializer = {th_init, th_destroy};

struct PackageRegistrationManager {
    PackageRegistrationManager() { Abc_FrameAddInitializer(&frame_initializer); }
} lsvPackageRegistrationManager;

int Lsv_CommandAig2Th(Abc_Frame_t* pAbc, int argc, char** argv) {
    Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
    int c;
    Extra_UtilGetoptReset();
    while ((c = Extra_UtilGetopt(argc, argv, "h")) != EOF) {
        switch (c) {
        case 'h':
            goto usage;
        default:
            goto usage;
        }
    }
    if (!pNtk) {
        Abc_Print(-1, "Empty network.\n");
        return 1;
    }
    printf("Make network comb and AIG...\n");
    if ( !Abc_NtkIsComb( pNtk ) ) {
        Abc_NtkMakeComb( pNtk , 0 ); 
    }
    if ( !Abc_NtkIsStrash( pNtk ) ) {
        pNtk = Abc_NtkStrash( pNtk, 0, 0, 0 );
        assert (pNtk != NULL);
    }
    Lsv_aig2th(pNtk);
    return 0;

usage:
    Abc_Print(-2, "usage: lsv_aig2th [-h]\n");
    Abc_Print(-2, "\t        convert AIG to threshold network\n");
    Abc_Print(-2, "\t-h    : print the command usage\n");
    return 1;
}

int Lsv_CommandCollapse(Abc_Frame_t* pAbc, int argc, char** argv) {
    Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
    int c, bound = -1;
    Extra_UtilGetoptReset();
    while ((c = Extra_UtilGetopt(argc, argv, "h")) != EOF) {
        switch (c) {
            case 'b': // bound
                if ( globalUtilOptind >= argc ) {
                    Abc_Print( -1, "Command line switch \"-B\" should be followed by an integer.\n" );
                    goto usage;
                }
                bound = atoi(argv[globalUtilOptind]);
                globalUtilOptind++;
                if ( bound < 1 ) goto usage;
                break;
            case 'h':
                goto usage;
            default:
                goto usage;
        }
    }
    if (!pNtk) {
        Abc_Print(-1, "Empty network.\n");
        return 1;
    }
    bound = 100;
    Lsv_collapse(bound);
    return 0;

usage:
    Abc_Print(-2, "usage: lsv_collapse [-h]\n");
    Abc_Print(-2, "\t        collapse the threshold network to directly connect PI and PO\n");
    Abc_Print(-2, "\t-b num  bound to the fanout size\n");
    Abc_Print(-2, "\t-h    : print the command usage\n");
    return 1;
}

void Lsv_PrintTh(bool flag) {
    int i, j;
    int _numPi = 0, _numPo = 0, _numNode = 0;
    queue<Th_Node*> pr_queue;
    // reset ref
    th_list[0]->printref = true; // const1 Node
    for (i = 0; i < th_list[0]->fanouts.size(); i++) {
        pr_queue.push(th_list[0]->fanouts[i]);
    }
    for (i = 1; i < th_list.size(); i++) {
        th_list[i]->printref = false;
    }

    // push PI's fanout
    for (i = 0; i < th_PI_list.size(); i++) {
        th_PI_list[i]->printref = true;
        for (j = 0; j < th_PI_list[i]->fanouts.size(); j++) {
            pr_queue.push(th_PI_list[i]->fanouts[j]);
        }
    }
    _numPi = th_PI_list.size();

    
    // print in topological order
    // printf("Print threshold logic ckt in topological order...\n");
    printf("Print summary...\n");
    Th_Node* temp;
    bool ready;
    while (!pr_queue.empty()) {
        temp = pr_queue.front();
        if (flag) printf("temp id: %d\n", temp->id);
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
                if (temp->type == TH_PO) { 
                    _numPo++; 
                }
                else if (temp->type == TH_NODE) { 
                    _numNode++; 
                    for (j = 0; j < temp->fanouts.size(); j++) {
                        pr_queue.push(temp->fanouts[j]);
                    }
                }
                temp->printref = true;
            } else {
                pr_queue.push(temp);
            }
        } 
    }

    // print total state
    printf("================================ \n");
    printf("Summary: \n");
    printf("Number of Pi: %d\n", _numPi);
    printf("Number of Po: %d\n", _numPo);
    printf("Number of Node: %d\n", _numNode);
    printf("================================ \n");

    return;
}

int Lsv_CommandPrintTh(Abc_Frame_t* pAbc, int argc, char** argv) {
  Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
  int c;
  Extra_UtilGetoptReset();
  while ((c = Extra_UtilGetopt(argc, argv, "h")) != EOF) {
    switch (c) {
      case 'h':
        goto usage;
      default:
        goto usage;
    }
  }
  if (!pNtk) {
    Abc_Print(-1, "Empty network.\n");
    return 1;
  }
  Lsv_PrintTh(0);
  return 0;

usage:
  Abc_Print(-2, "usage: lsv_print_nodes [-h]\n");
  Abc_Print(-2, "\t        prints the nodes in the network\n");
  Abc_Print(-2, "\t-h    : print the command usage\n");
  return 1;
}
