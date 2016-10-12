/////////////////////////////////////////////////////////////////////
//
// Module: implementation of FOIL algorithm.
// Author: Viktor Jovanoski
// Date: 20.07.1998, Ljubljana
/////////////////////////////////////////////////////////////////////

#include "foil.h"

namespace VikRuleClassifiers {

	//#define FOIL_DEBUG
	/////////////////////////////////////////////////////

	int TFoil::GetMostFreqClass(VikStd::TDataTable *tab, VikStd::TExampleWindow& ew){
		VikStd::TAttribute *at=&(tab->Attrs[m_TargetAttr]);
	  VikStd::TVec<int> count( at->ValNames.Len() );
	   
		int i;
		for (i=0; i<count.Len(); i++) count[i]=0;

		for (i=0; i<at->iVec.Len(); i++)
			if (ew.IsIn(i)) count[at->iVec[i]]++; 

		int max=0, m=-1;
		for (i=0; i<count.Len(); i++)
			if (count[i]>m){ m=count[i]; max=i; };

		return max;
	};

	//////////////////////////////////////////////////////

	void TFoil::Grow(VikStd::TDataTable *tab){

		if ((m_TargetAttr>=tab->Attrs.Len())||(m_TargetAttr<0))
			throw VikStd::TExc("TFoil: Grow(): invalid value of member m_TargetAttr.");
		
		if (tab->Attrs[m_TargetAttr].AttrType!=VikStd::atDiscr)
			throw VikStd::TExc("TFoil: Grow(): Target attribute not discrete.");

		if (m_MinSup<1)
			throw VikStd::TExc("TFoil: Grow(): MinSupport<1.");

		m_TimeTaken.Start();

		VikStd::TExampleWindow ew(tab->Attrs[0].Len()); // examples not covered by current RuleSet
		VikStd::TExampleWindow temp_ew(ew);             // examples from ew, covered by current rule
		VikStd::TExampleWindow new_ew(ew);              // examples from temp_ew, covered by current attribute
	  
		ew.PutToAll(true);
	  
		TRule rule;
	  bool OK=true, OK1;
	  VikStd::TExampleWindow unused(tab->Attrs.Len());

		int RuleCount=0, AttrCount;

	  while (OK){ // while any rule can be generated

			// create default rule
			rule.Clr();
			rule.m_Head.m_Type = VikStd::atDiscr;
			rule.m_Head.m_Attr = m_TargetAttr;
			rule.m_Head.m_Negation = false;		
			rule.m_Head.m_iVal = GetMostFreqClass(tab,ew);

			AttrCount=0;

			rule.RemoveUncoveredExamples(tab,&ew);

			unused.PutToAll(true);          // set of unused attributes
			unused.Put(m_TargetAttr,false); // don't use target attribute in body of a rule
			new_ew=ew;
			temp_ew=ew; // currently not covered examples

			OK1=(rule.m_Neg>0);

			while (OK1){ // Growth of a new rule
				double best_e, temp_e;
				TLiteral best, temp;
				bool first=true;
				int best_tc=temp_ew.TrueCount();
				best_e = Entropy(&(tab->Attrs[m_TargetAttr]),&temp_ew);

				for (int i=0; i<tab->Attrs.Len(); i++){ // For all unused attributes
					if (unused.IsIn(i)){
						
						if (tab->Attrs[i].AttrType==VikStd::atDiscr){ // discrete attribute

							for (int j=0; j<tab->Attrs[i].ValNames.Len(); j++){ // for each value of current attribute
								
								temp.m_Type = VikStd::atDiscr;
								temp.m_Attr = i;
								temp.m_iVal = j;
								temp.m_Negation = false;
								
								new_ew=temp_ew;

								rule.m_Body.Add(temp);
								rule.RemoveUncoveredExamples(tab,&new_ew);

								if (new_ew.TrueCount()>=m_MinSup){
									temp_e = Entropy(&(tab->Attrs[m_TargetAttr]),&new_ew);							

									if (temp_e<best_e){
										best_e=temp_e; 
										best=temp; 
										best_tc=new_ew.TrueCount();
										first = false;
									}
									else if (temp_e==best_e){
										int ttc=new_ew.TrueCount();
										if (best_tc<ttc){
											best_e=temp_e; 
											best=temp; 
											best_tc=ttc;
											first = false;
										};
									};
								};
								
								rule.m_Body.Delete(rule.m_Body.Len()-1);

								if (m_UseNegation){
									temp.m_Negation = true;
									rule.m_Body.Add(temp);
									new_ew=temp_ew;
									rule.RemoveUncoveredExamples(tab,&new_ew);
									
									if (new_ew.TrueCount()>=m_MinSup){
										temp_e=Entropy(&(tab->Attrs[m_TargetAttr]),&new_ew);
								
										if (temp_e<best_e){
											best_e=temp_e; 
											best=temp;
											best_tc=new_ew.TrueCount();
											first = false;
										}
										else if (temp_e==best_e){
											int ttc=new_ew.TrueCount();
											if (best_tc<ttc){
												best_e=temp_e; 
												best=temp; 
												best_tc=ttc;
												first = false; 
											};
										};
									};
									rule.m_Body.Delete(rule.m_Body.Len()-1);
								};// m_UseNegation==true
							}; // for each value of current attribute
						}
						else{ // atCon

							//printf("    testing attribute %s",tab->Attrs[i].AttrName.c_str());

							double t=DynamicDiscretisation( &(tab->Attrs[i]), &(tab->Attrs[m_TargetAttr]),&ew); // find best cut value
							temp.m_Type = VikStd::atCon;
							temp.m_Attr = i;
							temp.m_dVal = t;
							temp.m_Negation = false;

							rule.m_Body.Add(temp);						
							new_ew = temp_ew;

							rule.RemoveUncoveredExamples(tab,&new_ew);

							temp_e=Entropy(&(tab->Attrs[m_TargetAttr]),&new_ew);

							if (new_ew.TrueCount()<m_MinSup) temp_e=1000;
							if (temp_e<best_e){
								best_e=temp_e; 
								best=temp; 
								best_tc=new_ew.TrueCount();
								first = false;
							}
							else if (temp_e==best_e){
								int ttc=new_ew.TrueCount();
								if (best_tc<ttc){
									best_e=temp_e; 
									best=temp; 
									best_tc=ttc;
									first = false;
								};
							};
							rule.m_Body.Delete(rule.m_Body.Len()-1);

							temp.m_Negation = true;
							rule.m_Body.Add(temp);
							new_ew = temp_ew;
							rule.RemoveUncoveredExamples(tab,&new_ew);

							temp_e=Entropy(&(tab->Attrs[m_TargetAttr]),&new_ew);

							if (new_ew.TrueCount()<m_MinSup) temp_e=1000;
							if (temp_e<best_e){
								best_e=temp_e; 
								best=temp; 
								best_tc=new_ew.TrueCount();
								first = false;
							}
							else if (temp_e==best_e){
								int ttc=new_ew.TrueCount();
								if (best_tc<ttc){
									best_e=temp_e; 
									best=temp; 
									best_tc=ttc;
									first = false;
								};
							};
							rule.m_Body.Delete(rule.m_Body.Len()-1);
						}; // else
					}; // if unused
				}; // for

				// currently best attribute found, update rule
				if (first){ 
					OK1=false;				
					rule.m_Head.m_iVal = GetMostFreqClass(tab,ew);
					rule.RemoveUncoveredExamples(tab,&temp_ew);
				}
				else{
					rule.m_Body.Add(best);

					if (best.m_Type==VikStd::atDiscr)	unused.Put(best.m_Attr,false);

					new_ew = temp_ew;
					rule.RemoveUncoveredExamples(tab,&new_ew);
					rule.m_Head.m_iVal = GetMostFreqClass(tab,new_ew);
					rule.RemoveUncoveredExamples(tab,&temp_ew);
					
					if (rule.m_Neg==0) OK1=false;        // stopping criterion
					if (unused.TrueCount()<2) OK1=false; // stopping criterion
					AttrCount++;
					//if (AttrCount>0) OK1=false;
				};
			}; // while OK1 - growth of a new rule

			PruneRule(tab,rule,ew,temp_ew);
			
			if (rule.m_Pos>0){
				m_RuleSet.Add(rule);
				//printf("Rule found:\n%s\n\n",rule.GetRuleAsStr(tab,true,false).c_str());

				RuleCount++;
				//if (RuleCount>0) return;
				ew.Remove(temp_ew);
			}
			else { OK=false; }; // stopping criterion

			if (ew.TrueCount()==0) OK=false; // stopping criterion
		}; // while OK - any good rule can be generated	
		m_TimeTaken.Stop();
		m_CreationTime.Now();	
	};

	//////////////////////////////////////////////////////////////////////////
	 
	void TFoil::PruneRule( 
			VikStd::TDataTable *tab,
			TRule &r,
			VikStd::TExampleWindow &ew,
			VikStd::TExampleWindow &new_ew){

		VikStd::TSubsets ss( r.m_Body.Len() );
		TRule temp, best;

		double temp_e, best_e=Entropy(&(tab->Attrs[m_TargetAttr]),&new_ew);

		best.Copy(r);
		best.m_Head.m_iVal = GetMostFreqClass(tab,new_ew);
		ss.Init();

		while(ss.Next()){ // for each possible subset of currently selected attributes
			temp.Clr();
			for (int i=0; i<r.m_Body.Len(); i++){
				if (ss.IsSet(i))
					temp.m_Body.Add(r.m_Body[i]);
			};

			new_ew=ew;
			temp.m_Head = r.m_Head;		
			temp.RemoveUncoveredExamples(tab,&new_ew);
			temp.m_Head.m_iVal = GetMostFreqClass(tab,new_ew);
			temp_e=Entropy(&(tab->Attrs[m_TargetAttr]),&new_ew);

			if (temp_e<best_e){
				best_e=temp_e;
				best.Copy(temp);
				best.m_Head.m_iVal=GetMostFreqClass(tab,new_ew);
			};
		};

		r.Copy(best);
		r.GetCoverage(tab,&new_ew);
		r.RemoveUncoveredExamples(tab,&new_ew);
	};

	////////////////////////////////////////////////////////////////

	void TFoil::Output(VikStd::TOut *out, VikStd::TStr &InputFile, VikStd::TDataTable *tab){
		if (out==NULL)
			throw VikStd::TExc("Cannot output FOIL - out==NULL");

		if (tab==NULL)
			throw VikStd::TExc("Cannot output FOIL - tab==NULL");

		out->Put("[Classifier][Rules][FOIL]\n\n[");
		out->Put(m_CreationTime.GetDateAsStr("%d.%m.%y ").CStr());
		out->Put(m_TimeTaken.m_Diff.GetTimeAsStr("%h:%m:%s]\n[").CStr());
		out->Put(InputFile);
		out->Put("]\n[");
		for (int i=0; i<m_RuleSet.Len(); i++){
			out->Put(m_RuleSet[i].GetRuleAsStr(tab).CStr());
			out->Put("\n\n");
		};
		out->Put("]\n");
	};

	///////////////////////////////////////////////////////////////

	void TFoil::OutputXML(VikStd::TStr &OutputFile, VikStd::TDataTable *tab){
		int i;
		FILE *fp=fopen(OutputFile.c_str(),"w");
		if (fp==NULL) throw VikStd::TExc("Cannot open file to output rules.");

		//fprintf(fp,"<html>\n<head>\n\t<title>FOIL - results</title>\n</head>\n\n");
		//fprintf(fp,"<script language=\"CASTANEDA\">\n<!\n\n");

		fprintf(fp,"<rules_classifier_file>\n\t<algo>FOIL</algo>\n\n<rule_list>\n");
		for (i=0; i<m_RuleSet.Len(); i++){
			fprintf(fp,"%s\n",m_RuleSet[i].GetRuleAsStrXML(tab,false,false).c_str());
		};
		fprintf(fp,"</rule_list>\n\n</rules_classifier_file>");

		//fprintf(fp,">\n\n</script>\n\n<body>\n");

		//for (i=0; i<m_RuleSet.Len(); i++){
		//	fprintf(fp,"%s\n",m_RuleSet[i].GetRuleAsStrHTML(tab).c_str());
		//};

		//fprintf(fp,"</body>\n</html>");
		fclose(fp);
	};

}
