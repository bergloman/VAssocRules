#pragma once

///////////////////////////////////////////////////////////////////
//
// Author: Viktor Jovanoski
// Date  : 12.04.1998, Ljubljana revised 23.10.1998
// Module: Implementaton of class for execution of algorithm APRIORI
///////////////////////////////////////////////////////////////////

#include "prune.h"

namespace VikApriori {

    typedef VikStd::TPair<int, int> TItemRef;

    ///////////////////////////////////////////////////////////////////
    // Class AprioriOneStep: performs one step of apriori algorithm
    // Usage:   SetItemsNumber(x) - set number of items
    //          SetLevel(x)       - size of current itemsets
    //          NewLevel(ck,ck+1)
    ///////////////////////////////////////////////////////////////////

    class TAprioriOneStep {

        int items_number, level;
        bool consistent2(TItemset& a, TListOfItemsets& l);
        bool equivalent(TItemset& a, TItemset& b);
    public:

        VikStd::TVec<TItemRef> *m_ItemRef; // tell which attribute and value is an item refering to

        TAprioriOneStep() {
            items_number = 0;
        };
        ~TAprioriOneStep() {
            ;
        };
        void SetItemsNumber(int a) {
            items_number = a;
        };
        void SetLevel(int a) {
            level = a;
        };
        void NewLevel2(TListOfItemsets& Ck, TListOfItemsets& Ck1);
        void Clear() {
            ;
        };
    };

    ///////////////////////////////////////////////////////////////////
    // Object that performs apriori algorithm. Interface is very simple
    // and self-explanatory.
    ///////////////////////////////////////////////////////////////////

    class TApriori {

        bool m_RunForTarget;  // algorithm is ran from finding rules for classification
        int m_ItemsCount;     // number of items in itemsets
        TListOfItemsets m_l;    // temporary list
        TListOfItemsets m_l1; // temporary list
        TListOfItemsets m_l2; // temporary list

        VikStd::TVec<int> m_ResultsHead;                // list of indexes of items in head of the rules (only one item per rule)
        TListOfItemsets m_ResultsBody;  // list of indexes of items in body of the rules
        VikStd::TVec<double> m_ResultsConf;         // list of confidences of rules with given index
        VikStd::TVec<int> m_ResultsAllCovered;  // list of numbers of all covered examples by one rule
        VikStd::TVec<int> m_ResultsPosCovered;  // list of numbers of all positiver examples, covered by one rule

        TListOfItemsets *m_Data;                // pointer to list of itemsets with data

        TAprioriOneStep one_step;
        void FindAllRulesTarget(TListOfItemsets *l1, TListOfItemsets *l2);// find all rules with target item in head on this level
        void FindAllRules(TListOfItemsets *l1, TListOfItemsets *l2); // find all rules on this level
        void GetFrequencies(TListOfItemsets *l); // finds frequencies of itemsets in list of itemsets

        VikStd::TCommon *m_pCommon;

        void InitialChecks();
        void Initialisation();

    public:

        int m_MinSup;
        int m_GenItemsetsCount;
        double m_Conf;
        int m_TargetItem;
        int m_MaxRuleLen;
        VikStd::TVec<bool> *m_UseItem;
        VikStd::TVec<TItemRef> *m_ItemRef;

        TApriori(TListOfItemsets* DB, VikStd::TCommon *common) {
            m_Data = DB;
            m_Conf = 1.0;
            m_MinSup = 1;
            m_GenItemsetsCount = 0;
            m_pCommon = common;
            m_MaxRuleLen = -1;
            m_UseItem = NULL;
            m_ItemRef = NULL;
            m_RunForTarget = false;
        };

        ~TApriori() {};

        void SetData(TListOfItemsets *Data) {
            m_Data = Data;
        };
        void Perform();
        void PerformWithTarget();

        VikStd::TVec<int> *GetResultsHead() {
            return &m_ResultsHead;
        };
        TListOfItemsets *GetResultsBody() {
            return &m_ResultsBody;
        };
        VikStd::TVec<double> *GetResultConf() {
            return &m_ResultsConf;
        };
        VikStd::TVec<int> *GetResultsAllCovered() {
            return &m_ResultsAllCovered;
        };
        VikStd::TVec<int> *GetResultsPosCovered() {
            return &m_ResultsPosCovered;
        };
    };

    ///////////////////////////////////////////////////////////////////

}
