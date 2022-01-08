#include "threshold.h"
#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"

vector<Th_Node*> th_list;
int th_globalref;

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
    printf("\tMake network comb and AIG...\n");
    if ( !Abc_NtkIsComb( pNtk ) ) {
        Abc_NtkMakeComb( pNtk , 0 ); 
    }
    if ( !Abc_NtkIsStrash( pNtk ) ) {
        pNtk = Abc_NtkStrash( pNtk, 0, 0, 0 );
        assert (pNtk != NULL);
    }
    Lsv_aig2Th(pNtk);
    return 0;

usage:
    Abc_Print(-2, "usage: lsv_aig2th [-h]\n");
    Abc_Print(-2, "\t        convert AIG to threshold network\n");
    Abc_Print(-2, "\t-h    : print the command usage\n");
    return 1;
}

int Lsv_CommandCollapse(Abc_Frame_t* pAbc, int argc, char** argv) {
    Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
    int c, bound;
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
    Lsv_collapse(th_list);
    return 0;

usage:
    Abc_Print(-2, "usage: lsv_collapse [-h]\n");
    Abc_Print(-2, "\t        collapse the threshold network to directly connect PI and PO\n");
    Abc_Print(-2, "\t-b num  bound to the fanout size\n");
    Abc_Print(-2, "\t-h    : print the command usage\n");
    return 1;
}

void Lsv_PrintTh(vector<Th_Node*>& th_list) {
    printf("Print threshold list...\n");
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
  Lsv_PrintTh(th_list);
  return 0;

usage:
  Abc_Print(-2, "usage: lsv_print_nodes [-h]\n");
  Abc_Print(-2, "\t        prints the nodes in the network\n");
  Abc_Print(-2, "\t-h    : print the command usage\n");
  return 1;
}



