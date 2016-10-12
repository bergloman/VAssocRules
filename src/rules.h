#pragma once

///////////////////////////////////////////////////////

#include "stdvik.h"

namespace VikRuleClassifiers {


    ///////////////////////////////////////////////////////

    class TLiteral {
    public:
        VikStd::TAttrType m_Type;
        int m_Attr;
        int m_iVal;
        double m_dVal; // m_Attr < m_dVal
        bool m_Negation;

        TLiteral() {
            m_Type = VikStd::atErr;
            m_Attr = -1;
            m_iVal = -1;
            m_dVal = 0;
            m_Negation = false;
        };

        TLiteral(const TLiteral &l) {
            m_Type = l.m_Type;
            m_Attr = l.m_Attr;
            m_iVal = l.m_iVal;
            m_dVal = l.m_dVal;
            m_Negation = l.m_Negation;
        };

        void operator=(const TLiteral &l) {
            m_Type = l.m_Type;
            m_Attr = l.m_Attr;
            m_iVal = l.m_iVal;
            m_dVal = l.m_dVal;
            m_Negation = l.m_Negation;
        };

        bool Covers(int ex, VikStd::TDataTable *tab) { // returns true if rule covers the example ex in table tab
            if (tab->Attrs[m_Attr].AttrType == VikStd::atDiscr) {
                if (m_Negation == false) return (tab->Attrs[m_Attr].iVec[ex] == m_iVal);
                return (tab->Attrs[m_Attr].iVec[ex] != m_iVal);
            } else {
                if (tab->Attrs[m_Attr].DoubleIsMissing(ex)) return false;
                if (m_Negation == false) return (tab->Attrs[m_Attr].dVec[ex] <= m_dVal);
                return (tab->Attrs[m_Attr].dVec[ex] > m_dVal);
            };
        };
    };

    ///////////////////////////////////////////////////////

    class TLiteralStr {

    public:
        VikStd::TAttrType m_Type;
        VikStd::TStr m_Attr;
        VikStd::TStr m_Val;
        VikStd::TStr m_Op;

        TLiteralStr() {
            m_Type = VikStd::atErr;
        };

        TLiteralStr(const TLiteralStr &l) {
            m_Type = l.m_Type;
            m_Attr = l.m_Attr;
            m_Val = l.m_Val;
            m_Op = l.m_Op;
        };

        void operator=(const TLiteralStr &l) {
            m_Type = l.m_Type;
            m_Attr = l.m_Attr;
            m_Val = l.m_Val;
            m_Op = l.m_Op;
        };

    };

    ///////////////////////////////////////////////////////

    class TRule {
    private:

        void Check(VikStd::TDataTable *tab) { // check if rule defines same types of attributes
            if (tab->Attrs[m_Head.m_Attr].AttrType != VikStd::atDiscr)
                throw VikStd::TExc("TRule: Check(): Target attr not discrete");
            for (int i = 0; i < m_Body.Len(); i++)
                if (tab->Attrs[m_Body[i].m_Attr].AttrType != m_Body[i].m_Type)
                    throw VikStd::TExc("TRule: Check(): Incompatible types");
        };

    public:

        VikStd::TVec<TLiteral> m_Body; // body of a rule
        TLiteral m_Head;       // head of a rule
        double m_Pos, m_Neg;   // count covered positive and negative examples
        double m_Freq;         // frequency of a rule
        double m_Conf;         // confidence of a rule

        TRule() {
            m_Pos = 0;
            m_Neg = 0;
            m_Freq = 0;
            m_Conf = 0;
        };

        TRule(const TRule &r) {
            m_Head = r.m_Head;
            m_Body = r.m_Body;
            m_Pos = r.m_Pos;
            m_Neg = r.m_Neg;
            m_Freq = r.m_Freq;
            m_Conf = r.m_Conf;
        };

        void operator=(const TRule &r) {
            Clr();
            m_Head = r.m_Head;
            m_Body = r.m_Body;
            m_Pos = r.m_Pos;
            m_Neg = r.m_Neg;
            m_Freq = r.m_Freq;
            m_Conf = r.m_Conf;
        };

        bool operator<(const TRule &r) {
            if (m_Pos < r.m_Pos) return true;
            if (m_Pos > r.m_Pos) return false;
            return (m_Neg > r.m_Neg);
        };

        bool operator>(const TRule &r) {
            if (m_Pos > r.m_Pos) return true;
            if (m_Pos < r.m_Pos) return false;
            return (m_Neg < r.m_Neg);
        };

        bool operator>=(const TRule &r) {
            return (operator<(r) == false);
        };
        bool operator<=(const TRule &r) {
            return (operator>(r) == false);
        };

        bool CoverEx(VikStd::TDataTable *tab, int i) { // checks if rule-body coveres the specified example
            bool OK = true;
            for (int j = 0; j < m_Body.Len(); j++)
                OK = OK && m_Body[j].Covers(i, tab);
            return OK;
        };

        void GetCoverage(VikStd::TDataTable *tab, VikStd::TExampleWindow *ew) { // gets coverage of a rule
            //Check(tab);
            bool OK;
            int i, j;
            m_Pos = m_Neg = 0;
            for (i = 0; i < ew->Length(); i++) {
                if (ew->IsIn(i)) {
                    OK = true;
                    for (j = 0; j < m_Body.Len(); j++) {
                        OK = OK && m_Body[j].Covers(i, tab);
                    };
                    if (OK) {
                        if (m_Head.Covers(i, tab)) {
                            m_Pos++;
                        } else {
                            m_Neg++;
                        };
                    };
                };
            };
            m_Freq = m_Pos + m_Neg;
            m_Conf = 0;
            if (m_Freq > 0) m_Conf = m_Pos / m_Freq;
        };

        void RemoveUncoveredExamples(VikStd::TDataTable *tab, VikStd::TExampleWindow *ew) { // remove uncovered examples
            //Check(tab);
            bool OK;
            int i, j;
            m_Pos = m_Neg = 0;
            for (i = 0; i < ew->Length(); i++) {
                if (ew->IsIn(i)) {
                    OK = true;
                    for (j = 0; j < m_Body.Len(); j++) {
                        OK = OK && m_Body[j].Covers(i, tab);
                    };
                    if (OK) {
                        if (m_Head.Covers(i, tab)) {
                            m_Pos++;
                        } else {
                            m_Neg++;
                        };
                    } else {
                        ew->Put(i, false);
                    };
                };
            };
        };

        void RemoveCoveredExamples(VikStd::TDataTable *tab, VikStd::TExampleWindow *ew) { // remove covered examples
            //Check(tab);
            bool OK;
            int i, j;
            m_Pos = m_Neg = 0;
            for (i = 0; i < ew->Length(); i++) {
                if (ew->IsIn(i)) {
                    OK = true;
                    for (j = 0; j < m_Body.Len(); j++) {
                        OK = OK && m_Body[j].Covers(i, tab);
                    };
                    if (OK) {
                        if (m_Head.Covers(i, tab)) {
                            m_Pos++;
                        } else {
                            m_Neg++;
                        };
                        ew->Put(i, false);
                    };
                };
            };
        };

        void Clr() {
            m_Body.Clr();
        }; // Clears body
        void Copy(const TRule &r) {
            operator=(r);
        }; // copies a rule

        void CheckConsistency(VikStd::TDataTable *tab, int ClassAttr) { // checks consistency with given datatable

            if (m_Head.m_Type != VikStd::atDiscr)
                throw VikStd::TExc("TRule: CheckConsistency(): invalid rule - rule has non-discrete target attribute");

            if (m_Head.m_Attr != ClassAttr)
                throw VikStd::TExc("TRule: CheckConsistency(): invalid rule - rule has different target attribute");

            for (int j = 0; j < m_Body.Len(); j++) {
                if ((m_Body[j].m_Attr < 0) || (m_Body[j].m_Attr >= tab->Attrs.Len()))
                    throw VikStd::TExc("TRule: CheckConsistency(): attribute variable out of bounds");

                if (m_Body[j].m_Type != tab->Attrs[m_Body[j].m_Attr].AttrType)
                    throw VikStd::TExc("TRule: CheckConsistency(): attribute type mismatch");

                if (m_Body[j].m_Type == VikStd::atDiscr) {
                    if (m_Body[j].m_iVal < 0)
                        throw VikStd::TExc("TRule: CheckConsistency(): attribute value out of bounds");

                    if (m_Body[j].m_iVal >= tab->Attrs[m_Body[j].m_Attr].ValNames.Len())
                        throw VikStd::TExc("TRule: CheckConsistency(): attribute value out of bounds");
                };
            };
        };

        VikStd::TStr GetRuleAsStr(VikStd::TDataTable *tab, bool withPosNeg = true, bool withFreqConf = true) { // returns TStr representation of a rule
            VikStd::TStr temp("FROM\n");
            for (int j = 0; j < m_Body.Len(); j++) {
                switch (m_Body[j].m_Type) {
                case VikStd::atDiscr:
                    temp += tab->Attrs[m_Body[j].m_Attr].AttrName;
                    if (m_Body[j].m_Negation) {
                        temp += "<>";
                    } else {
                        temp += "=";
                    };
                    temp += tab->Attrs[m_Body[j].m_Attr].ValNames[m_Body[j].m_iVal];
                    temp += "\n";
                    break;
                case VikStd::atCon:
                    temp += tab->Attrs[m_Body[j].m_Attr].AttrName;
                    if (m_Body[j].m_Negation) {
                        temp += ">";
                    } else {
                        temp += "<=";
                    };
                    temp += m_Body[j].m_dVal;
                    temp += "\n";
                    break;
                };
            };
            temp += "FOLLOWS\n";
            temp += tab->Attrs[m_Head.m_Attr].AttrName;
            temp += "=";
            temp += tab->Attrs[m_Head.m_Attr].ValNames[m_Head.m_iVal];
            if (withPosNeg) {
                temp += "\n%(pos=";
                temp += m_Pos;
                temp += ",neg=";
                temp += m_Neg;
                temp += ")";
            };
            if (withFreqConf) {
                temp += "\n%(freq=";
                temp += m_Freq;
                temp += ",Conf=";
                temp += m_Conf;
                temp += ")";
            };
            return temp;
        };

        VikStd::TStr GetRuleAsStrPlaintText(VikStd::TDataTable *tab) {
            VikStd::TStr temp;
            temp += m_Conf;
            temp += ' ';
            temp += m_Freq;
            temp += ' ';
            for (int j = 0; j < m_Body.Len(); j++) {
                if (j > 0) temp += " & ";

                switch (m_Body[j].m_Type) {
                case VikStd::atDiscr:
                    temp += tab->Attrs[m_Body[j].m_Attr].AttrName;
                    if (m_Body[j].m_Negation) {
                        temp += "<>";
                    } else {
                        temp += "=";
                    };
                    temp += tab->Attrs[m_Body[j].m_Attr].ValNames[m_Body[j].m_iVal];
                    break;
                case VikStd::atCon:
                    temp += tab->Attrs[m_Body[j].m_Attr].AttrName;
                    if (m_Body[j].m_Negation) {
                        temp += "<";
                    } else {
                        temp += ">=";
                    };
                    temp += m_Body[j].m_dVal;
                    break;
                };
            };
            temp += " => ";
            temp += tab->Attrs[m_Head.m_Attr].AttrName;
            temp += "=";
            temp += tab->Attrs[m_Head.m_Attr].ValNames[m_Head.m_iVal];
            return temp;
        };

        // returns TStr representation of a rule in XML format
        VikStd::TStr GetRuleAsStrXML(VikStd::TDataTable *tab, bool withPosNeg = true, bool withFreqConf = true) {
            VikStd::TStr temp("<rule>\n\t<body>\n");
            for (int j = 0; j < m_Body.Len(); j++) {
                switch (m_Body[j].m_Type) {
                case VikStd::atDiscr:
                    temp += "\t\t<att type=\"d\">";
                    temp += tab->Attrs[m_Body[j].m_Attr].AttrName;
                    if (m_Body[j].m_Negation) {
                        temp += "<neq/>";
                    } else {
                        temp += "<eq/>";
                    };
                    temp += tab->Attrs[m_Body[j].m_Attr].ValNames[m_Body[j].m_iVal];
                    temp += "</att>\n";
                    break;
                case VikStd::atCon:
                    temp += "\t\t<att type=\"c\">";
                    temp += tab->Attrs[m_Body[j].m_Attr].AttrName;
                    if (m_Body[j].m_Negation) {
                        temp += "<grt/>";
                    } else {
                        temp += "<lwreq/>";
                    };
                    temp += m_Body[j].m_dVal;
                    temp += "</att>\n";
                    break;
                };
            };
            temp += "\t</body>\n\t<att type=\"d\">";
            temp += tab->Attrs[m_Head.m_Attr].AttrName;
            temp += "<eq/>";
            temp += tab->Attrs[m_Head.m_Attr].ValNames[m_Head.m_iVal];
            temp += "</att>\n";
            if (withPosNeg) {
                temp += "\t<ex>";
                temp += m_Pos + m_Neg;
                temp += "</ex>\n";
            };
            if (withFreqConf) {
                temp += "\t<sup>";
                temp += m_Freq;
                temp += "</sup>\n\t<conf>";
                temp += m_Conf;
                temp += "</conf>\n";
            };
            temp += "</rule>\n";
            return temp;
        };

        // returns TStr representation of a rule in HTML format
        VikStd::TStr GetRuleAsStrHTML(VikStd::TDataTable *tab) {
            VikStd::TStr temp("<p><font color=blue><b>FROM</b></font> ");
            for (int j = 0; j < m_Body.Len(); j++) {
                switch (m_Body[j].m_Type) {
                case VikStd::atDiscr:
                    if (j != 0) temp += ", ";
                    temp += tab->Attrs[m_Body[j].m_Attr].AttrName;
                    if (m_Body[j].m_Negation == false) {
                        temp += "=";
                    } else {
                        temp += "<>";
                    };
                    temp += tab->Attrs[m_Body[j].m_Attr].ValNames[m_Body[j].m_iVal];
                    break;
                case VikStd::atCon:
                    if (j != 0) temp += ", ";
                    temp += tab->Attrs[m_Body[j].m_Attr].AttrName;
                    if (m_Body[j].m_Negation == false) {
                        temp += "<=";
                    } else {
                        temp += ">";
                    };
                    temp += m_Body[j].m_dVal;
                    break;
                };
            };
            temp += "<br><font color=blue><b>FOLLOWS</b></font> ";
            temp += tab->Attrs[m_Head.m_Attr].AttrName;
            temp += "=";
            temp += tab->Attrs[m_Head.m_Attr].ValNames[m_Head.m_iVal];
            temp += "</p>";
            return temp;
        };
    };

    ///////////////////////////////////////////////////////

    class TRuleStr {

    public:

        VikStd::TVec<TLiteralStr> m_Body; // body of a rule
        TLiteralStr m_Head;       // head of a rule

        TRuleStr() {};

        TRuleStr(const TRuleStr &r) {
            for (int i = 0; i < r.m_Body.Len(); i++)
                m_Body.Add(r.m_Body[i]);
            m_Head = r.m_Head;
        };

        void operator=(const TRuleStr &r) {
            Clr();
            m_Head = r.m_Head;
            for (int i = 0; i < r.m_Body.Len(); i++)
                m_Body.Add(r.m_Body[i]);
        };

        void Clr() {
            m_Body.Clr();
        }; // clears body

        void Copy(const TRuleStr &r) {
            operator=(r);
        }; // copies a rule


        bool Compatible(VikStd::TDataTable &tab) { // is rule compatible with this datatable - target attribute must also exist
            VikStd::TStr err = "TRuleStr::Compatible: rule is not compatible with data - ";
            for (int i = 0; i < m_Body.Len(); i++) {
                int j = tab.FindAttrWithName(m_Body[i].m_Attr);
                if (j < 0) {
                    err += "attribute ";
                    err += m_Body[i].m_Attr;
                    err += " not defined.";
                    throw VikStd::TExc(err);
                };
                if (tab.Attrs[j].AttrType != m_Body[i].m_Type) {
                    err += "type of attribute ";
                    err += m_Body[i].m_Attr;
                    err += " does't match.";
                    throw VikStd::TExc(err);
                };
                if (tab.Attrs[j].AttrType == VikStd::atDiscr) {
                    if (tab.Attrs[j].ValNamesFind(m_Body[i].m_Val) < 0) {
                        err += "value ";
                        err += m_Body[i].m_Val;
                        err += " not defined.";
                        throw VikStd::TExc(err);
                    };
                    if ((m_Body[i].m_Op != "EQ/") && (m_Body[i].m_Op != "NEQ/")) {
                        err += "illegal operator: ";
                        err += m_Body[i].m_Op;
                        throw VikStd::TExc(err);
                    };
                } else if (tab.Attrs[j].AttrType == VikStd::atCon) {
                    ToFloat(m_Body[i].m_Val);
                    if ((m_Body[i].m_Op != "LWREQ/") && (m_Body[i].m_Op != "GRT/")) {
                        err += "illegal operator: ";
                        err += m_Body[i].m_Op;
                        throw VikStd::TExc(err);
                    };
                };
            };
            int j = tab.FindAttrWithName(m_Head.m_Attr);
            if (j < 0) return false;
            if (tab.Attrs[j].AttrType != m_Head.m_Type) {
                err += "type of attribute ";
                err += m_Head.m_Attr;
                err += " does't match.";
                throw VikStd::TExc(err);
            };
            if (tab.Attrs[j].AttrType == VikStd::atDiscr) {
                if (tab.Attrs[j].ValNamesFind(m_Head.m_Val) < 0) { // check value
                    err += "value ";
                    err += m_Head.m_Val;
                    err += " not defined.";
                    throw VikStd::TExc(err);
                };
                if ((m_Head.m_Op != "EQ/") && (m_Head.m_Op != "NEQ/")) { // check operator
                    err += "illegal operator: ";
                    err += m_Head.m_Op;
                    throw VikStd::TExc(err);
                };
            } else if (tab.Attrs[j].AttrType == VikStd::atCon) {
                ToFloat(m_Head.m_Val);
                if ((m_Head.m_Op != "LWREQ/") && (m_Head.m_Op != "GRT/")) { // check operator
                    err += "illegal operator: ";
                    err += m_Head.m_Op;
                    throw VikStd::TExc(err);
                };
            };
            return true;
        };

        ///////////////////

        bool Compatible2(VikStd::TDataTable &tab) { // is rule compatible with this datatable - target attribute must NOT exist
            VikStd::TStr err = "TRuleStr::Compatible: rule is not compatible with data - ";
            for (int i = 0; i < m_Body.Len(); i++) {
                int j = tab.FindAttrWithName(m_Body[i].m_Attr);
                if (j < 0) {
                    err += "attribute ";
                    err += m_Body[i].m_Attr;
                    err += " not defined.";
                    throw VikStd::TExc(err);
                };
                if (tab.Attrs[j].AttrType != m_Body[i].m_Type) {
                    err += "type of attribute ";
                    err += m_Body[i].m_Attr;
                    err += " does't match.";
                    throw VikStd::TExc(err);
                };
                if (tab.Attrs[j].AttrType == VikStd::atDiscr) {
                    /*
                    // not very nice - will hang if value is not defined

                    if (tab.Attrs[j].ValNamesFind(m_Body[i].m_Val)<0){
                        err+="value ";  err+=m_Body[i].m_Val;
                        err+=" of attribute "; err+=tab.Attrs[j].AttrName; err+=" not defined.";
                        throw TExc(err);
                    };
                    */
                    if ((m_Body[i].m_Op != "EQ/") && (m_Body[i].m_Op != "NEQ/")) {
                        err += "illegal operator: ";
                        err += m_Body[i].m_Op;
                        throw VikStd::TExc(err);
                    };
                } else if (tab.Attrs[j].AttrType == VikStd::atCon) {
                    ToFloat(m_Body[i].m_Val);
                    if ((m_Body[i].m_Op != "LWREQ/") && (m_Body[i].m_Op != "GRT/")) {
                        err += "illegal operator: ";
                        err += m_Body[i].m_Op;
                        throw VikStd::TExc(err);
                    };
                };
            };
            return true;
        };

        //////////////

        TRule CreateTRule(VikStd::TDataTable &tab) { // create a TRule object - to be used only after check with Compatible()
            TRule rule;
            for (int i = 0; i < m_Body.Len(); i++) {
                TLiteral lit;
                lit.m_Type = m_Body[i].m_Type;
                lit.m_Attr = tab.FindAttrWithName(m_Body[i].m_Attr);
                if (tab.Attrs[lit.m_Attr].AttrType == VikStd::atDiscr) {
                    lit.m_iVal = tab.Attrs[lit.m_Attr].ValNamesFind(m_Body[i].m_Val);
                } else {
                    lit.m_dVal = ToFloat(m_Body[i].m_Val);
                };
                if ((m_Body[i].m_Op == "EQ/") || (m_Body[i].m_Op == "LWREQ/")) {
                    lit.m_Negation = false;
                } else {
                    lit.m_Negation = true;
                };
                rule.m_Body.Add(lit);
            };

            rule.m_Head.m_Type = m_Head.m_Type;
            rule.m_Head.m_Attr = tab.FindAttrWithName(m_Head.m_Attr);
            if (tab.Attrs[rule.m_Head.m_Attr].AttrType == VikStd::atDiscr) {
                rule.m_Head.m_iVal = tab.Attrs[rule.m_Head.m_Attr].ValNamesFind(m_Head.m_Val);
            } else {
                rule.m_Head.m_dVal = ToFloat(m_Head.m_Val);
            };
            if ((m_Head.m_Op == "EQ/") || (m_Head.m_Op == "LWREQ/")) {
                rule.m_Head.m_Negation = false;
            } else {
                rule.m_Head.m_Negation = true;
            };
            return rule;
        };

        //////////////
        // create a TRule object - to be used only after check with Compatible()
        // this version is used when rules are used to create a new attribute

        TRule CreateTRule2(VikStd::TDataTable &tab, VikStd::TVec<VikStd::TStr> &Targets) {
            TRule rule;
            for (int i = 0; i < m_Body.Len(); i++) {
                TLiteral lit;
                lit.m_Type = m_Body[i].m_Type;
                lit.m_Attr = tab.FindAttrWithName(m_Body[i].m_Attr);
                if (tab.Attrs[lit.m_Attr].AttrType == VikStd::atDiscr) {
                    lit.m_iVal = tab.Attrs[lit.m_Attr].ValNamesFind(m_Body[i].m_Val);
                    if (lit.m_iVal < 0) lit.m_iVal = -2; // value is not defined
                } else {
                    lit.m_dVal = ToFloat(m_Body[i].m_Val);
                };
                if ((m_Body[i].m_Op == "EQ/") || (m_Body[i].m_Op == "LWREQ/")) {
                    lit.m_Negation = false;
                } else {
                    lit.m_Negation = true;
                };
                rule.m_Body.Add(lit);
            };

            rule.m_Head.m_Type = m_Head.m_Type;
            // we assume discrete attribute will be created
            for (int j = 0; j < Targets.Len(); j++)
                if (m_Head.m_Val == Targets[j]) rule.m_Head.m_iVal = j;

            if ((m_Head.m_Op == "EQ/") || (m_Head.m_Op == "LWREQ/")) {
                rule.m_Head.m_Negation = false;
            } else {
                rule.m_Head.m_Negation = true;
            };
            return rule;
        };
    };

    ///////////////

}
