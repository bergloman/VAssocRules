#include "itemsets.h"

namespace VikApriori {

    //-----------------------------------------------------------
    // Author: Viktor Jovanoski
    // Date	: Ljubljana, 22.06.1999
    // Reprogrammed: January 2000, Ljubjana
    //-----------------------------------------------------------

    //////////////////////////////////////////////////////////////////

    UCHAR mask2[] = { 128,64,32,16,8,4,2,1 };   // {10000000,01000000,00100000,...00000001}
    //unsigned char mask2[]={1,2,4,8,16,32,64,128};
    UCHAR mask3[] = { 255,127,63,31,15,7,3,1 }; // {11111111,01111111,00111111,..,00000001}

    TUCharManager m_glbUCharManager; // global variable

    void TUCharManager::AssertValid(int c, int i) {
        if ((c < 0) || (c >= m_Chunks.Len()) || (i < 0) || (i >= IM_CHUNK_SIZE)) {
            char bf[400];
            sprintf(bf, "Illegal itemset reference: Index overflow: Chunk=%d Index=%d", c, i);
            throw VikStd::TExc(bf);
        };
        /*if (m_Chunks[c].m_NextFreeItemset[i]>=-1){
            char bf[400];
            sprintf(bf,"Illegal itemset reference: Itemset not initialised: Chunk=%d Index=%d",c,i);
            throw TExc(bf);
        };*/
    };
    //////////////////////////////////////////////////////////////////

    TSetBitRet::TSetBitRet() {
        for (int ii = 0; ii < 256; ii++) {
            UCHAR i = ii;
            int c = 0;
            for (int j = 0; j < 8; j++) {
                UCHAR ch = (i&mask2[j]);
                if (ch != 0) c++;
            };
            array[ii] = c;
        };
    };

    //////////////////////////////////////////////////////////////////

}
