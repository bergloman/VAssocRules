#ifndef CLASSIFIERTEST_H	
#define CLASSIFIERTEST_H	
////////////////////////////////////////////////////////////////

#include "stdvik.h"
#include "rules.h"

namespace VikRuleClassifiers {

///////////////////////////////////////////////////////////////

    void TestClassifier(VikStd::TStr &r, VikStd::TStr &d, VikStd::TStr &o, VikStd::TOut &out);
    void AssingToClassesWithRules(VikStd::TStr &d, VikStd::TStr &r, VikStd::TStr &o, VikStd::TStr &a, VikStd::TOut &out);
    void AssingToClassesWithRulesVoting(VikStd::TStr &d, VikStd::TStr &r, VikStd::TStr &o, VikStd::TStr &a, VikStd::TOut &out);

}

#endif
