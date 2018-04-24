#ifndef bintopk_h
#define bintopk_h

#include <utils/libcdsBasics.h>

using namespace cds_utils;

namespace cds_static
{

  class wt_node;

  typedef struct {
    uint start_i;
    uint end_i;
    wt_node *node;
  } BinTopK_Item;

  class TopK_Cmp {
  public:
    bool operator() (BinTopK_Item a, BinTopK_Item b) {
      return (b.end_i-b.start_i) > (a.end_i-a.start_i);
    }
  };
};

#include <sequence/wt_node.h>
#endif
