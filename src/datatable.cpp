#include "datatable.h"

//////////////////////////////////////////////////////////////////////
// Implementation of TAttribute and TDataTable
//////////////////////////////////////////////////////////////////////

#include "lx.h"

namespace VikStd {

	//////////////////////////////////////////////////////////////////////
	// Grammar for saving the table to the binary stream:
	//
	// S   -> <int> LA;                {int=num of attrs, LA=list of attributes}
	//
	// LA  -> A | A LA;                {A=attribute}
	//
	// A   -> <0> <str> <int1> PVL <int2> VLD |
	//        {discrete attibute, str=AttrName, int1=number of declared values
	//         PVL=list of declared values, int2=number of values VLD=list of values }
	//
	//        <1> <str> MIS VLC;
	//        {continuous attibute, str=AttrName, int1=number of missing values
	//         MIS=list of missing values, int2=number of values VLC=list of values}
	//
	// PVL -> <str> | <str> PVL;       {str = value name}
	//
	// VLD -> <int> | <int> VLD;       {int = value}
	//
	// MIS -> <int> | <int> MIS;       {int = index}
	//
	// VLC -> <double> | <double> VLC; {double = value}
	//////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////
	// TAttribute
	//////////////////////////////////////////////////////////////////////

	TAttribute::TAttribute(TAttribute &at, bool CopyData)
	{
		AttrType = at.AttrType;
		AttrName = at.AttrName;
		ValNames.CopyFrom(at.ValNames);
		if (CopyData) {
			dVec.CopyFrom(at.dVec);
			iVec.CopyFrom(at.iVec);
			rMissing.CopyFrom(at.rMissing);
		};
	};

	bool TAttribute::SaveToStream(TOBinStream &f)
	{
		int i;
		if (AttrType == atDiscr) {
			f.PutInt(0);
			f.PutStr(AttrName);

			f.PutInt(ValNames.Len());
			for (i = 0; i < ValNames.Len(); i++) f.PutStr(ValNames[i]);

			f.PutInt(iVec.Len());
			for (i = 0; i < iVec.Len(); i++) f.PutInt(iVec[i]);
		} else {
			f.PutInt(1);
			f.PutStr(AttrName);

			f.PutInt(rMissing.Len());
			for (i = 0; i < rMissing.Len(); i++) f.PutInt(rMissing[i]);

			f.PutInt(dVec.Len());
			for (i = 0; i < dVec.Len(); i++) f.PutDouble(dVec[i]);
		};
		return true;
	};

	//////////////////////////////////////////////////////////////////////

	bool TAttribute::LoadFromStream(TIBinStream &f)
	{
		Clear();
		int i, j, k;
		TStr s;

		f.GetInt(i);

		if (i == 0) { // discrete attribute
			AttrType = atDiscr;
			f.GetStr(s);
			AttrName = s;

			f.GetInt(k);
			for (i = 0; i < k; i++) {
				f.GetStr(s);
				ValNames.Add(s);
			};

			f.GetInt(k);
			for (i = 0; i < k; i++) {
				f.GetInt(j);
				iVec.Add(j);
			};
		} else { // continuous attribute
			AttrType = atCon;
			f.GetStr(s);
			AttrName = s;

			f.GetInt(k);
			for (i = 0; i < k; i++) {
				f.GetInt(j);
				rMissing.Add(j);
			};

			double d;
			f.GetInt(k);
			for (i = 0; i < k; i++) {
				f.GetDouble(d);
				dVec.Add(d);
			};
		};
		return true;
	};

	//////////////////////////////////////////////////////////////////////
	// TDataTable
	//////////////////////////////////////////////////////////////////////

	bool TDataTable::SaveToStream(TOBinStream &f)
	{
		f.PutInt(Attrs.Len());
		for (int i = 0; i < Attrs.Len(); i++) {
			if (Attrs[i].SaveToStream(f) == false) {
				Clear();
				return false;
			};
		};
		return true;
	};

	bool TDataTable::SaveToFile(TStr &FileName)
	{
		TOBinFileStream f;
		if (f.Open(FileName.c_str()) == false) {
			TStr es = "TDataTable: SaveToFile: Cannot open file ";
			es += FileName;
			throw TExc(es);
		};
		return SaveToStream(f);
	};

	//////////////////////////////////////////////////////////////////////

	bool TDataTable::LoadFromStream(TIBinStream &f)
	{
		int i, j;
		f.GetInt(j);
		for (i = 0; i < j; i++) {
			TAttribute at(atDiscr, TStr(""));
			at.LoadFromStream(f);
			Attrs.Add(at);
		};
		return true;
	};

	bool TDataTable::LoadFromFile(TStr &FileName)
	{
		TIBinFileStream f;
		if (f.Open(FileName.c_str()) == false) {
			TStr es = "TDataTable: LoadFromFile: Cannot open file ";
			es += FileName;
			throw TExc(es);
		};
		return LoadFromStream(f);
	};

	//////////////////////////////////////////////////////////////////////

	bool TDataTable::SaveToFileTab(TStr &FileName)
	{
		FILE *fp = fopen(FileName.c_str(), "w");
		if (fp == NULL) {
			TStr es = "TDataTable: SaveToFileTab: Cannot open file ";
			es += FileName;
			throw TExc(es);
		};
		return this->SaveToFileTab(fp);
	};

	//////////////////////////////////////////////////////////////////////

	bool TDataTable::SaveToFileTab(FILE *fp)
	{
		int i, j, k;
		TStr s;

		if (Attrs.Len() == 0) {
			fclose(fp);
			return true;
		};
		// save names of the attributes

		for (i = 0; i < Attrs.Len(); i++) {
			s = Attrs[i].AttrName;
			if (Attrs[i].AttrType == atDiscr) {
				s += "(d)";
			} else {
				s += "(c)";
			};
			if (i == Attrs.Len() - 1) {
				fprintf(fp, "%s", s.c_str());
			} else {
				fprintf(fp, "%s\t", s.c_str());
			}
		};
		fprintf(fp, "\n");
		fflush(fp);

		for (j = 0; j < Attrs[0].Length(); j++) {
			for (i = 0; i < Attrs.Len(); i++) {
				if (Attrs[i].AttrType == atDiscr) {
					k = Attrs[i].iVec[j];
					if (k >= 0) {
						s = Attrs[i].ValNames[k];
					} else {
						s = "?";
					};
					fprintf(fp, "%s\t", s.c_str());
				} else {
					fprintf(fp, "%f\t", Attrs[i].dVec[j]);
				};
			};
			fprintf(fp, "\n");
		};
		fclose(fp);
		return true;
	};

	//////////////////////////////////////////////////////////////////////

	bool TDataTable::LoadFromFileTab(TStr &FileName)
	{

		FILE *fp = fopen(FileName.c_str(), "r");
		if (fp == NULL) {
			TStr s = "TDataTable: LoadFromFileTab: Cannot open file ";
			s += FileName;
			throw TExc(s);
		};
		return LoadFromFileTab(fp);
	};

	//////////////////////////////////////////////////////////////////////

	bool TDataTable::LoadFromFileTab(FILE *fp)
	{
		TLx lx(fp);

		int i;
		TStr s;
		bool FirstIsCon;

		lx.GetSym(true);
		while (lx.SymLnN == 1) {
			if (lx.Sym != syStr) {
				fclose(fp);
				Clear();
				TStr es = "TDataTable: LoadFromFileTab: Illegal attribute name: ";
				es += lx.Str;
				throw TExc(es);
			};
			TAttribute at;
			s = lx.Str.Mid(lx.Str.GetLength() - 3, 3);
			if (s == "(c)") {
				at.AttrType = atCon;
				at.AttrName = lx.Str.Left(lx.Str.GetLength() - 3);
			} else if (s == "(d)") {
				at.AttrType = atDiscr;
				at.AttrName = lx.Str.Left(lx.Str.GetLength() - 3);
			} else {
				fclose(fp);
				Clear();
				TStr es = "TDataTable: LoadFromFileTab: Illegal attribute type specifier - ";
				es += s;
				throw TExc(es);
			};
			Attrs.Add(at);
			if (Attrs.Len() == 1) FirstIsCon = (Attrs[0].AttrType == atCon);

			if (FirstIsCon == false) {
				lx.GetSym(true, true);
			} else {
				lx.GetSym(true);
			};
		};

		while (lx.Sym != syEof) {
			for (i = 0; i < Attrs.Len(); i++) {
				if (Attrs[i].AttrType == atDiscr) { // discrete attribute
					if ((lx.Sym != syStr) && (lx.Sym != syInt)) {
						fclose(fp);
						Clear();
						TStr es = "TDataTable: LoadFromFileTab: Illegal discrete attribute value - \"";
						es += lx.Str + "\"";
						throw TExc(es);
					};
					if (lx.Str != "?") {
						Attrs[i].AddDiscrStrNoCheck(lx.Str);
					} else {
						Attrs[i].iVec.Add(-1);
					};
				} else { // continuous attribute
					if ((lx.Sym != syFlt) && (lx.Sym != syInt)) {
						if ((lx.Sym == syStr) && (lx.Str != "?")) {
							fclose(fp);
							Clear();
							TStr es = "TDataTable: LoadFromFileTab: Illegal continuous attribute value - ";
							es += lx.Str;
							throw TExc(es);
						} else {
							Attrs[i].rMissing.Add(Attrs[i].dVec.Len());
							lx.Flt = 0.0;
						};
					}
					Attrs[i].AddDouble(lx.Flt);
				};


				if (i < Attrs.Len() - 1) {
					if (Attrs[i + 1].AttrType == atDiscr) {
						lx.GetSym(true, true);
					} else {
						lx.GetSym(true);
					};
				} else {
					if (Attrs[0].AttrType == atDiscr) {
						lx.GetSym(true, true);
					} else {
						lx.GetSym(true);
					};
				};
			};
		};

		for (i = 0; i < Attrs.Len(); i++) Attrs[i].SortByValName();
		fclose(fp);
		return true;
	};

	//////////////////////////////////////////////////////////////////////

	bool TDataTable::SaveToFileC5(TStr &FileName, int TargetClass, bool ClassFirst, bool EndingDot)
	{

		if ((TargetClass < 0) || (TargetClass >= Attrs.Len())) {
			TStr es = "TDataTable: SaveToFileC5(): Target class index out of bounds";
			throw TExc(es);
		};
		if (Attrs[TargetClass].AttrType != atDiscr) {
			TStr es = "TDataTable: SaveToFileC5(): Target class is not discrete";
			throw TExc(es);
		};

		TStr NamesFile, DataFile;
		NamesFile = FileName;
		NamesFile += ".names";

		DataFile = FileName;
		DataFile += ".data";

		FILE *fpn = fopen(NamesFile.c_str(), "w");
		if (fpn == NULL) {
			TStr es = "TDataTable: SaveToFileC5(): Cannot open .names file ";
			es += NamesFile;
			throw TExc(es);
		};

		FILE *fpd = fopen(DataFile.c_str(), "w");
		if (fpd == NULL) {
			TStr es = "TDataTable: SaveToFileC5(): Cannot open .data file ";
			es += DataFile;
			fclose(fpn);
			throw TExc(es);
		};

		int i, j, k;
		TAttribute *at;

		for (i = 0; i < Attrs.Len(); i++) {
			at = &Attrs[i];
			at->AttrName.Replace(' ', '_');
			for (j = 0; j < at->ValNames.Len(); j++)
				at->ValNames[j].Replace(' ', '_');
		};

		if (ClassFirst) {

			// first describe class attribute
			at = &Attrs[TargetClass];
			for (i = 0; i < at->ValNames.Len(); i++) {
				fprintf(fpn, "%s", at->ValNames[i].c_str());
				if (i < at->ValNames.Len() - 1) fprintf(fpn, ",");
			};
			fprintf(fpn, ".  | Target attribute\n\n");

			// now describe other attributes
			for (i = 0; i < Attrs.Len(); i++) {
				if (i != TargetClass) {
					if (Attrs[i].AttrType == atDiscr) {

						at = &Attrs[i];
						fprintf(fpn, "%s: ", at->AttrName.c_str());

						for (j = 0; j < at->ValNames.Len(); j++) {
							fprintf(fpn, "%s", at->ValNames[j].c_str());
							if (j < at->ValNames.Len() - 1) fprintf(fpn, ",");
						};
						fprintf(fpn, ".\n");
					} else {
						fprintf(fpn, "%s: continuous.\n", Attrs[i].AttrName.c_str());
					};
				};
			};

			//now save to .data file
			for (k = 0; k < Attrs[0].Len(); k++) {

				for (i = 0; i < Attrs.Len(); i++) {
					if (i != TargetClass) {
						if (Attrs[i].AttrType == atDiscr) {
							at = &Attrs[i];
							int val = at->iVec[k];
							if (val >= 0) {
								fprintf(fpd, "%s , ", at->ValNames[val].c_str());
							} else {
								fprintf(fpd, "? , ");
							};
						} else {
							if (Attrs[i].DoubleIsMissing(k)) {
								fprintf(fpd, "%f , ", Attrs[i].dVec[k]);
							} else {
								fprintf(fpd, "? , ");
							};
						};
					};
				};

				at = &Attrs[TargetClass];
				char Dot = ' ';
				if (EndingDot) Dot = '.';
				fprintf(fpd, "%s %c\n", at->ValNames[at->iVec[k]].c_str(), Dot);
			};
		} else { // ClassFirst==false

			// first specify class attribute

			fprintf(fpn, "%s.  | Target attribute\n\n", Attrs[TargetClass].AttrName.c_str());

			// now describe all attributes
			for (i = 0; i < Attrs.Len(); i++) {

				if (Attrs[i].AttrType == atDiscr) {
					at = &Attrs[i];
					fprintf(fpn, "%s: ", at->AttrName.c_str());

					for (j = 0; j < at->ValNames.Len(); j++) {
						fprintf(fpn, "%s", at->ValNames[j].c_str());
						if (j < at->ValNames.Len() - 1) fprintf(fpn, ",");
					};
					fprintf(fpn, ".\n");
				} else {
					fprintf(fpn, "%s: continuous.\n", Attrs[i].AttrName.c_str());
				};
			};

			//now save to .data file
			for (k = 0; k < Attrs[0].Len(); k++) {
				for (i = 0; i < Attrs.Len(); i++) {
					if (Attrs[i].AttrType == atDiscr) {
						at = &Attrs[i];
						int val = at->iVec[k];
						if (val >= 0) {
							fprintf(fpd, "%s ", at->ValNames[val].c_str());
						} else {
							fprintf(fpd, "? ");
						};
					} else {
						if (Attrs[i].DoubleIsMissing(k)) {
							fprintf(fpd, "%f ", Attrs[i].dVec[k]);
						} else {
							fprintf(fpd, "? ");
						};
					};
					if (i < Attrs.Len() - 1) {
						fprintf(fpd, ", ");
					} else {
						if (EndingDot) {
							fprintf(fpd, ".\n");
						} else {
							fprintf(fpd, "\n");
						};
					};
				};
			};
		};

		fclose(fpn);
		fclose(fpd);
		return true;
	};

	/////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////
	inline double log2(double t)
	{
		return log(t) / log(2.0);
	};
	//inline double log10(double t){ return log(t)/log(10); };

	/////////////////////////////////////////////////////////////
	// Information Gain for discrete attribute //////////////////
	double InformationGainD(TAttribute *Source, TAttribute *Class, TExampleWindow *ew)
	{

		if (Source == NULL)   throw TExc("InformationGainD: Source=NULL.");
		if (Class == NULL) throw TExc("InformationGainD: Class=NULL.");
		if (ew == NULL) throw TExc("InformationGainD: ExampleWindow=NULL.");

		if (Source == Class)
			throw TExc("InformationGainD: test and class attribute cannot be the same.");

		if (Source->AttrType != atDiscr)
			throw TExc("InformationGainD: test attribute is not discrete.");

		if (Class->AttrType != atDiscr)
			throw TExc("InformationGainD: class attribute is not discrete.");

		if (Source->Len() != Class->Len())
			throw TExc("InformationGainD: test and class attribute are not of the same size.");

		if (Source->Len() != ew->Length())
			throw TExc("InformationGainD: sizes of attributes and example window differ.");

		if (ew->TrueCount() == 0)
			throw TExc("InformationGainD: no examples in an example window.");

		int ca = Source->ValNames.Len();
		int cc = Class->ValNames.Len();
		TVec<double> atv(ca), clv(cc);
		TVVec<double> contab(ca, cc);
		int i, j;

		// Init data-structures
		for (i = 0; i < ca; i++) {
			atv[i] = 0;
			for (j = 0; j < cc; j++) contab.At(i, j) = 0;
		};
		for (i = 0; i < cc; i++) clv[i] = 0;

		// Start counting
		for (i = 0; i < Source->Len() ; i++) {
			if (ew->IsIn(i)) {
				clv[Class->iVec[i]]++;
				atv[Source->iVec[i]]++;
				contab.At(Source->iVec[i], Class->iVec[i])++;
			};
		};

		// Calculate value
		double e = 0, ep = 0, t, t1, t2;
		int ecount = ew->TrueCount();

		for (i = 0; i < cc; i++) {
			if (clv[i] > 0) {
				t = clv[i] / ecount;
				e += t * log2(t);
			};
		};

		for (i = 0; i < ca; i++) {
			if (atv[i] > 0) {
				t = 0;
				t2 = atv[i] / ecount; // p(attr-val i)
				for (j = 0; j < cc; j++) {
					if (contab.At(i, j) > 0) {
						t1 = contab.At(i, j) / clv[j]; // p(vij | clj)
						t1 /= t2;
						t += t1 * log2(t1);
					};
				};
				ep += t * t2;
			};
		};
		return -e + ep;
	};

	/////////////////////////////////////////////////////////////////
	// Information Gain for continuous attribute ////////////////////

	double InformationGainC(TAttribute *Source, double Treshold, TAttribute *Class, TExampleWindow *ew)
	{

		if (Source == NULL)   throw TExc("InformationGainC: Source=NULL.");
		if (Class == NULL) throw TExc("InformationGainC: Class=NULL.");
		if (ew == NULL) throw TExc("InformationGainC: ExampleWindow=NULL.");

		if (Source == Class)
			throw TExc("InformationGainC: test and class attribute cannot be the same.");

		if (Source->AttrType != atCon)
			throw TExc("InformationGainC: test attribute is not continuous.");

		if (Class->AttrType != atDiscr)
			throw TExc("InformationGainC: class attribute is not discrete.");

		if (Source->Len() != Class->Len())
			throw TExc("InformationGainC: test and class attribute are not of the same size.");

		if (Source->Len() != ew->Length())
			throw TExc("InformationGainC: sizes of attributes and example window differ.");

		if (ew->TrueCount() == 0)
			throw TExc("InformationGainC: no examples in an example window.");

		int ca = 2;
		int cc = Class->ValNames.Len();
		TVec<double> atv(ca), clv(cc);
		TVVec<double> contab(ca, cc);
		int i, j;

		// Init data-structures
		for (i = 0; i < ca; i++) {
			atv[i] = 0;
			for (j = 0; j < cc; j++) contab.At(i, j) = 0;
		};
		for (i = 0; i < cc; i++) clv[i] = 0;


		// Start counting
		for (i = 0; i < Source->Len() ; i++) {
			if (ew->IsIn(i)) {
				clv[Class->iVec[i]]++;
				if (Source->dVec[i] <= Treshold) {
					atv[0]++;
					contab.At(0, Class->iVec[i])++;
				} else {
					atv[1]++;
					contab.At(1, Class->iVec[i])++;
				};
			};
		};

		// Calculate value
		double e = 0, ep = 0, t, t1, t2;
		int ecount = ew->TrueCount();

		for (i = 0; i < cc; i++) {
			if (clv[i] > 0) {
				t = clv[i] / ecount;
				e += t * log2(t);
			};
		};

		for (i = 0; i < ca; i++) {
			if (atv[i] > 0) {
				t = 0;
				t2 = atv[i] / ecount; // p(attr-val i)
				for (j = 0; j < cc; j++) {
					if (contab.At(i, j) > 0) {
						t1 = contab.At(i, j) / clv[j]; // p(vij | clj)
						t1 /= t2;
						t += t1 * log2(t1);
					};
				};
				ep += t * t2;
			};
		};
		return -e + ep;
	};

	//////////////////////////////////////////////////////////////////////////////
	// Next two functions are used for dynamic discretisation
	// Given source and class attribute and example window, it finds the best cut value
	//////////////////////////////////////////////////////////////////////////////

	inline double DynDiscrEntropy(TVec< TPair<double, int> >& v, int c, int t)
	{
		TVec<double> cll(c), clg(c);
		int i;
		double t1 = 0, t2 = 0, t3 = v.Len();
		for (i = 0; i < c; i++) {
			cll[i] = 0;
			clg[i] = 0;
		};

		for (i = 0; i < t; i++) {
			cll[ v[i].m_c2 ]++;
		};
		for (i = t; i < v.Len(); i++) {
			clg[ v[i].m_c2 ]++;
		};

		for (i = 0; i < c; i++) {
			if (cll[i] > 0) {
				t1 += (cll[i] / t) * log2(cll[i] / t);
			};
			if (clg[i] > 0) {
				t2 += (clg[i] / (v.Len() - t)) * log2(clg[i] / (v.Len() - t));
			};
		};
		return -( ((double)t / v.Len()) * t1 + ((v.Len() - (double)t) / v.Len()) * t2);
	};

	// Main function - returns best treshold /////////////////////

	double DynamicDiscretisation(TAttribute *Source, TAttribute *Class, TExampleWindow *ew)
	{

		if (Source == NULL)   throw TExc("DynamicDiscretisation: Source=NULL.");
		if (Class == NULL) throw TExc("DynamicDiscretisation: Class=NULL.");
		if (ew == NULL) throw TExc("DynamicDiscretisation: ExampleWindow=NULL.");

		if (Source->AttrType != atCon) throw TExc("DynamicDiscretisation: Source attribute is not continuous.");
		if (Class->AttrType != atDiscr) throw TExc("DynamicDiscretisation: Class attribute is not discrete.");

		if (Source->Len() != Class->Len())
			throw TExc("DynamicDiscretisation: sizes of attr and class attribute differ.");
		if (Source->Len() != ew->Length())
			throw TExc("DynamicDiscretisation: sizes of attribute and example window differ.");
		if (ew->TrueCount() == 0)
			throw TExc("DynamicDiscretisation: cannot process empty subset");

		TVec< TPair<double, int> > tv;
		TPair< double, int > tp;

		int i;
		for (i = 0; i < ew->Length(); i++)
			if (ew->IsIn(i)) {
				tp.m_c1 = Source->dVec[i];
				tp.m_c2 = Class->iVec[i];
				tv.Add(tp);
			};

		tv.Sort();
		int best = -1;
		double best_e, temp;

		for (i = 0; i < tv.Len() - 1; i++) {
			if ((tv[i].m_c2 != tv[i + 1].m_c2) && (tv[i].m_c1 != tv[i + 1].m_c1)) {
				temp = DynDiscrEntropy(tv, Class->ValNames.Len(), i + 1);
				if (best == -1) {
					best = i;
					best_e = temp;
				};
				if (best_e > temp) {
					best_e = temp;
					best = i;
				}; // minimise entropy!
			};
		};
		return (tv[best].m_c1 + tv[best + 1].m_c1) / 2;
	};

	//////////////////////////////////////////////////////////////////////////////

	double Entropy(TAttribute *Class, TExampleWindow *ew)  // entropy of current example subset
	{

		if (ew == NULL) throw TExc("Entropy: ExampleWindow=NULL.");
		if (Class->AttrType != atDiscr) throw TExc("Entropy: Class attribute is not discrete.");
		if (Class->Len() != ew->Length()) throw TExc("Entropy: sizes of attribute and example window differ.");
		if (ew->TrueCount() == 0) return 1000;

		TVec<int> v;
		int i, all = 0;

		for (i = 0; i < Class->ValNames.Len(); i++) v.Add(0);

		// count
		for (i = 0; i < Class->iVec.Len(); i++) {
			if (ew->IsIn(i)) {
				v[Class->iVec[i]]++;
				all++;
			};
		};

		double res = 0;
		for (i = 0; i < Class->ValNames.Len(); i++) {
			if (v[i] > 0) { // VJIDBG
				double vi = v[i];
				vi = (v[i] / double(all)) * log2(v[i] / double(all));
				//res += (v[i]/all)*log2(v[i]/all);  // p(ci) * log2( p(ci) )
				res += vi;
			};
		};

		return -res;
	};

}
