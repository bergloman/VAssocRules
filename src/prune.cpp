#include "prune.h"


namespace VikApriori {

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Author:  Viktor Jovanoski
    // Date  :  Ljubljana, 26.03.1998
    // Module:  implementation of pruning of itemsets
    //
    // Updates:
    // VJI - 22 December 1999 - new underlaying infrastructure, optimisations
    // VJI - January 2000 - work continues
    ///////////////////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////
    // Detection of unwanted itemset
    // Unwanted items are items, that don't apear in any itemset with
    // any of the target items
    // Unwanted itemsets are those itemsets that contain at least
    // one unwanted item.
    //////////////////////////////////////////////////////////////

    void MarkUnwanted(TListOfItemsets& l, int Target) {
        TItemset unw;
        unw.Init();
        int i;

        unw.SetAllTrue();
        for (i = 0; i < l.Len(); i++)
            if ((l[i].IsSetItem(Target)) && (l[i].m_Freq > 0))
                unw.RemoveElems(l[i]);

        for (i = 0; i < l.Len(); i++)
            if ((l[i].IntersectionIsEmpty(unw) == false) && (l[i].m_Freq > 0))
                l[i].m_Freq = 0;
    };

    /*
    void Find_unwanted(TListOfItemsets& l, TItemset& targets, TItemset& unw){
      unw.SetAllTrue();
      for (int i=0; i<l.m_v.Len(); i++)
            if (targets.IntersectionIsEmpty(l.m_v[i])==false)
                unw.RemoveElems(l.m_v[i]);
    };

    ///////////////////////////////////////////////////////////////
    // Prunes itemsets, that contain at least one unwanted item.
    // Unwanted items are given in an itemset "targets"
    ///////////////////////////////////////////////////////////////

    void Prune_unwanted(TListOfItemsets& in, TItemset& targets){
        TVec<int> tmpIndex;
        for (int i=0; i<in.m_v.Len(); i++){
            if (in.m_v[i].IntersectionIsEmpty(targets)==false)
                tmpIndex.Add(i);
        };
        tmpIndex.Sort();
        for (int j=tmpIndex.Len()-1; j>=0; j--){
            if (tmpIndex[j]!=in.m_v.Len()-1)
                SwapItemsets(in.m_v[in.m_v.Len()-1],in.m_v[tmpIndex[j]]);
            in.m_v.DeleteLast();
        };
    };
    */

    //////////////////////////////////////////////////////////////
    // Detects itemsets, that don't meet specified treshold
    // of support (they are not frequent enough)
    //////////////////////////////////////////////////////////////


    void MarkUnfrequent(TListOfItemsets& in, int support) {
        for (int i = 0; i < in.Len(); i++)
            if (in[i].m_Freq < support)
                in[i].m_Freq = 0;
    };

    //////////////////////////////////////////////////////////////
    // Prunes itemsets, that don't meet specified treshold
    // of support (they are not frequent enough)
    //////////////////////////////////////////////////////////////


    void PruneUnfrequent(TListOfItemsets& in) {

        // VJIDBG
        /*printf("=======\n");
        for (int ii=0; ii<in.Len(); ii++)
            printf("itemset: chunk=%d index=%d\n",in[ii].m_Chunk,in[ii].m_Index);*/
            // end VJIDBG

        VikStd::TVec<int> tmpIndex;
        for (int i = 0; i < in.Len(); i++) {
            if (in[i].m_Freq == 0)
                tmpIndex.Add(i);
        };
        tmpIndex.Sort();
        for (int j = tmpIndex.Len() - 1; j >= 0; j--) {
            if (tmpIndex[j] != in.Len() - 1)
                SwapItemsets(in.Last(), in[tmpIndex[j]]);
            in.DeleteLast();
        };
        in.Sort();
    };

    /////////////////////////////////////////////////////////////
    // Sets frequency of unpromising itemsets to 0
    // Unpromising itemsets are those, for which exists their
    // proper subset that has the same frequency.
    // (But subset has to have the same target item as the original)
    /////////////////////////////////////////////////////////////

    void SetUnpromisingTo0(TListOfItemsets& l1, TListOfItemsets& l2, TItemset& Targets) {
        /*
            int BitSize=TItemset::m_glbUCharManager.GetItemsetSize();
            TItemset temp;
            temp.Init();

            for (int j=0; j<l2.m_v.Len(); j++){
                if (l2.m_v[j].m_Freq>0){
                    temp=l2.m_v[j];

                    for(int i=0; ((i<BitSize)&&(l2.m_v[j]>0)); i++){

                        if ((temp.IsSet(i)) && (Targets.IsSet(i)==false)){
                            temp.ResetItem(i);
                            int k=l1.Find(temp);
                            if (k>=0){
                                if (temp.m_Freq==l1.m_v[k].m_Freq){ // stopping criterion
                                    l2.GetCurr().freq=0;
                                    i=BitSize;
                                };
                            };
                            temp.SetItem(i);
                        };

                    };
                };
            };*/
    };
}
