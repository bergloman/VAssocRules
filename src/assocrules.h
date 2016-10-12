#pragma once

#include "apriori.h"
#include "stdvik.h"
#include "rules.h"
#include "rulesclassifier.h"

namespace VikApriori {


	//////////////////////////////////////////////////
	// FeatureSubsetSelection types
	//////////////////////////////////////////////////

	typedef enum {
		FSS_StatCorr,   // statistical correlation
		FSS_VRelief,    // VRELIEF algorithm
		FSS_VRelief2,   // VRELIEF2 algorithm
		FSS_OddsRatio,  // OddsRatio algorithm
		FSS_None        // no feature subset selection
	} TFSSType;

	//////////////////////////////////////////////////
	// Classifier construction algorithms
	//////////////////////////////////////////////////

	typedef enum {
		CC_UseAll,       // use all rules
		CC_UseNBest,     // use N best rules
		CC_UseNBestEach, // use N best rules for each class
		CC_Nada          // Nada'a algorithm
	} TClassConType;

	//////////////////////////////////////////////////

	//////////////////////////////////////////////////
	// Main class for mining association rules
	//////////////////////////////////////////////////

	class TAssocRules {

			// private member variables ///////////////////////////
		public:
			VikStd::TCommon *m_Common; // for output

			// normal data

			VikStd::TDataTable *m_DataTable; // normal data

		private:

			int m_TargetAttribute;   // index of target attribute in m_DataTable;

			// members used for transformation from and to binary form

			VikStd::TVec<int> m_NumOfValues;     // number of possible values for each attribute
			VikStd::TVec<int> m_StartOfValues;   // index of first item that belongs to this attribute
			VikStd::TVec<TItemRef> m_Reference;  // tells which item coresponds to which attribute and its value
			int m_NumOfItems;            // number of all items;

			// binary data

			TListOfItemsets m_DB;              // binary data
			VikStd::TVec<bool> m_UseAttribute;         // attributes selected with FSS
			VikStd::TExampleWindow m_LearningExamples; // examples selected for learning
			int m_TargetItem;                  // item that is target of FSS algorithms and APRIORI learning

			// results

			VikStd::TVec<VikRuleClassifiers::TRule> m_Rules;         // vector with rules
			VikStd::TVec<VikRuleClassifiers::TRule> m_Classifier;    // vector with selected rules - classifier

			// private member functions ///////////////////////////

			// different feature subset selection algorithms

			void DoStatCorr(VikStd::TProperties &prop);
			void DoVRelief(VikStd::TProperties &prop);
			void DoVRelief2(VikStd::TProperties &prop);
			void DoOddsRatio(VikStd::TProperties &prop);

			// for testing given parameters

			bool TestFSSParams(VikStd::TProperties &prop);
			bool TestAprioriParams(VikStd::TProperties &prop);
			bool TestClassifierConstructionParams(VikStd::TProperties &prop);

			// real "hard work"

			void DoFSS(VikStd::TProperties &prop);            // feature subset selection
			void RunApriori(VikStd::TProperties &prop);       // for association rules for classification
			void RunAprioriNormal(VikStd::TProperties &prop); // for normal association rules
			void RunClassifierConstruction(VikStd::TProperties &prop);

			// utilities

			void PrepareData();                           // prepare data
			void ClearAllIntermediateData();              // clear all intermediate datastructures
			void CreateRulesFromItemsets(TApriori *apr);  // creates TRule objects from itemsets and puts them into

		public:

			int m_ItemsetsGenerated;                      // for display statistics

			TAssocRules(VikStd::TCommon *common, VikStd::TDataTable *DataTable) {
				m_Common = common;
				m_DataTable = DataTable;
				m_TargetAttribute = -1;
			};

			void SetTargetAttribute(int a) { // define index of target attribute of m_DataTable
				if ((a < 0) || (a >= m_DataTable->Attrs.Len()))
					throw VikStd::TExc("TAssocRules: SetTargetAttribute: Invalid value.");
				m_TargetAttribute = a;
			};

			int GetTargetAttribute() {
				return m_TargetAttribute;
			};

			void Run(VikStd::TProperties &prop); // for creating classifier
			void RunX(VikStd::TProperties &prop, VikStd::TDataTable *Test, FILE *fp); // for creating classifier and testing it with 10-fold CV
			VikStd::TVec<VikRuleClassifiers::TRule> *GetClassifier() {
				return &m_Classifier;
			};
			VikStd::TVec<VikRuleClassifiers::TRule> *GetRules() {
				return &m_Rules;
			};

			void RunNormal(VikStd::TProperties &prop); // normal association rules
	};

	/////////////////////////////////////////////////////////////
	// Class for testing classifier - 10-fold crossvalidation
	/////////////////////////////////////////////////////////////

	class TAssocRules10FoldCV {

			VikStd::TDataTable *m_AllData; // all data that is available
			VikStd::TCommon *m_Common;     // for output
			int m_TargetAttribute; // target attribute

			//void DoOneRun(int RunIndex);
		public:

			int m_ItemsetsGenerated;

			VikStd::TVec<VikRuleClassifiers::TClassifierResults> m_Res;
			double m_AccuracyTab[10];
			double m_OverallAccuracy;

			TAssocRules10FoldCV(VikStd::TCommon *common, VikStd::TDataTable *DataTable) {
				m_Common = common;
				m_AllData = DataTable;
				m_TargetAttribute = -1;
				m_OverallAccuracy = 0;
			};

			void SetTargetAttribute(int a) { // define index of target attribute of m_DataTable
				if ((a < 0) || (a >= m_AllData->Attrs.Len()))
					throw VikStd::TExc("TAssocRules: SetTargetAttribute: Invalid value.");
				m_TargetAttribute = a;
			};

			void Run(VikStd::TProperties &prop);
			void RunX(VikStd::TProperties &prop, FILE *fp);
	};

}
