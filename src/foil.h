#include "stdvik.h"
#include "rules.h"

namespace VikRuleClassifiers {

    //////////////////////////////////////////////////////////

    class TFoil {
    private:

        int GetLeastFreqClass(VikStd::TDataTable *tab, VikStd::TExampleWindow& ew);
        int GetMostFreqClass(VikStd::TDataTable *tab, VikStd::TExampleWindow& ew);
        void PruneRule(VikStd::TDataTable *tab, TRule& r, VikStd::TExampleWindow& ew, VikStd::TExampleWindow& new_ew);

        VikStd::TVTime m_CreationTime;
        VikStd::TVTimer m_TimeTaken;

    public:

        VikStd::TVec<TRule> m_RuleSet;
        int m_MinSup;       // minimal support - limits depth of search
        bool m_UseNegation; // create negation for discrete attributes - default=false
        int m_TargetAttr;   // index of target attribute        

        TFoil() :
            m_RuleSet(),
            m_MinSup(10),
            m_TargetAttr(-1),
            m_UseNegation(false) {};

        void Grow(VikStd::TDataTable *tab); // main FOIL function - contructs rules
        void Output(VikStd::TOut *out, VikStd::TStr &InputFile, VikStd::TDataTable *tab); // simple text output to file
        void OutputXML(VikStd::TStr &OutputFile, VikStd::TDataTable *tab); // output to XML/HTML file
    };

    //////////////////////////////////////////////////////////
}
