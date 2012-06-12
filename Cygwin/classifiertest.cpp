#include "classifiertest.h"
#include "rulesclassifier.h"


namespace VikRuleClassifiers {

////////////////////////////////////////////////////////////

void ReadAttribute(TLiteralStr &lit, VikStd::TXMLParser &xml){
	// tag <att> already read, so check its parameters
	int i=xml.FindParameter("TYPE");
	if (i<0)
		throw VikStd::TExc("Illegal <att> tab - no TYPE parameter.");
	if ((xml.Values[i]!="D")&&(xml.Values[i]!="C"))
		throw VikStd::TExc(VikStd::TStr("Illegal value of TYPE parameter in <att> tab: ")+xml.Values[i]);
	if (xml.Values[i]=="D"){ lit.m_Type=VikStd::atDiscr; }
	else lit.m_Type=VikStd::atCon;
	
	xml.ReadToNextTag();
	lit.m_Attr=xml.LastText;
	if (xml.TagIsClosing)
		throw VikStd::TExc(VikStd::TStr("Unexpected closing tag: ")+xml.LastLine);
	if (xml.LastTag!="EQ/")
		if (xml.LastTag!="NEQ/")
			if (xml.LastTag!="LWR/")
				if (xml.LastTag!="GRT/")
					if (xml.LastTag!="GRTEQ/")
						if (xml.LastTag!="LWREQ/")
							throw VikStd::TExc(VikStd::TStr("Illegal tag denoting operator: ")+xml.LastTag);
	
	lit.m_Op=xml.LastTag;
	xml.ReadToNextTag();
	if (xml.TagIsClosing==false)
		throw VikStd::TExc("Closing tag expected.");
	if (xml.LastTag!="ATT")
		throw VikStd::TExc(VikStd::TStr("<ATT tag expected, found: </")+xml.LastTag+">.");
	lit.m_Val=xml.LastText;
	xml.ReadToNextTag();
};

////////////////////////////////////////////////////////////////

void ReadRule(VikStd::TVec<TRuleStr> &RuleList, VikStd::TXMLParser &xml){
	TRuleStr rule;
	xml.ReadToNextTag();
	
	if ((xml.LastTag!="BODY")||(xml.TagIsClosing!=false))
		throw VikStd::TExc("<body> expected at the beggining of rule description");
	xml.ReadToNextTag();

	while ((xml.LastTag=="ATT")&&(xml.TagIsClosing==false)){
		TLiteralStr lit;
		ReadAttribute(lit,xml);
		rule.m_Body.Add(lit);
	};
	
	if ((xml.LastTag!="BODY")||(xml.TagIsClosing==false))
		throw VikStd::TExc("</body> expected at the end of body description");

	xml.ReadToNextTag();
	ReadAttribute(rule.m_Head,xml);
	if ((xml.LastTag!="RULE")||(xml.TagIsClosing==false))
		throw VikStd::TExc(VikStd::TStr("</rule> expected at the end of rule description, found: ")+xml.LastLine);
	xml.ReadToNextTag();
	RuleList.Add(rule);
};

////////////////////////////////////////////////////////////////

bool ReadRulesFromFile(VikStd::TStr &FileName, VikStd::TVec<TRuleStr> &RuleList){
	FILE *fp=fopen(FileName.c_str(),"r");
	if (fp==NULL)
		throw VikStd::TExc(VikStd::TStr("Cannot open file ")+FileName);

	try{
		VikStd::TXMLParser xml(fp);
		xml.ReadToNextTag();
		//if (xml.ReadToCastanedaTag()==false) return false;

		RuleList.Clear();

		if ((xml.LastTag!="RULES_CLASSIFIER_FILE")||(xml.TagIsClosing!=false))
			throw VikStd::TExc("<rules_classifier_file> expected at beggining of script");

		xml.ReadToNextTag();
		while ((xml.LastTag!="RULES_CLASSIFIER_FILE")||(xml.TagIsClosing==false)){
			if ((xml.LastTag=="RULE_LIST")&&(xml.TagIsClosing==false)){
				xml.ReadToNextTag();
				while ((xml.LastTag!="RULE_LIST")||(xml.TagIsClosing==false))
					ReadRule(RuleList,xml);
			};
			xml.ReadToNextTag();
		};
	}
	catch(...){ fclose(fp); throw; };
	return true;
};

////////////////////////////////////////////////////////////////

bool CompareAndCreateRules(VikStd::TDataTable &tab, VikStd::TVec<TRuleStr> &RuleList, VikStd::TVec<TRule> &RL){
	RL.Clear();
	for (int i=0; i<RuleList.Len(); i++){
		if (RuleList[i].Compatible(tab)==false) return false;
		RL.Add(RuleList[i].CreateTRule(tab));
	};
	return true;
};

////////////////////////////////////////////////////////////////

bool CompareAndCreateRules2(VikStd::TDataTable &tab, VikStd::TVec<TRuleStr> &RuleList, VikStd::TVec<TRule> &RL, VikStd::TVec<VikStd::TStr> &Targets){
	RL.Clear();
	for (int i=0; i<RuleList.Len(); i++){
		if (RuleList[i].Compatible2(tab)==false) return false;
		RL.Add(RuleList[i].CreateTRule2(tab,Targets));
	};
	return true;
};

////////////////////////////////////////////////////////////////

void TestClassifier(VikStd::TStr &r, VikStd::TStr &d, VikStd::TStr &o, VikStd::TOut &out){
	VikStd::TVec<TRuleStr> RuleList;

	out.Put("Reading rules...\n");
	ReadRulesFromFile(r, RuleList);

	out.Put("Reading data...\n");
	VikStd::TDataTable tab;
	tab.LoadFromFileX(d);
	VikStd::TVec<TRule> RL;

	if (CompareAndCreateRules(tab,RuleList,RL)!=false){

		// there must be at least one rule
		if (RL.Len()==0) throw VikStd::TExc("No rules found in specified file.");

		// check if all rules reffer to the same target attribute
		for (int i=1; i<RL.Len(); i++)
			if (RL[0].m_Head.m_Attr!=RL[i].m_Head.m_Attr)
				throw VikStd::TExc("Illegal classifier: not all rules reffer to the same target attribute.");

		// OK, go ahead with testing
		TRulesClassifier rc;
		rc.m_Input=&RL;
		rc.m_RulesCount=5;
		rc.m_ClassAttr=RL[0].m_Head.m_Attr;

		rc.m_DataTable=&tab;
		rc.TestClassifier();

		
		FILE *fp=fopen(o.c_str(),"w");
		try{
			//fprintf(fp,"<html><head><title>Classifier test results</title></head>\n\n");
			//fprintf(fp,"<script language=\"castaneda\"><!\n\n");

			// write script
			fprintf(fp,"<classifier_results>\n");
			fprintf(fp,"\t<date>%s</date>\n",rc.m_Results.m_Time.GetDateAsStr("%D.%M.%Y").c_str());
			fprintf(fp,"\t<time>%s</time>\n",rc.m_Results.m_Time.GetTimeAsStr("%H:%M:%S").c_str());
			fprintf(fp,"\t<cl_type>RULES_CLASSIFIER</cl_type>\n");
			fprintf(fp,"\t<cl_file>%s</cl_file>\n",r.c_str());
			fprintf(fp,"\t<data_file>%s</data_file>\n\n",d.c_str());
			fprintf(fp,"\t<acc>%f</acc>\n",rc.m_Results.m_Accuracy);
			fprintf(fp,"\t<all_ex>%d</all_ex>\n",rc.m_Results.m_Correct+rc.m_Results.m_Wrong);
			fprintf(fp,"\t<all_corr>%d</all_corr>\n",rc.m_Results.m_Correct);
			fprintf(fp,"\t<all_wrong>%d</all_wrong>\n\n",rc.m_Results.m_Wrong);
			fprintf(fp,"\t<target_attr>%s</target_attr>\n",RuleList[0].m_Head.m_Attr.c_str());

			int i,j;
			fprintf(fp,"\t<target_vals>\n");
			for (i=0; i<tab.Attrs[rc.m_ClassAttr].ValNames.Len(); i++){
				//if (i>0) fprintf(fp,"<d>");
				fprintf(fp,"\t\t<target_val>%s</target_val>\n",tab.Attrs[rc.m_ClassAttr].ValNames[i].c_str());
			};
			fprintf(fp,"</target_vals>\n\n");

			fprintf(fp,"\t<matrix>\n");
			for (j=0; j<rc.m_Results.m_ConfusionMatrix.GetYDim(); j++){
				fprintf(fp,"\t\t<r>\n");
				for (i=0; i<rc.m_Results.m_ConfusionMatrix.GetXDim(); i++){				
					//if (i>0) fprintf(fp,"<d>");
					fprintf(fp,"\t\t\t<d>%d</d>\n",rc.m_Results.m_ConfusionMatrix.At(i,j));
				};
				fprintf(fp,"</r>\n");
			};
			fprintf(fp,"\t</matrix>\n");
			fprintf(fp,"</classifier_results>\n");

			/*
			fprintf(fp,"></script>\n");
			fprintf(fp,"<body>\n<font face=\"arial\"><table><tr><td bgcolor=blue width=700><Font color=white size=+1><b>&nbsp;&nbsp;Results of the classifier</b></font></td></tr></table><br>");

			// write ordinary HTML data

			fprintf(fp,"<b>Date</b>: %s<br>\n",rc.m_Results.m_Time.GetDateAsStr("%D.%M.%Y").c_str());
			fprintf(fp,"<b>Time</b>: %s<br>\n",rc.m_Results.m_Time.GetTimeAsStr("%H:%M:%S").c_str());
			fprintf(fp,"<b>Classifier type</b>: RULES_CLASSIFIER<br>\n");
			fprintf(fp,"<b>Classifier file</b>: %s<br>\n",r.c_str());
			fprintf(fp,"<b>Data_file</b>: %s<br>\n<br>\n",d.c_str());

			fprintf(fp,"<b>Accuracy</b>: %f<br>\n",rc.m_Results.m_Accuracy);
			fprintf(fp,"<b>All examples</b>: %d<br>\n",rc.m_Results.m_Correct+rc.m_Results.m_Wrong);
			fprintf(fp,"<b>All correct</b>: %d<br>\n",rc.m_Results.m_Correct);
			fprintf(fp,"<b>All wrong</b>: %d<br>\n\n",rc.m_Results.m_Wrong);
			fprintf(fp,"<b>Target attribute</b>: %s<br>\n<br>\n",RuleList[0].m_Head.m_Attr.c_str());

			fprintf(fp,"<b>Target_values</b>: ");
			for (i=0; i<tab.Attrs[rc.m_ClassAttr].ValNames.Len(); i++){
				if (i>0) fprintf(fp,", ");
				fprintf(fp,"%s",tab.Attrs[rc.m_ClassAttr].ValNames[i].c_str());
			};
			fprintf(fp,"<br>\n<br>\n");

			fprintf(fp,"<b>Confusion matrix</b>:\n<br>\n");
			fprintf(fp,"<table cellpadding=2 border=2>\n");
			
			fprintf(fp,"<tr><td bgcolor=blue><font color=white><b>&nbsp;</b></font></td>");
			for (i=0; i<rc.m_Results.m_ConfusionMatrix.GetYDim(); i++)
				fprintf(fp,"<td bgcolor=blue><font color=white><b>%s</b></font></td>",tab.Attrs[rc.m_ClassAttr].ValNames[i].c_str());
			fprintf(fp,"<td bgcolor=blue><font color=white><b>Unclassified</b></font></td> </tr>\n");

			for (j=0; j<rc.m_Results.m_ConfusionMatrix.GetYDim(); j++){
				fprintf(fp,"<tr><td bgcolor=blue><font color=white><b>%s</b></font></td>",tab.Attrs[rc.m_ClassAttr].ValNames[j].c_str());
				for (i=0; i<rc.m_Results.m_ConfusionMatrix.GetXDim(); i++){				
					if (i==j){
						fprintf(fp,"<td bgcolor=\"#dddddd\">%d</td>",rc.m_Results.m_ConfusionMatrix.At(i,j));
					}
					else{
						fprintf(fp,"<td>%d</td>",rc.m_Results.m_ConfusionMatrix.At(i,j));
					};
				};
				fprintf(fp,"</tr>\n");
			};

			fprintf(fp,"</table></body></html>");
			*/
		}
		catch(...){ fclose(fp); throw; };
		fclose(fp);

	}
	else{
		throw VikStd::TExc("Rules and data are not compatible");
	};
};

////////////////////////////////////////////////////////////

void AssingToClassesWithRules(VikStd::TStr &d, VikStd::TStr &r, VikStd::TStr &o, VikStd::TStr &a, VikStd::TOut &out){
	VikStd::TVec<TRuleStr> RuleList;

	out.Put("Reading rules...\n");
	ReadRulesFromFile(r, RuleList);

	out.Put("Reading data...\n");
	VikStd::TDataTable tab;
	tab.LoadFromFileX(d);
	if (tab.FindAttrWithName(a)>=0)
		throw VikStd::TExc(VikStd::TStr("Attribute with name '")+a+"' already exists.");
	
	VikStd::TVec<VikStd::TStr> Targets;
	for (int i=0; i<RuleList.Len(); i++){
		bool found=false;
		for (int j=0; j<Targets.Len(); j++){
			if (Targets[j]==RuleList[i].m_Head.m_Val)
				found=true;
		};
		if (found==false) Targets.Add(RuleList[i].m_Head.m_Val);
	};

	VikStd::TVec<TRule> RL;

	if (CompareAndCreateRules2(tab,RuleList,RL,Targets)!=false){

		// there must be at least one rule
		if (RL.Len()==0) throw VikStd::TExc("No rules found in specified file.");

		// OK, go ahead with testing
		TRulesClassifier rc;
		rc.m_Input=&RL;
		rc.m_DataTable=&tab;
		out.Put("Working...\n");
		rc.CreateNewAttribute(a,Targets);
		
		out.Put("Writing results...\n");
		tab.SaveToFileTab(o);

	}
	else{
		throw VikStd::TExc("Rules and data are not compatible");
	};
};

/////////////////////////////////////////////////////////////

void AssingToClassesWithRulesVoting(VikStd::TStr &d, VikStd::TStr &r, VikStd::TStr &o, VikStd::TStr &a, VikStd::TOut &out){
	VikStd::TVec<TRuleStr> RuleList;

	out.Put("Reading rules...\n");
	ReadRulesFromFile(r, RuleList);

	out.Put("Reading data...\n");
	VikStd::TDataTable tab;
	tab.LoadFromFileX(d);
	if (tab.FindAttrWithName(a)>=0)
		throw VikStd::TExc(VikStd::TStr("Attribute with name '")+a+"' already exists.");
	
	VikStd::TVec<VikStd::TStr> Targets;
	for (int i=0; i<RuleList.Len(); i++){
		bool found=false;
		for (int j=0; j<Targets.Len(); j++){
			if (Targets[j]==RuleList[i].m_Head.m_Val)
				found=true;
		};
		if (found==false) Targets.Add(RuleList[i].m_Head.m_Val);
	};

	VikStd::TVec<TRule> RL;

	if (CompareAndCreateRules2(tab,RuleList,RL,Targets)!=false){

		// there must be at least one rule
		if (RL.Len()==0) throw VikStd::TExc("No rules found in specified file.");

		// OK, go ahead with testing
		TRulesClassifier rc;
		rc.m_Input=&RL;
		rc.m_DataTable=&tab;
		out.Put("Working...\n");
		rc.CreateNewAttributeVoting(a,Targets);
		
		out.Put("Writing results...\n");
		tab.SaveToFileTab(o);

	}
	else{
		throw VikStd::TExc("Rules and data are not compatible");
	};
};
}
