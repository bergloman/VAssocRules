#include "rulesclassifier.h"

namespace VikRuleClassifiers {

    ///////////////////////////////////////////////////////////

    void TRulesClassifier::CheckData() {
        if (m_RulesCount <= 0)
            throw VikStd::TExc("TRulesClassifier: CheckData(): m_RulesCount <= 0");

        if (m_ClassAttr < 0)
            throw VikStd::TExc("TRulesClassifier: CheckData(): m_ClassAttr < 0");

        if (m_DataTable == NULL)
            throw VikStd::TExc("TRulesClassifier: CheckData(): m_DataTable==NULL");

        if (m_DataTable->Attrs.Len() <= m_ClassAttr)
            throw VikStd::TExc("TRulesClassifier: CheckData(): invalid m_ClassAttr (out of bounds)");
        if (m_DataTable->Attrs[m_ClassAttr].AttrType != VikStd::atDiscr)
            throw VikStd::TExc("TRulesClassifier: CheckData(): invalid m_ClassAttr (attribute not discrete)");

        if (m_Input == NULL)
            throw VikStd::TExc("TRulesClassifier: CheckData(): m_Input==NULL");

        for (int i = 0; i < m_Input->Len(); i++)
            m_Input->At(i).CheckConsistency(m_DataTable, m_ClassAttr);

        //m_ResultingClassifier.Clear();
    };

    ///////////////////////////////////////////////////////////

    void TRulesClassifier::CreateDefaultClass() {
        VikStd::TIntVec v;
        int i;
        VikStd::TAttribute *at = &(m_DataTable->Attrs[m_ClassAttr]);

        for (i = 0; i < at->ValNames.Len(); i++) v.Add(0);

        // count class occurances
        for (i = 0; i < at->iVec.Len(); i++)
            if (m_ExWindow.IsIn(i))
                v[at->iVec[i]]++;

        // find maximum
        int m = 0, max = v[0];
        for (i = 1; i < v.Len(); i++)
            if (max < v[i]) {
                m = i;
                max = v[i];
            };

        TRule r;
        r.m_Head.m_Type = VikStd::atDiscr;
        r.m_Head.m_Attr = m_ClassAttr;
        r.m_Head.m_iVal = m;

        m_ResultingClassifier.Add(r);
    };

    ///////////////////////////////////////////////////////////

    void TRulesClassifier::UseAllRules() {
        CheckData();

        int i;
        VikStd::TVec<TRule> rvec;
        for (i = 0; i < m_Input->Len(); i++)
            rvec.Add(m_Input->At(i));
        m_ExWindow.Resize(m_DataTable->Attrs[0].Length());

        for (i = 0; rvec.Len() > 0; i++) {
            for (int j = 0; j < rvec.Len(); j++)
                rvec.At(j).GetCoverage(m_DataTable, &m_ExWindow);
            rvec.Sort();
            TRule r = rvec.Last();
            rvec.Delete(rvec.Len() - 1);
            r.RemoveCoveredExamples(m_DataTable, &m_ExWindow);
            m_ResultingClassifier.Add(r);
        };

        if (m_DefaultClass) // create default class rule
            //for (i=0; i<m_ResultingClassifier.Len(); i++)
            //m_ResultingClassifier[i].RemoveCoveredExamples(m_DataTable,&m_ExWindow);
            CreateDefaultClass();

        /*
        int i;
        for (i=0; i<m_Input->Len(); i++) // copy all rules
            m_ResultingClassifier.Add(m_Input->At(i));

        m_ResultingClassifier.Sort(false);

        if (m_DefaultClass){ // create default class rule
            m_ExWindow.Resize(m_DataTable->Attrs[0].Length());
            for (i=0; i<m_ResultingClassifier.Len(); i++)
                m_ResultingClassifier[i].RemoveCoveredExamples(m_DataTable,&m_ExWindow);
            CreateDefaultClass();
        };*/
    };

    ///////////////////////////////////////////////////////////

    void TRulesClassifier::UseNRulesForAllClasses() {
        CheckData();

        int i;
        VikStd::TVec<TRule> rvec;
        for (i = 0; i < m_Input->Len(); i++)
            rvec.Add(m_Input->At(i));
        m_ExWindow.Resize(m_DataTable->Attrs[0].Length());

        for (i = 0; ((i < m_RulesCount) && (rvec.Len() > 0)); i++) {
            for (int j = 0; j < rvec.Len(); j++)
                rvec.At(j).GetCoverage(m_DataTable, &m_ExWindow);
            rvec.Sort();
            TRule r = rvec.Last();
            rvec.Delete(rvec.Len() - 1);
            r.RemoveCoveredExamples(m_DataTable, &m_ExWindow);
            m_ResultingClassifier.Add(r);
        };

        if (m_DefaultClass) { // create default class rule
            m_ExWindow.Resize(m_DataTable->Attrs[0].Length(), true);
            for (i = 0; i < m_ResultingClassifier.Len(); i++)
                m_ResultingClassifier[i].RemoveCoveredExamples(m_DataTable, &m_ExWindow);
            CreateDefaultClass();
        };
    };

    ///////////////////////////////////////////////////////////

    void TRulesClassifier::UseNRulesForEachClass() {
        CheckData();
        int i, j, k;
        for (k = 0; k < m_DataTable->Attrs[m_ClassAttr].ValNames.Len(); k++) { // for each class

            VikStd::TVec<TRule> rvec;
            for (i = 0; i < m_Input->Len(); i++) // copy all rules for class k
                if (m_Input->At(i).m_Head.m_iVal == k)
                    rvec.Add(m_Input->At(i));

            m_ExWindow.Resize(m_DataTable->Attrs[0].Length());

            for (i = 0; ((i < m_RulesCount) && (rvec.Len() > 0)); i++) {
                for (j = 0; j < rvec.Len(); j++)
                    rvec.At(j).GetCoverage(m_DataTable, &m_ExWindow);
                rvec.Sort();
                TRule r = rvec.Last();
                rvec.Delete(rvec.Len() - 1);
                r.RemoveCoveredExamples(m_DataTable, &m_ExWindow);
                m_ResultingClassifier.Add(r);
            };
        };

        m_ResultingClassifier.Sort();

        if (m_DefaultClass) { // create default class rule
            m_ExWindow.Resize(m_DataTable->Attrs[0].Length());
            for (i = 0; i < m_ResultingClassifier.Len(); i++)
                m_ResultingClassifier[i].RemoveCoveredExamples(m_DataTable, &m_ExWindow);
            CreateDefaultClass();
        };
    };

    //////////////////////////////////////////////////////////////

    void TRulesClassifier::NadaAlgo() {
        CheckData();
        int i, j, k;

        VikStd::TVec<int> counter, weight;
        for (i = 0; i < m_DataTable->Attrs[0].Len(); i++)
            counter.Add(1);

        VikStd::TVec<TRule> rvec;
        for (i = 0; i < m_Input->Len(); i++)
            rvec.Add(m_Input->At(i));

        for (i = 0; i < m_RulesCount; i++) { // select m_RulesCount rules
            if (rvec.Len() > 0) {
                weight.Clear();
                for (j = 0; j < rvec.Len(); j++) { // for each candidate rule
                    weight.Add(0);
                    for (k = 0; k < counter.Len(); k++) { // for each example
                        if (rvec[j].CoverEx(m_DataTable, k) != false)
                            weight[j] += 1 / counter[k];
                    };
                };
                // find best rule
                int pos;
                weight.MaxIndex(pos);
                // add selected rule to output ruleset
                m_ResultingClassifier.Add(rvec[pos]);
                // update counter
                for (k = 0; k < counter.Len(); k++) {
                    if (rvec[pos].CoverEx(m_DataTable, k) != false)
                        counter[k]++;
                };
                // eliminate selected rule
                rvec.Swap(pos, rvec.Len() - 1);
                rvec.DeleteLast();
            };
        };
    };

    //////////////////////////////////////////////////////////////

    void TRulesClassifier::TestClassifier() {

        m_RulesCount = 5;
        CheckData();
        m_Results.Clr();

        int dim = m_DataTable->Attrs[m_ClassAttr].ValNames.Len();
        m_Results.m_ConfusionMatrix.Gen(dim + 1, dim);
        m_Results.m_ConfusionMatrix.PutToAll(0);

        for (int i = 0; i < m_DataTable->Attrs[0].Len(); i++) { // for each example
            int j = 0;
            while (j < m_Input->Len()) { // try rules as they are ordered
                if (m_Input->At(j).CoverEx(m_DataTable, i)) {

                    int pred = m_Input->At(j).m_Head.m_iVal;
                    int real = m_DataTable->Attrs[m_ClassAttr].iVec[i];

                    if (pred == real) {
                        m_Results.m_Correct++;
                    } else {
                        m_Results.m_Wrong++;
                    };

                    m_Results.m_ConfusionMatrix.At(pred, real)++;
                    j = m_Input->Len() + 2;
                };
                j++;
                if (j == m_Input->Len()) {
                    // if this is the case, it means that no rule fired
                    // so example is unclassified
                    m_Results.m_ConfusionMatrix.At(dim, m_DataTable->Attrs[m_ClassAttr].iVec[i])++;
                    m_Results.m_Wrong++;
                    m_Results.m_Unclassified++;
                };
            };
        };

        if (m_Results.m_Correct + m_Results.m_Wrong > 0) {
            m_Results.m_Accuracy = double(m_Results.m_Correct) / (m_Results.m_Correct + m_Results.m_Wrong);
        } else m_Results.m_Accuracy = 0;
        m_Results.GetTime();
    };

    ///////////////////////////////////////////////////////////////////////

    void TRulesClassifier::CreateNewAttribute(VikStd::TStr &a, VikStd::TVec<VikStd::TStr> &Targets)  // creates new attribute, based on rule prediction
    {
        VikStd::TAttribute at(VikStd::atDiscr, a);
        for (int j = 0; j < Targets.Len(); j++)
            at.DeclareValue(Targets[j], false);

        // check everything
        if (m_DataTable == NULL)
            throw VikStd::TExc("TRulesClassifier: CheckData(): m_DataTable==NULL");
        if (m_Input == NULL)
            throw VikStd::TExc("TRulesClassifier: CheckData(): m_Input==NULL");

        // ok, proceed
        for (int i = 0; i < m_DataTable->Attrs[0].Len(); i++) { // for each example
            int j = 0;
            while (j < m_Input->Len()) { // try rules as they are ordered
                if (m_Input->At(j).CoverEx(m_DataTable, i)) {

                    int pred = m_Input->At(j).m_Head.m_iVal;
                    at.AddDiscrStrNoCheck(Targets[pred]);
                    j = m_Input->Len() + 2; // to jump out of the while loop - ok, it is not nice, but....
                };
                j++;
                if (j == m_Input->Len()) {
                    // if this is the case, it means that no rule fired
                    // so example is unclassified
                    at.iVec.Add(-1); // missing value
                };
            };
        };

        m_DataTable->Attrs.Add(at);
    };

    ///////////////////////////////////////////////////////////////////////
    // creates new attribute, based on rule prediction with voting

    void TRulesClassifier::CreateNewAttributeVoting(VikStd::TStr &a, VikStd::TVec<VikStd::TStr> &Targets) {
        int i, j;
        VikStd::TAttribute at(VikStd::atCon, a);

        // check everything
        if (m_DataTable == NULL)
            throw VikStd::TExc("TRulesClassifier: CreateNewAttributeVoting(): m_DataTable==NULL");
        if (m_Input == NULL)
            throw VikStd::TExc("TRulesClassifier: CreateNewAttributeVoting(): m_Input==NULL");
        if (Targets.Len() > 1)
            throw VikStd::TExc("TRulesClassifier: CreateNewAttributeVoting(): not all rules classify to the same class.");

        // ok, proceed
        for (i = 0; i < m_DataTable->Attrs[0].Len(); i++) { // for each example
            double vote = 0;
            for (j = 0; j < m_Input->Len(); j++) {
                if (m_Input->At(j).CoverEx(m_DataTable, i)) vote++;
            };
            at.dVec.Add(vote / m_Input->Len());
        };

        m_DataTable->Attrs.Add(at);
    };

}
