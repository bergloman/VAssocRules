#pragma once

///////////////////////////////////

#include "rules.h"
#include "stdvik.h"

namespace VikRuleClassifiers {

	////////////////////////////////////////////////////////
	// Class that stores results of classification
	////////////////////////////////////////////////////////

	class TClassifierResults {
		public:
			VikStd::TVVec<int> m_ConfusionMatrix;   // for multiple example test
			double m_Accuracy;                      // for multiple example test
			int m_Correct, m_Wrong;             // for multiple example test
			int m_Unclassified;
			VikStd::TVTime m_Time;

			TClassifierResults() {
				Clr();
			};
			TClassifierResults(const TClassifierResults &r) {
				m_Accuracy = r.m_Accuracy;
				m_Correct = r.m_Correct;
				m_Time = r.m_Time;
				m_Unclassified = r.m_Unclassified;
				m_Wrong = r.m_Wrong;
				m_ConfusionMatrix = r.m_ConfusionMatrix;
			};
			void Clr() {
				m_ConfusionMatrix.Gen(0, 0);
				m_Accuracy = m_Correct = m_Wrong = m_Unclassified = 0;
			};
			void GetTime() {
				m_Time.Now();
			};
	};

	////////////////////////////////////////////////////////
	// Class that creates classifier from list of rules
	////////////////////////////////////////////////////////

	class TRulesClassifier {

			VikStd::TExampleWindow m_ExWindow;
			void CreateDefaultClass();
			void CheckData();

		public:

			bool m_DefaultClass; // should algorithm produce the rule for default class
			int m_RulesCount;    // number of rules to be selected - parameter N
			int m_ClassAttr;

			TClassifierResults m_Results; // for multiple example test

			VikStd::TDataTable *m_DataTable;           // data
			VikStd::TVec<TRule> *m_Input;              // list of all rules
			VikStd::TVec<TRule> m_ResultingClassifier; // result - selected rules

			TRulesClassifier() {
				m_DefaultClass = true;
				m_RulesCount = -1;
				m_DataTable = NULL;
				m_Input = NULL;
				m_ClassAttr = -1;
			};

			void UseAllRules();
			void UseNRulesForAllClasses();
			void UseNRulesForEachClass();
			void NadaAlgo();

			void TestClassifier();
			void CreateNewAttribute(VikStd::TStr &a, VikStd::TVec<VikStd::TStr> &Targets); // creates new attribute, based on rule prediction
			void CreateNewAttributeVoting(VikStd::TStr &a, VikStd::TVec<VikStd::TStr> &Targets); // creates new attribute, based on rule prediction with voting

			void Clear() {
				m_DefaultClass = true;
				m_RulesCount = -1;
				m_DataTable = NULL;
				m_Input = NULL;
				m_ResultingClassifier.Clear();
				m_ClassAttr = -1;
			};
	};

	/////////////////
}
