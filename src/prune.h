#pragma once

#include "itemsets.h"


namespace VikApriori {

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Author:  Viktor Jovanoski
    // Date  :  Ljubljana, 26.03.1998
    // Module:  definition of pruning of itemsets
    //
    // Updates:
    // VJI - 22 December 1999 - new underlaying infrastructure, optimisations
    ///////////////////////////////////////////////////////////////////////////////////////////////


    void MarkUnwanted(TListOfItemsets& l, int Target);
    //void Find_unwanted(TListOfItemsets& l, TItemset& targets, TItemset& unw);
    //void Prune_unwanted( TListOfItemsets& in, TItemset& targets);
    void MarkUnfrequent(TListOfItemsets& in, int support);
    void PruneUnfrequent(TListOfItemsets& in);
    void SetUnpromisingTo0(TListOfItemsets& l1, TListOfItemsets& l2, TItemset& Targets);

    ////////////////////
}
