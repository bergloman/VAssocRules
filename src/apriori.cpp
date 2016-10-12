#include "apriori.h"

namespace VikApriori {

	///////////////////////////////////////////////////////////////////
	// Author: Viktor Jovanoski
	// Date  : 12.04.1998, Ljubljana
	// Module: Implementaton of class for execution of algorithm APRIORI
	// Reprogrammed: January 2000, Ljubjana
	// Revisions:
	// May 2000 - VJI - added normal apriori execution, for normal rules
	///////////////////////////////////////////////////////////////////

	inline bool myXOR(bool a, bool b)
	{
		if (a == false) return b;
		return (b == false);
	};

	/////////////////////////////////////////////////////////////
	// Two itemsets are equivalent if they are of same length
	// and they differ only in last item. (Items are ordered)
	/////////////////////////////////////////////////////////////

	bool TAprioriOneStep::equivalent(TItemset& a, TItemset& b)
	{

		int btc = b.TrueCount();
		int f1;
		TItemset temp;
		temp.Init();
		temp = a;
		temp.Union(b);

		if (temp.TrueCount() != btc + 1) return false;
		if (btc < 3) return true;
		int h = 0;
		for (int i = a.ItemsCount() - 1; i >= 0; i--) {
			if (a.IsSetItem(i) && b.IsSetItem(i))
				return false;
			if (myXOR(a.IsSetItem(i), b.IsSetItem(i))) {
				h++;
				if (h == 1) f1 = i;
				if (h == 2) {
					int a1 = m_ItemRef->At(i).m_c1;
					int a2 = m_ItemRef->At(f1).m_c1;
					if (m_ItemRef->At(i).m_c1 == m_ItemRef->At(f1).m_c1) {
						return false;
					};
					return true;
				};
			};
		};
		return false;
	};

	/////////////////////////////////////////////////////////////
	// Equivalent itemsets of given itemset are given in a list
	/////////////////////////////////////////////////////////////
	/*
	void GetEquivalent(TListOfItemsets& Ck, TListOfItemsets& eqclass, int items_number){

	    int i, in=1;
	    while (equivalent(Ck[0],Ck[in])) in++;

	    eqclass.Clear();
	    for (i=0; i<in; i++)
	        eqclass.Add(Ck[i]);

	    for (i=0; i<in; i++){
	        SwapItemsets(Ck[i],Ck.Last());
	        Ck.DeleteLast();
	    };
	    Ck.Sort();
	};

	*/
	////////////////////////////////////////////////////////////
	// Remove eventual duplicates
	/////////////////////////////////////////////////////////////

	void RemoveDuplicates(TListOfItemsets &l)
	{

		if (l.Len() < 2) return;

		int i;
		TListOfItemsets l1;
		for (i = 0 ; i < l.Len(); i++)
			l1.Add(l[i]);
		l1.Sort();
		l.Clear();
		l.Add(l1[0]);
		for (i = 1 ; i < l1.Len(); i++) {
			if ((l1[i] == l1[i - 1]) == false)
				l.Add(l1[i]);
		};
	};

	////////////////////////////////////////////////////////////
	// Checks if given (k+1)-itemset is consistent with given list
	// of k-itemsets. That means that every k-subset of given
	// itemset must exist in given list of itemsets.

	bool TAprioriOneStep::consistent2(TItemset& a, TListOfItemsets& l)
	{
		for (int i = 0; i < a.ItemsCount(); i++) {
			if (a.IsSetItem(i)) {
				a.ResetItem(i);
				if (l.Find(a) == -1) {
					a.SetItem(i);
					return false;
				};
				a.SetItem(i);
			};
		};
		return true;
	};

	////////////////////////////////////////////////////////////
	// Function generates new level of itemsets
	////////////////////////////////////////////////////////////

	void TAprioriOneStep::NewLevel2(TListOfItemsets& Ck, TListOfItemsets& Ck1)
	{

		TItemset curr;
		curr.Init();

		Ck1.Clear();
		Ck.Sort();
		Ck.CreateIndex();
		int j, i = Ck.Len() - 1;

		while(i > 0) {
			j = i - 1;
			while ((j >= 0) && ( equivalent(Ck[i], Ck[j]) )) {
				curr = Ck[i];
				curr.Union(Ck[j]);
				if (consistent2(curr, Ck))
					Ck1.Add(curr);
				j--;
			};
			i--;
		};
		Ck1.Sort();
		RemoveDuplicates(Ck1);
	};

	///////////////////////////////////////////////////////////////////
	// Initial checks - check if given values are in valid range.
	// If not, throw an exception.
	///////////////////////////////////////////////////////////////////

	void TApriori::InitialChecks()
	{
		if (m_Data == NULL) 
			throw VikStd::TExc("TApriori: Perform: m_Data==NULL.");
		if (m_Data->Len() < 1) 
			throw VikStd::TExc("TApriori: Perform: m_Data->m_v.Len()<1 (no data specified).");
		if (m_Conf > 1) 
			throw VikStd::TExc("TApriori: Perform: m_Conf>1.");
		if (m_Conf <= 0) 
			throw VikStd::TExc("TApriori: Perform: m_Conf<=0.");
		if (m_MinSup < 1) m_MinSup = 1;

		if (m_RunForTarget != false) {
			if (m_UseItem == NULL) 
				throw VikStd::TExc("TApriori: Perform: m_UseItem=NULL");
			if (m_TargetItem < 0) 
				throw VikStd::TExc("TApriori: Perform: m_TargetItem<0.");
			if (m_TargetItem >= m_Data->At(0).ItemsCount()) 
				throw VikStd::TExc("TApriori: Perform: m_TargetItem>=ItemsCount.");
		};
	};

	///////////////////////////////////////////////////////////////////
	// Initialise values of member variables.
	///////////////////////////////////////////////////////////////////

	void TApriori::Initialisation()
	{
		m_ResultsBody.Clear();
		m_ResultsHead.Clear();
		m_ResultsConf.Clear();
		m_l.Clear();
		m_l1.Clear();
		m_l2.Clear();
		m_ItemsCount = m_Data->At(0).ItemsCount();
	};

	///////////////////////////////////////////////////////////////////

	void TApriori::PerformWithTarget()
	{

		m_RunForTarget = true;
		InitialChecks();
		Initialisation();

		one_step.m_ItemRef = m_ItemRef;

		int acount = 0;

		m_pCommon->Notify("Number of items: ");
		m_pCommon->NotifyP(VikStd::TStr(m_ItemsCount));

		// First create singletons

		int i;
		TItemset temp;
		temp.Init();
		temp.Clear();
		for (i = 0; i < m_ItemsCount; i++) {
			if (m_UseItem->At(i) != false) {
				temp.SetItem(i);
				m_l1.Add();
				m_l1.Last().Copy(temp);
				temp.ResetItem(i);
			};
		};

		acount += m_ItemsCount;

		GetFrequencies(&m_l1);
		MarkUnfrequent(m_l1, m_MinSup);
		PruneUnfrequent(m_l1);

		m_pCommon->Notify("Number of 1-tuples: ");
		m_pCommon->NotifyP(VikStd::TStr(m_l1.Len()));

		// Now itemsets with 2 elements

		for (i = 0; i < m_l1.Len(); i++) {
			for (int j = i + 1; j < m_l1.Len(); j++) {
				if (m_ItemRef->At(m_l1[i].FirstItem()).m_c1 != m_ItemRef->At(m_l1[j].FirstItem()).m_c1) {
					m_l2.Add();
					m_l2.Last().Copy(m_l1[i]);
					m_l2.Last().Union(m_l1[j]);
				};
			};
		};

		m_pCommon->Notify("Number of 2-tuples: ");
		m_pCommon->Notify(VikStd::TStr(m_l2.Len()));
		acount += m_l2.Len();

		GetFrequencies(&m_l2);
		MarkUnfrequent(m_l2, m_MinSup);
		MarkUnwanted(m_l2, m_TargetItem);
		FindAllRulesTarget(&m_l1, &m_l2);
		PruneUnfrequent(m_l2);

		m_pCommon->Notify(" (still frequent: ");
		m_pCommon->Notify(VikStd::TStr(m_l2.Len()));
		m_pCommon->NotifyP(")");

		// start with while loops

		TListOfItemsets *l1;
		TListOfItemsets *l2;

		m_l1.Clear();
		l1 = &m_l2;
		l2 = &m_l1;

		int level = 3;
		one_step.SetItemsNumber(m_ItemsCount);

		while (
			(l1->Empty() == false) &&
			((level <= m_MaxRuleLen) || (m_MaxRuleLen < 0))
		) {
			one_step.SetLevel(level);
			one_step.NewLevel2(*l1, *l2);
			acount += l2->Len();

			m_pCommon->Notify("Number of ");
			m_pCommon->Notify(VikStd::TStr(level));
			m_pCommon->Notify("-tuples: ");
			m_pCommon->Notify(VikStd::TStr(l2->Len()));
			acount += l2->Len();

			GetFrequencies(l2);
			MarkUnfrequent(*l2, m_MinSup);
			MarkUnwanted(*l2, m_TargetItem);
			FindAllRulesTarget(l1, l2);
			PruneUnfrequent(*l2);

			m_pCommon->Notify(" (still frequent: ");
			m_pCommon->Notify(VikStd::TStr(l2->Len()));
			m_pCommon->NotifyP(")");

			// swap lists of itemsets

			TListOfItemsets *tl;
			tl = l1;
			l1 = l2;
			l2 = tl;
			level++;
		};

		m_GenItemsetsCount = acount;
		m_pCommon->Notify("Number of all generated itemsets: ");
		m_pCommon->NotifyP(VikStd::TStr(acount));
		m_pCommon->Notify("Number of all generated rules: ");
		m_pCommon->NotifyP(VikStd::TStr(m_ResultsBody.Len()));
		m_pCommon->Post();
	};

	////////////////////////////////////////////////////////////////
	// Find all rules that have target item in head
	////////////////////////////////////////////////////////////////

	void TApriori::FindAllRulesTarget(TListOfItemsets *l1, TListOfItemsets *l2)
	{

		l1->CreateIndex();
		l2->CreateIndex();

		for (int i = 0; i < l2->Len(); i++) {
			if ((l2->At(i).IsSetItem(m_TargetItem)) && (l2->At(i).m_Freq > 0)) {
				l2->At(i).ResetItem(m_TargetItem);
				int in = l1->Find(l2->At(i));

				if (in >= 0) {
					double cc = double(l2->At(i).m_Freq);
					cc /= double(l1->At(in).m_Freq);
					if (cc >= m_Conf) {
						m_ResultsBody.Add();
						m_ResultsBody.Last().Copy(l1->At(in));
						m_ResultsHead.Add(m_TargetItem);
						m_ResultsConf.Add(cc);
						m_ResultsAllCovered.Add(l1->At(in).m_Freq);
						m_ResultsPosCovered.Add(l2->At(i).m_Freq);
						l2->At(i).m_Freq = 0;
					};
				};
				l2->At(i).SetItem(m_TargetItem);
			};
		};
	};

	///////////////////////////////////////////////////////////////////
	// Determines frequencies of itemsets in data.
	///////////////////////////////////////////////////////////////////

	void TApriori::GetFrequencies(TListOfItemsets *l)
	{
		l->SetAllFreqZero();
		for (int i = 0; i < m_Data->Len(); i++) {
			for (int j = 0; j < l->Len(); j++) {
				if (l->At(j).IsSubsetOf(m_Data->At(i)))
					l->At(j).m_Freq++;
			};
		};
	};

	////////////////////////////////////////////////////////////////////
	// Search for normal association rules
	////////////////////////////////////////////////////////////////////

	void TApriori::Perform()
	{

		InitialChecks();
		Initialisation();

		one_step.m_ItemRef = m_ItemRef;

		int acount = 0;

		m_pCommon->Notify("Number of items: ");
		m_pCommon->NotifyP(VikStd::TStr(m_ItemsCount));

		// First create singletons

		int i;
		TItemset temp;
		temp.Init();
		temp.Clear();
		for (i = 0; i < m_ItemsCount; i++) {
			temp.SetItem(i);
			m_l1.Add();
			m_l1.Last().Copy(temp);
			temp.ResetItem(i);
		};

		acount += m_ItemsCount;

		GetFrequencies(&m_l1);
		MarkUnfrequent(m_l1, m_MinSup);
		PruneUnfrequent(m_l1);

		m_pCommon->Notify("Number of 1-tuples: ");
		m_pCommon->NotifyP(VikStd::TStr(m_l1.Len()));

		// Now itemsets with 2 elements

		for (i = 0; i < m_l1.Len(); i++) {
			for (int j = i + 1; j < m_l1.Len(); j++) {
				if (m_ItemRef->At(m_l1[i].FirstItem()).m_c1 != m_ItemRef->At(m_l1[j].FirstItem()).m_c1) {
					m_l2.Add();
					m_l2.Last().Copy(m_l1[i]);
					m_l2.Last().Union(m_l1[j]);
				};
			};
		};

		m_pCommon->Notify("Number of 2-tuples: ");
		m_pCommon->Notify(VikStd::TStr(m_l2.Len()));
		acount += m_l2.Len();

		GetFrequencies(&m_l2);
		MarkUnfrequent(m_l2, m_MinSup);
		//MarkUnwanted(m_l2,m_TargetItem);
		FindAllRules(&m_l1, &m_l2);
		PruneUnfrequent(m_l2);

		m_pCommon->Notify(" (still frequent: ");
		m_pCommon->Notify(VikStd::TStr(m_l2.Len()));
		m_pCommon->NotifyP(")");

		// start with while loops

		TListOfItemsets *l1;
		TListOfItemsets *l2;

		m_l1.Clear();
		l1 = &m_l2;
		l2 = &m_l1;

		int level = 3;
		one_step.SetItemsNumber(m_ItemsCount);

		while ((l1->Empty() == false) && ( (level <= m_MaxRuleLen) || (m_MaxRuleLen < 0) )) {
			one_step.SetLevel(level);
			one_step.NewLevel2(*l1, *l2);
			acount += l2->Len();

			m_pCommon->Notify("Number of ");
			m_pCommon->Notify(VikStd::TStr(level));
			m_pCommon->Notify("-tuples: ");
			m_pCommon->Notify(VikStd::TStr(l2->Len()));
			acount += l2->Len();

			GetFrequencies(l2);

			MarkUnfrequent(*l2, m_MinSup);
			//MarkUnwanted(*l2,m_TargetItem);
			FindAllRules(l1, l2);
			PruneUnfrequent(*l2);

			m_pCommon->Notify(" (still frequent: ");
			m_pCommon->Notify(VikStd::TStr(l2->Len()));
			m_pCommon->NotifyP(")");

			// swap lists of itemsets

			TListOfItemsets *tl;
			tl = l1;
			l1 = l2;
			l2 = tl;
			level++;
		};

		m_GenItemsetsCount = acount;
		m_pCommon->Notify("Number of all generated itemsets: ");
		m_pCommon->NotifyP(VikStd::TStr(acount));
		m_pCommon->Notify("Number of all generated rules: ");
		m_pCommon->NotifyP(VikStd::TStr(m_ResultsBody.Len()));
		m_pCommon->Post();
	};

	////////////////////////////////////////////////////////////////
	// Find all rules on this level
	////////////////////////////////////////////////////////////////

	void TApriori::FindAllRules(TListOfItemsets *l1, TListOfItemsets *l2)
	{

		l1->CreateIndex();
		l2->CreateIndex();

		for (int i = 0; i < l2->Len(); i++) { // each itemset
			if (l2->At(i).m_Freq > 0) {    // each frequent itemset
				for (int j = 0; j < m_ItemsCount; j++) { // each item
					if (l2->At(i).IsSetItem(j) == true) { // each set item
						l2->At(i).ResetItem(j);
						int in = l1->Find(l2->At(i));
						if (in >= 0) { // if such itemset exists
							double cc = double(l2->At(i).m_Freq);
							cc /= double(l1->At(in).m_Freq);
							if (cc >= m_Conf) { // if confident enough
								m_ResultsBody.Add();
								m_ResultsBody.Last().Copy(l1->At(in));
								m_ResultsHead.Add(j);
								m_ResultsConf.Add(cc);
								m_ResultsAllCovered.Add(l1->At(in).m_Freq);
								m_ResultsPosCovered.Add(l2->At(i).m_Freq);
							};
						};
						l2->At(i).SetItem(j);
					};
				};
			};
		};
	};

}
