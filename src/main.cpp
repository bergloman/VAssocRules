// TestAR.cpp : Defines the entry point for the console application.
//

#include "str.h"
#include "ds.h"
#include "stdvik.h"
#include "assocrules.h"
#include "foil.h"
#include "clusterfuncs.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////


void NormalRunAssocRules(VikApriori::TAssocRules &ar, VikStd::TProperties &prop) {

    VikStd::TVar var;

    VikStd::TVTimer timer;
    timer.Start();
    ar.Run(prop);
    timer.Stop();

    char bf[1000];
    sprintf(bf, "Time taken: %s\n", timer.m_Diff.GetTimeAsStr(VikStd::TStr("%h:%m:%s")).c_str());
    ar.m_Common->Notify(bf);
    int i;
    for (i = 0; i < ar.GetClassifier()->Len(); i++)
        ar.m_Common->NotifyP(ar.GetClassifier()->At(i).GetRuleAsStr(ar.m_DataTable).c_str());

    if (prop.Find("OutputFile", var) != false) { // output rules to specified file
        FILE *fp = fopen(var.m_Str.c_str(), "w");
        if (fp == NULL)
            throw VikStd::TExc("Cannot open file to output rules.");

        fprintf(fp, "<rules_classifier_file>\n\t<algo>ASSOC_RULES_CLASSIFIER</algo>\n\n<rule_list>\n");
        for (i = 0; i < ar.GetClassifier()->Len(); i++) {
            fprintf(
                fp,
                "%s\n",
                ar.GetClassifier()->At(i).GetRuleAsStrXML(
                    ar.m_DataTable,
                    false,
                    false).c_str());
        };
        fprintf(fp, "</rule_list>\n\n</rules_classifier_file>");
        fclose(fp);
    };
};

int main(int argc, char* argv[]) {
    // first prepare some utility objects
    VikStd::TOutFile outfile("c:\\temp\\assoc_rules_log.txt");
    VikStd::TOutStd outstd;
    VikStd::TOutDouble out2(&outfile, &outstd);
    VikStd::TCommon common(&out2);
    common.NotifyP("Reading data...");

    // load data from input file
    VikStd::TDataTable tab;
    VikStd::TStr temp_s("c:\\temp\\data.txt");
    tab.LoadFromFileX(temp_s);
    VikApriori::TAssocRules ar(&common, &tab);

    // determine target attribute
    VikStd::TStr TargetName = "A86";
    int target = tab.FindAttrWithName(TargetName);
    if (target < 0)
        throw VikStd::TExc(VikStd::TStr("Incompatible data file and parameter Target. No such attribute: ") + TargetName);
    if (tab.Attrs[target].AttrType != VikStd::atDiscr)
        throw VikStd::TExc("Incompatible data file and parameter Target. Attribute not discrete.");
    ar.SetTargetAttribute(target);

    // now setup properties for this algorithm
    VikStd::TProperties props;
    VikStd::TProp prop;

    prop.m_c1 = "OutputFile";
    prop.m_c2.m_Str = "c:\\temp\\res.txt";
    prop.m_c2.m_Type = VikStd::VTStr;
    props.Add(prop);


    prop.m_c1 = "MaxRuleLen";
    prop.m_c2.m_Str = 2;
    prop.m_c2.m_Type = VikStd::VTInt;
    props.Add(prop);

    prop.m_c1 = "MinSup";
    prop.m_c2.m_Double = 0.05;
    prop.m_c2.m_Type = VikStd::VTDouble;
    props.Add(prop);

    prop.m_c1 = "MinConf";
    prop.m_c2.m_Double = 0.7;
    prop.m_c2.m_Type = VikStd::VTDouble;
    props.Add(prop);

    // detect feature subset selection type
    prop.m_c1 = "FSSType";
    prop.m_c2.m_Type = VikStd::VTInt;
    //    if ((PercentOfSelectedAttributes >= 1) || (PercentOfSelectedAttributes <=0))
    //      prop.m_c2.m_Int = FSS_None;
    //    else
    prop.m_c2.m_Int = VikApriori::FSS_OddsRatio;
    props.Add(prop);

    prop.m_c1 = "PercentOfSelectedAttrs";
    prop.m_c2.m_Type = VikStd::VTDouble;
    prop.m_c2.m_Double = 0.2;
    props.Add(prop);

    // detect classifier construction algorithm
    // 0 - use all rules
    // 1 - use n rules
    // 2 - use n rules for each class
    prop.m_c1 = "ClassConstrAlgo";
    prop.m_c2.m_Type = VikStd::VTInt;
    prop.m_c2.m_Int = VikApriori::CC_UseAll;
    props.Add(prop);

    prop.m_c1 = "N";
    prop.m_c2.m_Type = VikStd::VTInt;
    prop.m_c2.m_Int = 1;
    props.Add(prop);

    prop.m_c1 = "DefaultRule";
    prop.m_c2.m_Type = VikStd::VTBool;
    prop.m_c2.m_Bool = true;
    props.Add(prop);

    //NormalRunAssocRules(ar, props);

    /////////////
    /*
    out2.Put("Running FOIL...\n");
    VikRuleClassifiers::TFoil foil_obj;
    foil_obj.m_TargetAttr = target;
    foil_obj.Grow(&tab);
    foil_obj.Output(&out2, temp_s, &tab);
    */

    VikStd::TStr out_file_name("c:\\temp\\res.txt");
    VikClustering::DoClustering(temp_s, out_file_name, 10, 5, out2);
    return 0;
}
