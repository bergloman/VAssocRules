#pragma once

///////////////////////////////////////////////////////////////////////////

#include "ds.h"
#include "str.h"
#include "binfilestream.h"

///////////////////////////////////////////////////////////////////////////

namespace VikStd {

    class TExampleWindow {
    protected:
        TBoolVec m_vec;
    public:
        TExampleWindow() {};
        TExampleWindow(int size, bool DefaultValue = true) {
            for (int i = 0; i < size; i++) m_vec.Add(DefaultValue);
        };
        TExampleWindow(TExampleWindow &e) {
            //Resize(e.Length());
            //for (int i=0; i<m_vec.Len(); i++) m_vec[i] = e.IsIn(i);
            m_vec.QuickCopy(e.m_vec.GetArray(), e.m_vec.Len());
        };

        void Resize(int new_l, bool DefaultValue = true) {
            if (new_l < 0)
                throw TExc("TExampleWindow: Resize(): illegal new size.");
            m_vec.Clear();
            m_vec.Gen(new_l);
            memset(m_vec.GetArray(), DefaultValue, new_l);
            m_vec.ResizeX(new_l);
            //for(int i=0; i<new_l; i++) m_vec.Add(DefaultValue);
        };

        void operator=(const TExampleWindow &e) {
            //m_vec.Clr();
            //for (int i=0; i<e.m_vec.Len(); i++) m_vec.Add(e.m_vec[i]);
            m_vec.QuickCopy(e.m_vec.GetArray(), e.m_vec.Len());
        };

        bool IsIn(int i) const {
            //if ((i<0)||(i>=m_vec.Len())) throw TExc("TExampleWindow: IsIn() index out of bounds.");
            return m_vec[i];
        };

        void Put(int i, bool val) {
            //if ((i<0)||(i>=m_vec.Len())) throw TExc("TExampleWindow: Put() index out of bounds.");
            m_vec[i] = val;
        };

        void PutToAll(bool val) {
            for (int i = 0; i < m_vec.Len(); i++)
                m_vec[i] = val;
        };
        int Length() {
            return m_vec.Len();
        };

        int TrueCount() { // returns number of true values
            int i, count = 0;
            for (i = 0; i < m_vec.Len(); i++)
                if (m_vec[i])
                    count++;
            return count;
        };

        void Add(TExampleWindow &e) { // put true to all true indexes from e - other indexes remain the same
            for (int i = 0; i < m_vec.Len(); i++)
                m_vec[i] = m_vec[i] || e.m_vec[i];
        };

        void Remove(TExampleWindow &e) { // put false to all true indexes from e - other indexes remain the same
            for (int i = 0; i < m_vec.Len(); i++)
                m_vec[i] = m_vec[i] && (e.m_vec[i] == false);
        };
    };

    //////////////////////////////////////////////////////////////////////////

    typedef enum { atDiscr, atCon, atErr } TAttrType;

    class TAttribute {
    public:
        TAttrType AttrType;
        TStr AttrName;

        TVec<double> dVec;
        TVec<int> iVec;
        TVec<int> rMissing;
        TVec<TStr> ValNames;

        TAttribute() {
            AttrType = atErr;
            AttrName = "";
        };
        TAttribute(TAttrType at, TStr name) {
            AttrType = at;
            AttrName = name;
        };
        TAttribute(TAttribute &at, bool CopyData = true);

        ~TAttribute() {};

        void operator=(const TAttribute &a) {
            Clear();
            AttrType = a.AttrType;
            AttrName = a.AttrName;
            int i;
            for (i = 0; i < a.dVec.Len(); i++) dVec.Add(a.dVec[i]);
            for (i = 0; i < a.iVec.Len(); i++) iVec.Add(a.iVec[i]);
            for (i = 0; i < a.rMissing.Len(); i++) rMissing.Add(a.rMissing[i]);
            for (i = 0; i < a.ValNames.Len(); i++) ValNames.Add(a.ValNames[i]);
        };

        int ValNamesFind(TStr &s) {
            for (int i = 0; i < ValNames.Len(); i++) {
                if (ValNames[i] == s) return i;
            };
            return -1;
        };

        void AddDouble(double d) {
            assert(AttrType == atCon);
            dVec.Add(d);
        };

        void AddDiscrStr(TStr &s) {
            if (AttrType != atDiscr) {
                VikStd::StdErrorLog.m_Msg = "AddDiscrStr: attribute not discrete.";
                throw TExc(VikStd::StdErrorLog.m_Msg);
            };
            int i = ValNamesFind(s);
            if (i < 0) {
                VikStd::StdErrorLog.m_Msg = "AddDiscrStr: attribute ";
                VikStd::StdErrorLog.m_Msg = s;
                VikStd::StdErrorLog.m_Msg = " not found.";
                throw TExc(VikStd::StdErrorLog.m_Msg);
            };
            iVec.Add(i);
        };

        void AddDiscrStrNoCheck(TStr &s) { // if value not declared yet, declare it
            if (AttrType != atDiscr) {
                VikStd::StdErrorLog.m_Msg = "AddDiscrStrNoCheck: attribute not discrete.";
                throw TExc(VikStd::StdErrorLog.m_Msg);
            };
            int i = ValNamesFind(s);
            if (i < 0) {
                DeclareValue(s, false);
                i = ValNamesFind(s);
            };
            iVec.Add(i);
        };

        void AddDiscrInt(int i) {
            if (AttrType != atDiscr) {
                VikStd::StdErrorLog.m_Msg = "AddDiscrInt: attribute not discrete.";
                throw TExc(VikStd::StdErrorLog.m_Msg);
            };
            if ((i < 0) || (i >= ValNames.Len())) {
                VikStd::StdErrorLog.m_Msg = "AddDiscrInt: parameter i out of bounds.";
                throw TExc(VikStd::StdErrorLog.m_Msg);
            };
            iVec.Add(i);
        };

        void DeclareValue(TStr &s, bool NoPreviousDeclaration) {
            int i = ValNamesFind(s);
            if (i >= 0) {
                if (NoPreviousDeclaration) {
                    TStr es = "TAttribute: DeclareValue: value ";
                    es += s;
                    es += " already declared.";
                    throw TExc(es);
                };
            } else ValNames.Add(s);
        };

        bool DoubleIsMissing(int index) {
            for (int i = 0; i < rMissing.Len(); i++) {
                if (index == rMissing[i]) return true;
            };
            return false;
        };

        bool SaveToStream(TOBinStream &f);
        bool LoadFromStream(TIBinStream &f);

        void Clear() {
            AttrName = "";
            dVec.Clr();
            iVec.Clr();
            rMissing.Clr();
            ValNames.Clr();
        };

        void ClearData() { // clears only data, preserves AttrName and ValNames
            dVec.Clr();
            iVec.Clr();
            rMissing.Clr();
        };

        int Length() {
            if (AttrType == atDiscr) return iVec.Len();
            return dVec.Len();
        };

        int Len() {
            if (AttrType == atDiscr) return iVec.Len();
            return dVec.Len();
        };

        TStr ValueAsStr(int index) {
            if (AttrType == atDiscr) {
                if (iVec[index] < 0)
                    return TStr("?");
                return ValNames[iVec[index]];
            };
            if (DoubleIsMissing(index) == true)
                return TStr("?");
            return TStr(dVec[index]);
        };

        void SortByValName() { // sorts value names (also updates iVec!)
            if (AttrType == atCon) return;
            TIntVec tiv;
            TVec<TStr> tsv;
            int i, j;
            for (i = 0; i < ValNames.Len(); i++) {
                tiv.Add(0);
                tsv.Add(ValNames[i]);
            };
            ValNames.Sort();
            for (i = 0; i < tsv.Len(); i++) {
                int ci = 0;
                for (j = 0; j < ValNames.Len(); j++)
                    if (ValNames[j] == tsv[i])
                        ci = j;
                tiv[i] = ci;
            };
            for (i = 0; i < iVec.Len(); i++) {
                if (iVec[i] >= 0)
                    iVec[i] = tiv[iVec[i]];
            };
        };
    };

    /////////////////////////////////////////////////////////////////////////////

    class TDataTable {
    public:
        TVec<TAttribute> Attrs;

        TDataTable() {};
        virtual ~TDataTable() {};

        int FindAttrWithName(TStr &s) {
            for (int i = 0; i < Attrs.Len(); i++) {
                if (Attrs[i].AttrName == s) return i;
            };
            return -1;
        };

        void Clear() {
            Attrs.Clr();
        };
        void CopyNamesAndDefs(TDataTable &source) { // copies attributes, their names and value definitions
            Clear();
            for (int i = 0; i < source.Attrs.Len(); i++) {
                TAttribute at(source.Attrs[i], false);
                Attrs.Add(at);
            };
        };

        void AddRowFromSource(TDataTable &source, int index) { // add row 'index' from source
            for (int i = 0; i < Attrs.Len(); i++) {

                if (Attrs[i].AttrType != source.Attrs[i].AttrType)
                    throw TExc("TDataTable: AddRowFromSource(): incompatible types.");

                if (Attrs[i].AttrType == atDiscr) {
                    Attrs[i].iVec.Add(source.Attrs[i].iVec[index]);
                } else {
                    Attrs[i].dVec.Add(source.Attrs[i].dVec[index]);
                    if (source.Attrs[i].DoubleIsMissing(index)) {
                        Attrs[i].rMissing.Add(Attrs[i].dVec.Len() - 1);
                    };
                };
            };
        };

        bool CheckLengths() { // check if all attributes have the same length
            if (Attrs.Len() == 0) return true;
            for (int i = 1; i < Attrs.Len(); i++)
                if (Attrs[i].Len() != Attrs[0].Len())
                    return false;
            return true;
        };

        bool SaveToFileX(TStr &FileName) { // type of file determined by file extension
            TStr s = FileName.GetFileEx();
            if (s == "bin")
                return SaveToFile(FileName);
            return SaveToFileTab(FileName);
        };

        bool LoadFromFileX(TStr &FileName) { // type of file determined by file extension
            TStr s = FileName.GetFileEx();
            if (s == "bin") {
                bool res = LoadFromFile(FileName);
                //SaveToFileTab(FileName + ".txt");
                return res;
            };
            return LoadFromFileTab(FileName);
        };

        bool SaveToStream(TOBinStream &f);
        bool LoadFromStream(TIBinStream &f);

        bool SaveToFile(TStr &FileName);
        bool LoadFromFile(TStr &FileName);

        bool SaveToFileTab(TStr &FileName);
        bool SaveToFileTab(FILE *fp);
        bool LoadFromFileTab(TStr &FileName);
        bool LoadFromFileTab(FILE *fp);

        bool SaveToFileC5(TStr &FileName, int TargetClass, bool ClassFirst = false, bool EndingDot = false); // save into files X.names and X.data
        //bool LoadFromFileC5(TStr &FileName);
    };

    ///////////////////////////////////////////////////////////////////////////

    inline void RandomSplit(TDataTable *in, TDataTable *out1, TDataTable*out2, double ratio = 0.9) {
        out1->CopyNamesAndDefs((*in));
        out2->CopyNamesAndDefs((*in));

        TRnd rnd;
        for (int i = 0; i < in->Attrs[0].Len(); i++) {
            if (rnd.GetFlt() > ratio) {
                out2->AddRowFromSource((*in), i);
            } else {
                out1->AddRowFromSource((*in), i);
            };
        };
    };

    ///////////////////////////////////////////////////////////////////////////

    double InformationGainD(TAttribute *Source, TAttribute *Class, TExampleWindow *ew); // for discrete attribute Source
    double InformationGainC(TAttribute *Source, double Treshold, TAttribute *Class, TExampleWindow *ew); // for continuous attribute Source
    double DynamicDiscretisation(TAttribute *Source, TAttribute *Class, TExampleWindow *ew); // finds best cut point - to lower the entropy
    double Entropy(TAttribute *Class, TExampleWindow *ew); // entropy of current example subset

    ///////////////////////////////////////////////////////////////////////////

}
