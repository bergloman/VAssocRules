#include "assocrules.h"

namespace VikApriori {

	/////////////////////////////////////////////////////
	// Call appropriate FSS algorithm
	/////////////////////////////////////////////////////

	void TAssocRules::DoFSS(VikStd::TProperties &prop)
	{
		VikStd::TVar var;
		prop.Find("FSSType", var);
		switch (var.m_Int) {
			case FSS_StatCorr:
				DoStatCorr(prop);
				break;
			case FSS_VRelief:
				DoVRelief(prop);
				break;
			case FSS_VRelief2:
				DoVRelief2(prop);
				break;
			case FSS_OddsRatio:
				DoOddsRatio(prop);
				break;
			case FSS_None: {
				for (int i = 0; i < m_NumOfItems; i++) // select all attributes
					m_UseAttribute[i] = true;
			};
			break;
			default:
				throw VikStd::TExc("TAssocRules: DoFSS: illegal algorithm type.");
		};
	};

	/////////////////////////////////////////////////////

	void TAssocRules::DoStatCorr(VikStd::TProperties &prop)
	{

		VikStd::TVec<double> mean(m_NumOfItems), sigma(m_NumOfItems);
		int i, j, k;

		for (i = 0; i < m_NumOfItems; i++) {
			mean[i] = 0.0;
			sigma[i] = 0.0;
		};

		// calculate mean value
		for (i = 0; i < m_DB.Len(); i++) {
			for (j = 0; j < m_NumOfItems; j++) {
				if (m_DB[i].IsSetItem(j) == true)
					mean[j] += 1.0;
			};
		};
		for (j = 0; j < m_NumOfItems; j++) mean[j] /= m_DB.Len();

		// calculate sigma
		for (i = 0; i < m_DB.Len(); i++) {
			for (j = 0; j < m_NumOfItems; j++) {
				if (m_DB[i].IsSetItem(j) == true) {
					sigma[j] += (1 - mean[j]) * (1 - mean[j]);
				} else {
					sigma[j] += mean[j] * mean[j];
				}
			};
		};
		for (j = 0; j < m_NumOfItems; j++) {
			sigma[j] /= m_DB.Len();
			sigma[j] = sqrt(sigma[j]);
		};


		VikStd::TVVec<double> matrix00(m_NumOfItems, m_NumOfItems);
		VikStd::TVVec<double> matrix01(m_NumOfItems, m_NumOfItems);
		VikStd::TVVec<double> matrix10(m_NumOfItems, m_NumOfItems);
		VikStd::TVVec<double> matrix11(m_NumOfItems, m_NumOfItems);

		for (j = 0; j < m_NumOfItems; j++) {
			for (k = 0; k < m_NumOfItems; k++) {
				matrix00.At(j, k) = 0.0;
				matrix01.At(j, k) = 0.0;
				matrix10.At(j, k) = 0.0;
				matrix11.At(j, k) = 0.0;
			};
		};
		// calculate pobabilities of pairs of values
		for (i = 0; i < m_DB.Len(); i++) {
			for (j = 0; j < m_NumOfItems; j++) {
				for (k = j + 1; k < m_NumOfItems; k++) {
					if (m_DB[i].IsSetItem(j) == true) {
						if (m_DB[i].IsSetItem(k) == true) {
							matrix11.At(j, k) += 1.0;
						} else {
							matrix10.At(j, k) += 1.0;
						}
					} else {
						if (m_DB[i].IsSetItem(k) == true) {
							matrix01.At(j, k) += 1.0;
						} else {
							matrix00.At(j, k) += 1.0;
						}
					};
				};
			};
		};
		for (j = 0; j < m_NumOfItems; j++) {
			for (k = j + 1; k < m_NumOfItems; k++) {
				matrix00.At(j, k) /= m_DB.Len();
				matrix01.At(j, k) /= m_DB.Len();
				matrix10.At(j, k) /= m_DB.Len();
				matrix11.At(j, k) /= m_DB.Len();
			};
		};


		// calculate correlation matrix
		VikStd::TVVec<double> corr(m_NumOfItems, m_NumOfItems);
		for (j = 0; j < m_NumOfItems; j++) {
			if (sigma[j] > 0) {
				for (k = j + 1; k < m_NumOfItems; k++) {
					if (sigma[k] > 0) {
						corr.At(j, k) = matrix00.At(j, k) * ( ((0 - mean[j]) / sigma[j]) * ((0 - mean[k]) / sigma[k]) );
						corr.At(j, k) += matrix01.At(j, k) * ( ((0 - mean[j]) / sigma[j]) * ((1 - mean[k]) / sigma[k]) );
						corr.At(j, k) += matrix10.At(j, k) * ( ((1 - mean[j]) / sigma[j]) * ((0 - mean[k]) / sigma[k]) );
						corr.At(j, k) += matrix11.At(j, k) * ( ((1 - mean[j]) / sigma[j]) * ((1 - mean[k]) / sigma[k]) );
					};
				};
			};
		};

		// Filter out pairs correlated more than treshold and eliminate one of the items

		m_UseAttribute.Clear();
		for (i = 0; i < m_NumOfItems; i++) // select all attributes
			m_UseAttribute.Add(true);

		VikStd::TVar var;
		prop.Find("StatCorrTreshold", var);
		int Removed = 0;

		for (j = 0; j < m_NumOfItems; j++) {
			if (m_TargetItem != j) {
				for (k = j + 1; k < m_NumOfItems; k++) {
					if (m_TargetItem != k) {
						if (corr.At(j, k) > var.m_Double) {
							if (m_UseAttribute[k] != false) Removed++;
							m_UseAttribute[k] = false;
						};
					};
				};
			};
		};
		VikStd::TStr s = VikStd::TStr("\tStatistical correlation: ") + Removed + " items removed.";
		m_Common->NotifyP(s);
		m_UseAttribute[m_TargetItem] = true;
	};

	/////////////////////////////////////////////////////

	void TAssocRules::DoVRelief(VikStd::TProperties &prop)
	{
		VikStd::TVar mv, var;
		prop.Find("PercentOfSelectedAttrs", var);
		prop.Find("m", mv);
		int m = VikStd::Round(mv.m_Double);

		VikStd::TRnd r(0); // random generator, seed creation with time()

		VikStd::TVec<double> q(m_NumOfItems);
		VikStd::TVec<double> misses;
		VikStd::TVec<double> hits;
		VikStd::TVec<int> i_misses;
		VikStd::TVec<int> i_hits;
		VikStd::TVec<int> index;

		// init datastructures
		int i, j, k;
		for (i = 0; i < m_NumOfItems; i++) q[i] = 0.0;
		for (i = 0; i < m; i++) {
			index.Add(r.GetInt(m_DB.Len()));
			misses.Add(-1.0);
			hits.Add(-1.0);
			i_misses.Add(-1);
			i_hits.Add(-1);
		};

		// get nearest hits and misses
		for (i = 0; i < m_DB.Len(); i++) {
			for (j = 0; j < m; j++) {
				if (index[j] != i) {

					double dist = 0;
					for (k = 0; k < m_NumOfItems; k++) {
						if ((k != m_TargetItem) && (m_DB[i].IsSetItem(k) != m_DB[j].IsSetItem(k))) dist += 1.0;
					};

					if (m_DB[i].IsSetItem(m_TargetItem) == m_DB[j].IsSetItem(m_TargetItem)) {
						if (hits[j] < 0) {
							hits[j] = dist;
							i_hits[j] = i;
						} else if (hits[j] > dist) {
							hits[j] = dist;
							i_hits[j] = i;
						};
					} else {
						if (misses[j] < 0) {
							misses[j] = dist;
							i_misses[j] = i;
						} else if (misses[j] > dist) {
							misses[j] = dist;
							i_misses[j] = i;
						};
					};
				};
			};
		};

		for (i = 0; i < m; i++) {
			for (j = 0; j < m_NumOfItems; j++) {
				double th = 0;
				if (m_DB[index[i]].IsSetItem(m_TargetItem) == true) {

					if (m_DB[i_hits[i]].IsSetItem(j) == m_DB[index[i]].IsSetItem(j)) {
						if (m_DB[index[i]].IsSetItem(j) == true) th += 1;
						else th -= 1;
					};

					if ((m_DB[i_misses[i]].IsSetItem(j) != m_DB[index[i]].IsSetItem(j)) && (m_DB[index[i]].IsSetItem(j) == true)) th += 1;
					if ((m_DB[i_misses[i]].IsSetItem(j) != m_DB[index[i]].IsSetItem(j)) && (m_DB[index[i]].IsSetItem(j) == false)) th -= 1;
				} else {
					if ((m_DB[i_hits[i]].IsSetItem(j) == m_DB[index[i]].IsSetItem(j)) && (m_DB[index[i]].IsSetItem(j) == true)) th -= 1;
					if ((m_DB[i_hits[i]].IsSetItem(j) == m_DB[index[i]].IsSetItem(j)) && (m_DB[index[i]].IsSetItem(j) == false)) th += 1;

					if ((m_DB[i_misses[i]].IsSetItem(j) != m_DB[index[i]].IsSetItem(j)) && (m_DB[index[i]].IsSetItem(j) == true)) th -= 1;
					if ((m_DB[i_misses[i]].IsSetItem(j) != m_DB[index[i]].IsSetItem(j)) && (m_DB[index[i]].IsSetItem(j) == false)) th += 1;
				};
				q[j] += th / m;
			};
		};

		// Sort attributes
		VikStd::TVec<int> index1(m_NumOfItems);
		for (i = 0; i < m_NumOfItems; i++) 
			index1[i] = i;

		for (i = 0; i < m_NumOfItems - 1; i++) {
			for (j = i + 1; j < m_NumOfItems; j++) {
				if (q[j] > q[j - 1]) {
					q.Swap(j, j - 1);
					index1.Swap(j, j - 1);
				};
			};
		};

		for (i = 0; i < m_NumOfItems - 1; i++)
			if (index1[i] == m_TargetItem)
				index1.Swap(i, index1.Len() - 1);

		for (i = 0; i < m_NumOfItems; i++) // deselect all attributes
			m_UseAttribute.Add(false);

		j = 0;
		for (i = 0; i < VikStd::Round(m_NumOfItems * var.m_Double + j); i++) { // select only best attributes
			if (index1[i] == m_TargetItem) j++;
			m_UseAttribute[index1[i]] = true;
		};
		m_UseAttribute[m_TargetItem] = true;
	};

	/////////////////////////////////////////////////////

	void TAssocRules::DoVRelief2(VikStd::TProperties &prop)
	{

		VikStd::TVar mv, var;
		prop.Find("PercentOfSelectedAttrs", var);
		prop.Find("m", mv);
		int m = VikStd::Round(mv.m_Double);

		VikStd::TRnd r(0); // random generator, seed creation with time()

		VikStd::TVec<double> q(m_NumOfItems);
		VikStd::TVec<double> misses;
		VikStd::TVec<double> hits;
		VikStd::TVec<int> i_misses;
		VikStd::TVec<int> i_hits;
		VikStd::TVec<int> index;

		// init datastructures
		int i, j, k;
		for (i = 0; i < m_NumOfItems; i++)
			q[i] = 0.0;
		for (i = 0; i < m; i++) {
			index.Add(r.GetInt(m_DB.Len()));
			misses.Add(-1.0);
			hits.Add(-1.0);
			i_misses.Add(-1);
			i_hits.Add(-1);
		};

		// get nearest hits and misses
		for (i = 0; i < m_DB.Len(); i++) {
			for (j = 0; j < m; j++) {
				if (index[j] != i) {

					double dist = 0;
					for (k = 0; k < m_NumOfItems; k++) {
						if ((k != m_TargetItem) && (m_DB[i].IsSetItem(k) != m_DB[j].IsSetItem(k))) dist += 1.0;
					};

					if (m_DB[i].IsSetItem(m_TargetItem) == m_DB[j].IsSetItem(m_TargetItem)) {
						if (hits[j] < 0) {
							hits[j] = dist;
							i_hits[j] = i;
						} else if (hits[j] > dist) {
							hits[j] = dist;
							i_hits[j] = i;
						};
					} else {
						if (misses[j] < 0) {
							misses[j] = dist;
							i_misses[j] = i;
						} else if (misses[j] > dist) {
							misses[j] = dist;
							i_misses[j] = i;
						};
					};
				};
			};
		};

		for (i = 0; i < m; i++) {
			for (j = 0; j < m_NumOfItems; j++) {
				double th = 0;
				if (m_DB[index[i]].IsSetItem(m_TargetItem) == true) {

					if (m_DB[i_hits[i]].IsSetItem(j) == m_DB[index[i]].IsSetItem(j)) {
						if (m_DB[index[i]].IsSetItem(j) == true) th += 2;
						else th -= 2;
					};

					if ((m_DB[i_misses[i]].IsSetItem(j) != m_DB[index[i]].IsSetItem(j)) && (m_DB[index[i]].IsSetItem(j) == true)) th += 2;
					if ((m_DB[i_misses[i]].IsSetItem(j) != m_DB[index[i]].IsSetItem(j)) && (m_DB[index[i]].IsSetItem(j) == false)) th -= 2;
					if ((m_DB[i_misses[i]].IsSetItem(j) == m_DB[index[i]].IsSetItem(j)) && (m_DB[index[i]].IsSetItem(j) == true)) th -= 1;
				} else {
					if ((m_DB[i_hits[i]].IsSetItem(j) == m_DB[index[i]].IsSetItem(j)) && (m_DB[index[i]].IsSetItem(j) == true)) th -= 2;
					if ((m_DB[i_hits[i]].IsSetItem(j) == m_DB[index[i]].IsSetItem(j)) && (m_DB[index[i]].IsSetItem(j) == false)) th += 2;
					if (m_DB[i_hits[i]].IsSetItem(j) != m_DB[index[i]].IsSetItem(j)) th -= 1;

					if ((m_DB[i_misses[i]].IsSetItem(j) != m_DB[index[i]].IsSetItem(j)) && (m_DB[index[i]].IsSetItem(j) == true)) th -= 2;
					if ((m_DB[i_misses[i]].IsSetItem(j) != m_DB[index[i]].IsSetItem(j)) && (m_DB[index[i]].IsSetItem(j) == false)) th += 2;
					if ((m_DB[i_misses[i]].IsSetItem(j) == m_DB[index[i]].IsSetItem(j)) && (m_DB[index[i]].IsSetItem(j) == true)) th -= 1;
				};
				q[j] += th / m;
			};
		};

		// Sort attributes
		VikStd::TVec<int> index1(m_NumOfItems);
		for (i = 0; i < m_NumOfItems; i++) index1[i] = i;

		for (i = 0; i < m_NumOfItems - 1; i++) {
			for (j = i + 1; j < m_NumOfItems; j++) {
				if (q[j] > q[j - 1]) {
					q.Swap(j, j - 1);
					index1.Swap(j, j - 1);
				};
			};
		};

		for (i = 0; i < m_NumOfItems - 1; i++)
			if (index1[i] == m_TargetItem)
				index1.Swap(i, index1.Len() - 1);

		for (i = 0; i < m_NumOfItems; i++) // deselect all attributes
			m_UseAttribute.Add(false);

		j = 0;
		int Sel = VikStd::Round(m_NumOfItems * var.m_Double);
		for (i = 0; i < Sel + j; i++) { // select only best attributes
			if (index1[i] == m_TargetItem) j++;
			m_UseAttribute[index1[i]] = true;
		};
		m_UseAttribute[m_TargetItem] = true;
	};

	/////////////////////////////////////////////////////

	void TAssocRules::DoOddsRatio(VikStd::TProperties &prop)
	{
		VikStd::TVar var;
		prop.Find("PercentOfSelectedAttrs", var);

		VikStd::TVec<double> v00, v10, v01, v11, q, v0, v1;
		double n0 = 0, n1 = 0, nall = 0;

		int i, j;
		for (i = 0; i < m_NumOfItems; i++) {
			v00.Add(0.0);
			v10.Add(0.0); // First digit is value for class, second for attribute
			v01.Add(0.0);
			v11.Add(0.0);
			q.Add(0.0);
			v1.Add(0.0);
			v0.Add(0.0);
		};

		for (i = 0; i < m_DB.Len(); i++) {
			nall += 1.0;
			if (m_DB[i].IsSetItem(m_TargetItem) != false) {
				n1 += 1.0;
			} else n0 += 1.0;
			for (j = 0; j < m_NumOfItems; j++) {
				if (j != m_TargetItem) {
					if (m_DB[i].IsSetItem(j) != false) {
						v1[j] += 1.0;
						if (m_DB[i].IsSetItem(m_TargetItem)) {
							v11[j] += 1.0;
						} else {
							v01[j] += 1.0;
						};
					} else {
						v0[j] += 1.0;
						if (m_DB[i].IsSetItem(m_TargetItem)) {
							v10[j] += 1.0;
						} else {
							v00[j] += 1.0;
						};
					};
				};
			};
		};

		// calculate OddsRatio
		for (j = 0; j < m_NumOfItems; j++) {
			if (j == m_TargetItem) { // Skipp target attribute
				q[j] = 0.0;
				continue;
			};
			double t1 = 0.0, t2 = 0.0;

			if (v11[j] == 0.0) {
				t1 = (1 / (nall * nall)) / (1 - (1 / (nall * nall)));
			} else if (v11[j] == n1) {
				t1 = (1 - (1 / (nall * nall))) / (1 / (nall * nall));
			} else t1 = (v11[j] / n1) / (1 - (v11[j] / n1));

			if (v01[j] == 0.0) {
				t2 = (1 / (nall * nall)) / (1 - (1 / (nall * nall)));
			} else if (v01[j] == n0) {
				t2 = (1 - (1 / (nall * nall))) / (1 / (nall * nall));
			} else t2 = (v01[j] / n0) / (1 - (v01[j] / n0));

			q[j] = VikStd::log2(t1 / t2);
		};

		// Sort attributes by value of OddsRatio
		VikStd::TVec<int> index1(m_NumOfItems);
		for (i = 0; i < m_NumOfItems; i++) index1[i] = i;

		for (i = 0; i < m_NumOfItems - 2; i++) {
			for (j = m_NumOfItems - 1; j > i; j--) {
				if (q[j] > q[j - 1]) {
					q.Swap(j, j - 1);
					index1.Swap(j, j - 1);
				};
			};
		};

		for (i = 0; i < m_NumOfItems - 1; i++)
			if (index1[i] == m_TargetItem)
				index1.Swap(i, index1.Len() - 1);

		for (i = 0; i < m_NumOfItems; i++) // deselect all attributes
			m_UseAttribute.Add(false);

		j = 0;
		for (i = 0; i < MIN(m_NumOfItems, VikStd::Round(m_NumOfItems * var.m_Double + j)); i++) { // select only best attributes
			if (index1[i] == m_TargetItem) j++;
			m_UseAttribute[index1[i]] = true;
		};
		m_UseAttribute[m_TargetItem] = true;
	};

	/////////////////////////////////////////////////////

	bool TAssocRules::TestFSSParams(VikStd::TProperties &prop)
	{
		VikStd::TVar var;

		// check type

		if (prop.Find("FSSType", var) == false)
			throw VikStd::TExc("TAssocRules: TestFSSParams: FSS algorithm type not specified in properties.");

		if (var.m_Type != VikStd::VTInt)
			throw VikStd::TExc("TAssocRules: TestFSSParams: illegal FSS algorithm type (not VikStd::VTInt).");

		if ((var.m_Int < FSS_StatCorr) || (var.m_Int > FSS_None))
			throw VikStd::TExc("TAssocRules: TestFSSParams: illegal FSS algorithm type (out of bounds).");

		// check additional parameters - dependant on type

		if (var.m_Int == FSS_None) return true;

		if (var.m_Int == FSS_StatCorr) {

			if (prop.Find("StatCorrTreshold", var) == false)
				throw VikStd::TExc("TAssocRules: TestFSSParams: StatCorrTreshold not specified.");

			if (var.m_Type != VikStd::VTDouble)
				throw VikStd::TExc("TAssocRules: TestFSSParams: illegal StatCorrTreshold parameter (not VikStd::VTDouble).");

			if ((var.m_Double < 0) || (var.m_Double > 1))
				throw VikStd::TExc("TAssocRules: TestFSSParams: illegal StatCorrTreshold parameter (out of bounds).");

			return true;
		};

		if (var.m_Int == FSS_VRelief) {

			if (prop.Find("PercentOfSelectedAttrs", var) == false)
				throw VikStd::TExc("TAssocRules: TestFSSParams: PercentOfSelectedAttrs not specified.");

			if (var.m_Type != VikStd::VTDouble)
				throw VikStd::TExc("TAssocRules: TestFSSParams: illegal PercentOfSelectedAttrs parameter (not VikStd::VTDouble).");

			if ((var.m_Double < 0) || (var.m_Double > 1))
				throw VikStd::TExc("TAssocRules: TestFSSParams: illegal PercentOfSelectedAttrs parameter (out of bounds).");

			if (prop.Find("m", var) == false)
				throw VikStd::TExc("TAssocRules: TestFSSParams: 'm' not specified.");

			if (var.m_Type != VikStd::VTInt)
				throw VikStd::TExc("TAssocRules: TestFSSParams: illegal 'm' parameter (not VikStd::VTInt).");

			if (var.m_Int < 1)
				throw VikStd::TExc("TAssocRules: TestFSSParams: illegal 'm' parameter (<1).");

			return true;
		};

		if (var.m_Int == FSS_VRelief2) {

			if (prop.Find("PercentOfSelectedAttrs", var) == false)
				throw VikStd::TExc("TAssocRules: TestFSSParams: PercentOfSelectedAttrs not specified.");

			if (var.m_Type != VikStd::VTDouble)
				throw VikStd::TExc("TAssocRules: TestFSSParams: illegal PercentOfSelectedAttrs parameter (not VikStd::VTDouble).");

			if ((var.m_Double < 0) || (var.m_Double > 1))
				throw VikStd::TExc("TAssocRules: TestFSSParams: illegal PercentOfSelectedAttrs parameter (out of bounds).");

			if (prop.Find("m", var) == false)
				throw VikStd::TExc("TAssocRules: TestFSSParams: 'm' not specified.");

			if (var.m_Type != VikStd::VTInt)
				throw VikStd::TExc("TAssocRules: TestFSSParams: illegal 'm' parameter (not VikStd::VTInt).");

			if (var.m_Int < 1)
				throw VikStd::TExc("TAssocRules: TestFSSParams: illegal 'm' parameter (<1).");

			return true;
		};

		if (var.m_Int == FSS_OddsRatio) {

			if (prop.Find("PercentOfSelectedAttrs", var) == false)
				throw VikStd::TExc("TAssocRules: TestFSSParams: PercentOfSelectedAttrs not specified.");

			if (var.m_Type != VikStd::VTDouble)
				throw VikStd::TExc("TAssocRules: TestFSSParams: illegal PercentOfSelectedAttrs parameter (not VikStd::VTDouble).");

			if ((var.m_Double < 0) || (var.m_Double > 1))
				throw VikStd::TExc("TAssocRules: TestFSSParams: illegal PercentOfSelectedAttrs parameter (out of bounds).");

			return true;
		};

		return true;
	};

	/////////////////////////////////////////////////////

	bool TAssocRules::TestAprioriParams(VikStd::TProperties &prop)
	{

		VikStd::TVar var;

		// Check MinConf

		if (prop.Find("MinConf", var) == false)
			throw VikStd::TExc("TAssocRules: TestAprioriParams: MinConf not specified in properties.");

		if (var.m_Type != VikStd::VTDouble)
			throw VikStd::TExc("TAssocRules: TestAprioriParams: illegal MinConf (not VikStd::VTDouble).");

		if ((var.m_Double < 0) || (var.m_Double > 1))
			throw VikStd::TExc("TAssocRules: TestAprioriParams: illegal MinConf (out of bounds).");


		// Check MinSup

		if (prop.Find("MinSup", var) == false)
			throw VikStd::TExc("TAssocRules: TestAprioriParams: MinSup not specified in properties.");

		if (var.m_Type != VikStd::VTDouble)
			throw VikStd::TExc("TAssocRules: TestAprioriParams: illegal MinSup (not VikStd::VTDouble).");

		if ((var.m_Double < 0) || (var.m_Double > 1))
			throw VikStd::TExc("TAssocRules: TestAprioriParams: illegal MinSup (out of bounds).");

		return true;
	};

	/////////////////////////////////////////////////////

	bool TAssocRules::TestClassifierConstructionParams(VikStd::TProperties &prop)
	{

		VikStd::TVar var;

		if (prop.Find("ClassConstrAlgo", var) == false)
			throw VikStd::TExc("TAssocRules: TestClassifierConstructionParams: Classifier construction algorithm type not specified in properties.");
		if (var.m_Type != VikStd::VTInt)
			throw VikStd::TExc("TAssocRules: TestClassifierConstructionParams: illegal CC algorithm type (not VikStd::VTInt).");
		if ((var.m_Int < CC_UseAll) || (var.m_Int > CC_Nada))
			throw VikStd::TExc("TAssocRules: TestClassifierConstructionParams: illegal CC algorithm type (out of bounds).");

		if (var.m_Int != CC_Nada) {
			if(prop.Find("DefaultRule", var) == false)
				throw VikStd::TExc("TAssocRules: TestClassifierConstructionParams: Default rule not specified in properties.");
			if (var.m_Type != VikStd::VTBool)
				throw VikStd::TExc("TAssocRules: TestClassifierConstructionParams: illegal type of Default rule flag (not VTBool).");
		};

		// check algorithm dependant parameters
		if ((var.m_Int == CC_UseNBest) || (var.m_Int == CC_UseNBestEach)) {
			if (prop.Find("N", var) == false)
				throw VikStd::TExc("TAssocRules: TestClassifierConstructionParams: parameter N not specified.");
			if (var.m_Type != VikStd::VTInt)
				throw VikStd::TExc("TAssocRules: TestClassifierConstructionParams: illegal type of parameter N (expected Int).");
		};

		return true;
	};

	/////////////////////////////////////////////////////
	// Create binary data and all other utility members
	// that help transform the data.
	/////////////////////////////////////////////////////

	void TAssocRules::PrepareData()
	{
		if (m_DataTable == NULL)
			throw VikStd::TExc("TAssocRules::PrepareData(): m_DataTable==NULL.");

		ClearAllIntermediateData();
		int i, j, k, all;

		all = 0;
		for (i = 0; i < m_DataTable->Attrs.Len(); i++) {
			m_NumOfValues.Add(m_DataTable->Attrs[i].ValNames.Len());
			m_StartOfValues.Add(all);
			all += m_NumOfValues.Last();
		};

		m_glbUCharManager.InitItemsetSize(all, m_Common); // xxx - is this safe enough - what if function is reran???
		m_NumOfItems = all;

		for (i = 0; i < m_DataTable->Attrs.Len(); i++) {
			for (j = 0; j < m_NumOfValues[i]; j++) {
				TItemRef ref;
				ref.m_c1 = i;
				ref.m_c2 = j;
				m_Reference.Add(ref);
			};
		};

		for (k = 0; k < m_DataTable->Attrs[0].Len(); k++) { // for each record
			TItemset t;
			t.Init();
			for (i = 0; i < m_DataTable->Attrs.Len(); i++) { // for each attribute
				if (m_DataTable->Attrs[i].iVec[k] >= 0)
					t.SetItem(m_StartOfValues[i] + m_DataTable->Attrs[i].iVec[k]);
			};
			m_DB.Add(t);
		};

	};

	/////////////////////////////////////////////////////

	void TAssocRules::ClearAllIntermediateData()
	{
		m_Classifier.Clear();
		m_DB.Clear();
		m_NumOfValues.Clear();
		m_Reference.Clear();
		m_Rules.Clear();
		m_StartOfValues.Clear();
		m_UseAttribute.Clear();
	};

	//////////////////////////////////////////////////////////////
	// Create TRule objects from binary results (TItemset)
	// and add them to m_Rules. Don't delete previous entries.
	//////////////////////////////////////////////////////////////

	void TAssocRules::CreateRulesFromItemsets(TApriori *apr)
	{
		if (apr == NULL)
			throw VikStd::TExc("TAssocRules: CreateRulesFromItemsets: Parameter apr is NULL.");

		TListOfItemsets *body = apr->GetResultsBody();
		VikStd::TVec<int> *head = apr->GetResultsHead();
		VikStd::TVec<double> *conf = apr->GetResultConf();

		for (int i = 0; i < body->Len(); i++) {
			VikRuleClassifiers::TRule rule;
			VikRuleClassifiers::TLiteral lit;
			for (int j = 0; j < m_NumOfItems; j++) {
				if (body->At(i).IsSetItem(j)) {
					lit.m_Attr = m_Reference[j].m_c1;
					lit.m_iVal = m_Reference[j].m_c2;
					lit.m_Negation = false;
					lit.m_Type = VikStd::atDiscr;
					rule.m_Body.Add(lit);
				};
			};
			rule.m_Head.m_Attr = m_Reference[head->At(i)].m_c1;
			rule.m_Head.m_iVal = m_Reference[head->At(i)].m_c2;
			rule.m_Head.m_Negation = false;
			rule.m_Head.m_Type = VikStd::atDiscr;

			rule.m_Freq = apr->GetResultsAllCovered()->At(i);
			rule.m_Pos = apr->GetResultsPosCovered()->At(i);
			rule.m_Neg = rule.m_Freq - rule.m_Pos;
			rule.m_Conf = rule.m_Pos;
			rule.m_Conf /= rule.m_Freq;
			m_Rules.Add(rule);
		};
	};

	////////////////////////////////////////////////////////////
	// Top level function that calls all the others
	////////////////////////////////////////////////////////////

	void TAssocRules::Run(VikStd::TProperties &prop)
	{

		TestFSSParams(prop);
		TestAprioriParams(prop);
		TestClassifierConstructionParams(prop);

		m_Common->NotifyP("Preparing data...");
		PrepareData();

		//m_Common->NotifyP("Performing feature subset selection...");
		//DoFSS(prop);

		m_Common->NotifyP("Running APRIORI algorithm...");
		RunApriori(prop);

		m_Common->NotifyP("Creating classifier...");
		RunClassifierConstruction(prop);
	};

	///////////////////////////////////////////////////////////

	void TAssocRules::RunX(VikStd::TProperties &prop, VikStd::TDataTable *Test, FILE *fp)
	{

		TestFSSParams(prop);
		TestAprioriParams(prop);
		//TestClassifierConstructionParams(prop);

		m_Common->NotifyP("Preparing data...");
		PrepareData();

		//m_Common->NotifyP("Performing feature subset selection...");
		//DoFSS(prop);

		m_Common->NotifyP("Running APRIORI algorithm...");
		RunApriori(prop);

		m_Common->NotifyP("Creating classifier...");
		int cca[] = {1, 2, 5, 10, 15, 20};

		// UseNBestRules //////////////////////////////////////////
		int i;
		for (i = 0; i < 6; i++) {
			VikRuleClassifiers::TRulesClassifier r;

			r.m_DataTable = m_DataTable;
			r.m_Input = &m_Rules;
			r.m_RulesCount = cca[i];
			r.m_ClassAttr = m_TargetAttribute;
			r.m_DefaultClass = true;
			r.UseNRulesForAllClasses();

			VikRuleClassifiers::TRulesClassifier tr;
			tr.m_DataTable = Test;
			tr.m_Input = &r.m_ResultingClassifier;
			tr.m_ClassAttr = m_TargetAttribute;
			tr.TestClassifier();
			//matrix.At(0+i,fold)=tr.m_Results.m_Accuracy;
			printf("%f ", tr.m_Results.m_Accuracy);
			fprintf(fp, "%f\t", tr.m_Results.m_Accuracy);
		};
		printf("\n");

		for (i = 0; i < 6; i++) {
			VikRuleClassifiers::TRulesClassifier r;

			r.m_DataTable = m_DataTable;
			r.m_Input = &m_Rules;
			r.m_RulesCount = cca[i];
			r.m_ClassAttr = m_TargetAttribute;
			r.m_DefaultClass = false;
			r.UseNRulesForAllClasses();

			VikRuleClassifiers::TRulesClassifier tr;
			tr.m_DataTable = Test;
			tr.m_ClassAttr = m_TargetAttribute;
			tr.m_Input = &r.m_ResultingClassifier;
			tr.TestClassifier();
			//matrix.At(6+i,fold)=tr.m_Results.m_Accuracy;
			printf("%f ", tr.m_Results.m_Accuracy);
			fprintf(fp, "%f\t", tr.m_Results.m_Accuracy);
		};
		printf("\n");

		// UseNBestEach Rules //////////////////////////////////////////

		for (i = 0; i < 6; i++) {
			VikRuleClassifiers::TRulesClassifier r;

			r.m_DataTable = m_DataTable;
			r.m_Input = &m_Rules;
			r.m_RulesCount = cca[i];
			r.m_ClassAttr = m_TargetAttribute;
			r.m_DefaultClass = true;
			r.UseNRulesForEachClass();

			VikRuleClassifiers::TRulesClassifier tr;
			tr.m_DataTable = Test;
			tr.m_ClassAttr = m_TargetAttribute;
			tr.m_Input = &r.m_ResultingClassifier;
			tr.TestClassifier();
			//matrix.At(12+i,fold)=tr.m_Results.m_Accuracy;
			printf("%f ", tr.m_Results.m_Accuracy);
			fprintf(fp, "%f\t", tr.m_Results.m_Accuracy);
		};
		printf("\n");
		for (i = 0; i < 6; i++) {
			VikRuleClassifiers::TRulesClassifier r;

			r.m_DataTable = m_DataTable;
			r.m_Input = &m_Rules;
			r.m_RulesCount = cca[i];
			r.m_ClassAttr = m_TargetAttribute;
			r.m_DefaultClass = false;
			r.UseNRulesForEachClass();

			VikRuleClassifiers::TRulesClassifier tr;
			tr.m_DataTable = Test;
			tr.m_ClassAttr = m_TargetAttribute;
			tr.m_Input = &r.m_ResultingClassifier;
			tr.TestClassifier();
			//matrix.At(18+i,fold)=tr.m_Results.m_Accuracy;
			printf("%f ", tr.m_Results.m_Accuracy);
			fprintf(fp, "%f\t", tr.m_Results.m_Accuracy);
		};
		printf("\n");

		// Nada //////////////////////////////////////////

		for (i = 0; i < 6; i++) {
			VikRuleClassifiers::TRulesClassifier r;

			r.m_DataTable = m_DataTable;
			r.m_Input = &m_Rules;
			r.m_RulesCount = cca[i];
			r.m_ClassAttr = m_TargetAttribute;
			r.m_DefaultClass = true;
			r.NadaAlgo();

			VikRuleClassifiers::TRulesClassifier tr;
			tr.m_DataTable = Test;
			tr.m_ClassAttr = m_TargetAttribute;
			tr.m_Input = &r.m_ResultingClassifier;
			tr.TestClassifier();
			//matrix.At(24+i,fold)=tr.m_Results.m_Accuracy;
			printf("%f ", tr.m_Results.m_Accuracy);
			fprintf(fp, "%f\t", tr.m_Results.m_Accuracy);
		};
		printf("\n");
		for (i = 0; i < 6; i++) {
			VikRuleClassifiers::TRulesClassifier r;

			r.m_DataTable = m_DataTable;
			r.m_Input = &m_Rules;
			r.m_RulesCount = cca[i];
			r.m_ClassAttr = m_TargetAttribute;
			r.m_DefaultClass = false;
			r.NadaAlgo();

			VikRuleClassifiers::TRulesClassifier tr;
			tr.m_DataTable = Test;
			tr.m_ClassAttr = m_TargetAttribute;
			tr.m_Input = &r.m_ResultingClassifier;
			tr.TestClassifier();
			//matrix.At(30+i,fold)=tr.m_Results.m_Accuracy;
			printf("%f ", tr.m_Results.m_Accuracy);
			fprintf(fp, "%f\t", tr.m_Results.m_Accuracy);
		};
		fprintf(fp, "\n");
	};

	////////////////////////////////////////////////////////
	// Runs apriori algorithm and collects produced rules
	// For association rules for classification
	////////////////////////////////////////////////////////

	void TAssocRules::RunApriori(VikStd::TProperties &prop)
	{

		VikStd::TVar var;
		TApriori apr(&m_DB, m_Common);

		prop.Find("MinConf", var);
		apr.m_Conf = var.m_Double;

		prop.Find("MinSup", var);
		apr.m_MinSup = var.m_Double * (double)(m_DataTable->Attrs[0].Len());

		apr.m_MaxRuleLen = 10;
		if (prop.Find("MaxRuleLen", var) != false) {
			if (var.m_Type == VikStd::VTInt) {
				if (var.m_Int > 1) apr.m_MaxRuleLen = var.m_Int;
			};
		};

		apr.m_UseItem = &m_UseAttribute;
		apr.m_ItemRef = &m_Reference;
		for (int j = 0; j < m_NumOfItems; j++) m_UseAttribute.Add(false);
		m_ItemsetsGenerated = 0;

		for (int i = 0; i < m_NumOfValues[m_TargetAttribute]; i++) {
			m_TargetItem = m_StartOfValues[m_TargetAttribute] + i;
			m_Common->Notify("Target value: ");
			m_Common->NotifyP(m_DataTable->Attrs[m_Reference[m_TargetItem].m_c1].ValNames[m_Reference[m_TargetItem].m_c2]);
			m_Common->NotifyP("Performing feature subset selection...");
			for (int j = 0; j < m_UseAttribute.Len(); j++) m_UseAttribute[j] = false;
			DoFSS(prop);

			apr.m_TargetItem = m_StartOfValues[m_TargetAttribute] + i;
			apr.PerformWithTarget();
			CreateRulesFromItemsets(&apr);
			m_ItemsetsGenerated += apr.m_GenItemsetsCount;
		};
	};

	////////////////////////////////////////////////////////
	// Runs apriori algorithm and collects produced rules
	// For normal association rules
	////////////////////////////////////////////////////////

	void TAssocRules::RunAprioriNormal(VikStd::TProperties &prop)
	{

		VikStd::TVar var;
		TApriori apr(&m_DB, m_Common);

		prop.Find("MinConf", var);
		apr.m_Conf = var.m_Double;

		prop.Find("MinSup", var);
		apr.m_MinSup = var.m_Double * (double)(m_DataTable->Attrs[0].Len());

		apr.m_MaxRuleLen = 10;
		if (prop.Find("MaxRuleLen", var) != false) {
			if (var.m_Type == VikStd::VTInt) {
				if (var.m_Int > 1) apr.m_MaxRuleLen = var.m_Int;
			};
		};

		apr.m_ItemRef = &m_Reference;
		apr.Perform();
		CreateRulesFromItemsets(&apr);
		m_ItemsetsGenerated = apr.m_GenItemsetsCount;
	};

	////////////////////////////////////////////////////////
	// Runs classifier construction algorithm
	////////////////////////////////////////////////////////

	void TAssocRules::RunClassifierConstruction(VikStd::TProperties &prop)
	{

		VikRuleClassifiers::TRulesClassifier r;

		r.m_DataTable = m_DataTable;
		r.m_Input = &m_Rules;
		r.m_RulesCount = 5;
		r.m_ClassAttr = m_TargetAttribute;

		VikStd::TVar var;
		prop.Find("ClassConstrAlgo", var);
		if (var.m_Int == CC_UseAll) {
			r.UseAllRules();
		} else if (var.m_Int == CC_UseNBest) {
			prop.Find("N", var);
			r.m_RulesCount = var.m_Int;
			r.UseNRulesForAllClasses();
		} else if (var.m_Int == CC_UseNBestEach) {
			prop.Find("N", var);
			r.m_RulesCount = var.m_Int;
			r.UseNRulesForEachClass();
		};

		prop.Find("DefaultRule", var);
		r.m_DefaultClass = var.m_Bool;

		for (int i = 0; i < r.m_ResultingClassifier.Len(); i++)
			m_Classifier.Add(r.m_ResultingClassifier[i]);
	};

	/////////////////////////////////////////////////////////////
	// Ten-fold cross validation
	/////////////////////////////////////////////////////////////

	void TAssocRules10FoldCV::Run(VikStd::TProperties &prop)
	{
		if (m_AllData == NULL)
			throw VikStd::TExc("TAssocRules10FoldCV.Run: m_AllData==NULL.");

		VikStd::TVec<int> Fold;
		VikStd::TRnd rnd;
		int i, j;

		int num = m_AllData->Attrs[0].Len() / 10;
		int mod = m_AllData->Attrs[0].Len() % 10;
		int len = m_AllData->Attrs[0].Len();
		for (i = 0; i < m_AllData->Attrs[0].Len(); i++) Fold.Add(-1);

		for (i = 0; i < 10; i++) {
			if (i < mod) {
				for (j = 0; j < num + 1; j++) {
					int r = rnd.GetInt(len);
					while (Fold[r] >= 0) r = rnd.GetInt(len);
					Fold[r] = i;
				};
			} else {
				for (j = 0; j < num; j++) {
					int r = rnd.GetInt(len);
					while (Fold[r] >= 0) r = rnd.GetInt(len);
					Fold[r] = i;
				};
			};
		};

		m_ItemsetsGenerated = 0;

		for (i = 0; i < 10; i++) {
			m_Common->NotifyP("");
			m_Common->NotifyP(VikStd::TStr("************ Testing fold ") + i);
			m_Common->NotifyP("");


			VikStd::TDataTable Learn;    // data for learning
			VikStd::TDataTable Test;     // data for testing

			Learn.CopyNamesAndDefs(*m_AllData);
			Test.CopyNamesAndDefs(*m_AllData);

			// arrange data
			for (j = 0; j < Fold.Len(); j++) {
				if (Fold[j] == i) {
					Test.AddRowFromSource(*m_AllData, j);
				} else {
					Learn.AddRowFromSource(*m_AllData, j);
				};
			};

			if (Learn.CheckLengths() == false) 
				throw VikStd::TExc("Internal error: datatable is corrupt.");
			if (Test.CheckLengths() == false) 
				throw VikStd::TExc("Internal error: datatable is corrupt.");
				
			// association rules learning
			TAssocRules ar(m_Common, &Learn);
			ar.SetTargetAttribute(m_TargetAttribute);
			ar.Run(prop);
			m_ItemsetsGenerated += ar.m_ItemsetsGenerated;

			// testing classifier on test data
			VikRuleClassifiers::TRulesClassifier rc;
			rc.m_ClassAttr = m_TargetAttribute;
			rc.m_DataTable = &Test;
			rc.m_Input = ar.GetClassifier();
			rc.m_RulesCount = 5;
			rc.TestClassifier();
			m_AccuracyTab[i] = rc.m_Results.m_Accuracy;
			m_Res.Add(rc.m_Results);
		};

		m_OverallAccuracy = 0;
		for (i = 0; i < 10; i++) 
			m_OverallAccuracy += m_AccuracyTab[i];
		m_OverallAccuracy /= 10;
	};

	///////////////////////////////////////////////////////////////////////////////

	void TAssocRules10FoldCV::RunX(VikStd::TProperties &prop, FILE *fp)
	{
		if (m_AllData == NULL)
			throw VikStd::TExc("TAssocRules10FoldCV.Run: m_AllData==NULL.");

		VikStd::TVec<int> Fold;
		VikStd::TRnd rnd;
		int i, j;

		int num = m_AllData->Attrs[0].Len() / 10;
		int mod = m_AllData->Attrs[0].Len() % 10;
		int len = m_AllData->Attrs[0].Len();
		for (i = 0; i < m_AllData->Attrs[0].Len(); i++) Fold.Add(-1);

		for (i = 0; i < 10; i++) {
			if (i < mod) {
				for (j = 0; j < num + 1; j++) {
					int r = rnd.GetInt(len);
					while (Fold[r] >= 0) r = rnd.GetInt(len);
					Fold[r] = i;
				};
			} else {
				for (j = 0; j < num; j++) {
					int r = rnd.GetInt(len);
					while (Fold[r] >= 0) r = rnd.GetInt(len);
					Fold[r] = i;
				};
			};
		};

		m_ItemsetsGenerated = 0;

		for (i = 0; i < 10; i++) {
			m_Common->NotifyP("");
			m_Common->NotifyP(VikStd::TStr("************ Testing fold ") + i);
			m_Common->NotifyP("");


			VikStd::TDataTable Learn;    // data for learning
			VikStd::TDataTable Test;     // data for testing

			Learn.CopyNamesAndDefs(*m_AllData);
			Test.CopyNamesAndDefs(*m_AllData);

			// arrange data
			for (j = 0; j < Fold.Len(); j++) {
				if (Fold[j] == i) {
					Test.AddRowFromSource(*m_AllData, j);
				} else {
					Learn.AddRowFromSource(*m_AllData, j);
				};
			};

			if (Learn.CheckLengths() == false) throw VikStd::TExc("Internal error: datatable is corrupt.");
			if (Test.CheckLengths() == false) throw VikStd::TExc("Internal error: datatable is corrupt.");
			// association rules learning

			TAssocRules ar(m_Common, &Learn);
			ar.SetTargetAttribute(m_TargetAttribute);
			ar.RunX(prop, &Test, fp);
		};
	};

	//////////////////////////////////////////////////////////////////////
	// Search for normal association rules
	//////////////////////////////////////////////////////////////////////

	void TAssocRules::RunNormal(VikStd::TProperties &prop)
	{
		TestAprioriParams(prop);
		PrepareData();
		RunAprioriNormal(prop);
	};

}
