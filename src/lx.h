#pragma once

////////////////////////////////////////////////////////////////////

#include "ds.h"
#include "str.h"

#include <string>
#include <stdio.h>
#include <assert.h>

namespace VikStd {

	////////////////////////////////////////////////////////////////////
	// Parser //////////////////////////////////////////////////////////
	// Main part of unit kindly programed by Marko Grobelnik ///////////
	////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////
	// Lexical-Symbols

	typedef enum {
		syUndef, syLn, syInt, syFlt, syStr, syIdStr, syQStr,
		syPeriod, syDPeriod,
		syComma, syColon, sySemicolon,
		syPlus, syMinus, syAsterisk, sySlash, syPercent,
		syExclamation,    // '!'
		syVBar,           // '|'
		syAmpersand,      // '&'
		syQuestion,       // '?'
		syEq, syNEq, syLwr, syGtr, syLEq, syGEq,
		syLParen, syRParen,        // '(', ')'
		syLBracket, syRBracket,    // '[', ']'
		syLBrace, syRBrace,        // '{', '}'
		syUScore,                  // '_'
		syEoln,
		syEof, syErr
	} TLxSym;

	const char VikEofCh = 26;

	/////////////////////////////////////////////////////////////////////
	//  delimiters

	class TDelims {

			bool m_tab[256];
			TLxSym m_sym[256];

		public:

			TDelims() {
				for (int i = 0; i < 256; i++) {
					m_tab[i] = false;
					m_sym[i] = syUndef;
				};
				m_sym[int('.')] = syPeriod;
				m_sym[int(',')] = syComma;
				m_sym[int(':')] = syColon;
				m_sym[int(';')] = sySemicolon;

				m_sym[int('+')] = syPlus;
				m_sym[int('-')] = syMinus;
				m_sym[int('*')] = syAsterisk;
				m_sym[int('/')] = sySlash;
				m_sym[int('%')] = syPercent;

				m_sym[int('!')] = syExclamation;
				m_sym[int('|')] = syVBar;
				m_sym[int('&')] = syAmpersand;
				m_sym[int('?')] = syQuestion;
				m_sym[int('_')] = syUScore;

				m_sym[int('=')] = syEq;
				m_sym[int('>')] = syGtr;
				m_sym[int('<')] = syLwr;
				m_sym[int('(')] = syLParen;
				m_sym[int(')')] = syRParen;
				m_sym[int('[')] = syLBracket;
				m_sym[int(']')] = syRBracket;
				m_sym[int('{')] = syLBrace;
				m_sym[int('}')] = syRBrace;
			};

			void Add(char *str) {
				for (unsigned int i = 0; i < strlen(str); i++)
					m_tab[(int)str[i]] = true;
			};
			bool IsIn(char ch) {
				return m_tab[ch];
			};
			TLxSym GetSymOfChar(char ch) {
				return m_sym[ch];
			};
			void Clear() {
				for (int i = 0; i < 256; i++) m_tab[i] = false;
			};
	};

	////////////////////////////////////////////////////////////////////
	// TLx - lexical analiser for parsing from file
	////////////////////////////////////////////////////////////////////

	class TLx {
		private:
			FILE* SIn;

			char PCh, Ch;

			char GetCh() {
				assert(Ch != VikEofCh);
				PCh = Ch;
				LnChN++;
				ChN++;
				Ch = (char)getc(SIn);
				if (Ch == EOF) {
					Ch = VikEofCh;
				};
				if (Ch == '\n') {
					LnN++;
					LnChN = 1;
				};
				return Ch;
			};

		public:
			int LnN, LnChN, ChN;
			TLxSym Sym;
			TStr Str;
			int Int;
			double Flt;
			int SymLnN, SymLnChN, SymChN;
			TDelims m_delims;
			char m_CommentChar;

			TLx(FILE* _SIn):
				PCh(' '), Ch(' '), LnN(1), LnChN(0), ChN(0),
				Sym(syUndef), Str(), Int(0), Flt(0),
				SymLnN(-1), SymLnChN(-1), SymChN(-1), m_CommentChar('%') {
				SIn = _SIn;
			};

			char GetNextCh() const {
				return Ch;
			}; // peek next character

			TStr GetPosition() const { // return string describing the position of lx in th file
				TStr s = "Line ";
				s += LnN;
				s += ", character ";
				s += LnChN;
				return s;
			};

			// main function - gets the new symbol

			TLxSym GetSym(bool intDouble = false, bool NoNum = false) {

				while (isspace(Ch) || (Ch == '\n') || (Ch == '\r') || (Ch == '\t')) {
					GetCh();
				}
				SymLnN = LnN;
				SymLnChN = LnChN;
				SymChN = ChN;

				// First check for integers or floats //////////////////////
				if ((NoNum == false) && ((isdigit(Ch)) || (Ch == '+') || (Ch == '-'))) {
					Str = "";
					do {
						Str += Ch;
					} while (isdigit(GetCh()));

					if (intDouble) {
						if (Ch == '.') {
							Str += Ch;
							while (isdigit(GetCh())) Str += Ch;
						};

						if ( (Ch == 'e') || (Ch == 'E')) {
							Str += Ch;
							GetCh();
							if ((Ch == '+') || (Ch == '-')) {
								Str += Ch;
								GetCh();
							};
							while (isdigit(Ch)) {
								Str += Ch;
								GetCh();
							};
						};
						Sym = syFlt;
						Flt = atof(Str.c_str());
					} else {
						Sym = syInt;
						Int = atoi(Str.c_str());
					};
				}
				// If comment sign is found, skipp until end of line or file
				else if (Ch == m_CommentChar) {
					do {
						GetCh();
					} while ((Ch != '\n') && (Ch != '\r') && (Ch != VikEofCh));
					if (Ch == '\n') {
						GetCh();
					};
					if (Ch == '\r') {
						GetCh();
					};
					GetSym(intDouble);
				} else if (m_delims.IsIn(Ch)) {
					Str = "";
					Str += Ch;
					Sym = m_delims.GetSymOfChar(Ch);
					GetCh();
				} else if (Ch == VikEofCh) {
					Str = "";
					Sym = syEof;
				}

				// Parse string ////////////////////////////////////////////

				else if (Ch == '\"') { // parse quoted string
					Str = "";
					Sym = syStr;
					GetCh();
					while ( (Ch != VikEofCh) && (Ch != '\"')) {
						Str += Ch;
						GetCh();
					}
					if (Ch != '\"') Sym = syErr;
				} else { // parse non-quoted string
					Str = "";
					Sym = syStr;
					do {
						Str += Ch;
						GetCh();
					} while ( (Ch != VikEofCh) &&
							  (!isspace(Ch)) &&
							  (Ch != '\n') &&
							  (Ch != '\r') &&
							  (m_delims.IsIn(Ch) == false) &&
							  (m_CommentChar != Ch)
							);
				};
				return Sym;
			}; // end of GetSym(..)

	};
	typedef TLx* PLx;

	////////////////////////////////////////////////////////////////////
	// TLxStr - lexical analiser for parsing from string
	////////////////////////////////////////////////////////////////////

	class TLxStr {
		private:
			TStr SIn;
			int m_Pos;

			char PCh, Ch;

			char GetCh() {
				assert(Ch != VikEofCh);
				PCh = Ch;
				LnChN++;
				ChN++;
				if (m_Pos == SIn.Len()) {
					Ch = VikEofCh;
				} else {
					Ch = SIn[m_Pos];
					m_Pos++;
				};
				if (Ch == '\n') {
					LnN++;
					LnChN = 1;
				};
				return Ch;
			};

		public:

			int LnN, LnChN, ChN;
			TLxSym Sym;
			TStr Str;
			int Int;
			double Flt;
			int SymLnN, SymLnChN, SymChN;
			TDelims m_delims;
			char m_CommentChar;

			TLxStr(TStr _SIn):
				PCh(' '), Ch(' '), LnN(1), LnChN(0), ChN(0),
				Sym(syUndef), Str(), Int(0), Flt(0),
				SymLnN(-1), SymLnChN(-1), SymChN(-1), m_CommentChar('%'), m_Pos(0) {
				SIn = _SIn;
			};

			char GetNextCh() const {
				return Ch;
			}; // peek next character

			TStr GetPosition() const { // return string describing the position of lx in th file
				TStr s = "Line ";
				s += LnN;
				s += ", character ";
				s += LnChN;
				return s;
			};

			// main function - gets the new symbol

			TLxSym GetSym(bool intDouble = false) {

				while (isspace(Ch) || (Ch == '\n') || (Ch == '\r') || (Ch == '\t')) {
					GetCh();
				}
				SymLnN = LnN;
				SymLnChN = LnChN;
				SymChN = ChN;

				// First check for integers or floats //////////////////////
				if ((isdigit(Ch)) || (Ch == '+') || (Ch == '-')) {
					Str = "";
					do {
						Str += Ch;
					} while (isdigit(GetCh()));

					if (intDouble) {
						if (Ch == '.') {
							Str += Ch;
							while (isdigit(GetCh())) Str += Ch;
						};

						if ( (Ch == 'e') || (Ch == 'E')) {
							Str += Ch;
							GetCh();
							if ((Ch == '+') || (Ch == '-')) {
								Str += Ch;
								GetCh();
							};
							while (isdigit(Ch)) {
								Str += Ch;
								GetCh();
							};
						};
						Sym = syFlt;
						Flt = atof(Str.c_str());
					} else {
						Sym = syInt;
						Int = atoi(Str.c_str());
					};
				}
				// If comment sign is found, skipp until end of line or file
				else if (Ch == m_CommentChar) {
					do {
						GetCh();
					} while ((Ch != '\n') && (Ch != '\r') && (Ch != VikEofCh));
					if (Ch == '\n') {
						GetCh();
					};
					if (Ch == '\r') {
						GetCh();
					};
					GetSym(intDouble);
				} else if (m_delims.IsIn(Ch)) {
					Str = "";
					Str += Ch;
					Sym = m_delims.GetSymOfChar(Ch);
					GetCh();
				} else if (Ch == VikEofCh) {
					Str = "";
					Sym = syEof;
				}

				// Parse string ////////////////////////////////////////////

				else if (Ch == '\"') { // parse quoted string
					Str = "";
					Sym = syStr;
					GetCh();
					while ( (Ch != VikEofCh) && (Ch != '\"')) {
						Str += Ch;
						GetCh();
					}
					if (Ch != '\"') Sym = syErr;
				} else { // parse non-quoted string
					Str = "";
					Sym = syStr;
					do {
						Str += Ch;
						GetCh();
					} while ( (Ch != VikEofCh) &&
							  (!isspace(Ch)) &&
							  (Ch != '\n') &&
							  (Ch != '\r') &&
							  (m_delims.IsIn(Ch) == false) &&
							  (m_CommentChar != Ch)
							);
				};
				return Sym;
			}; // end of GetSym(..)

	};

	///////////////////////////////////////////////////////
	// TXMLParser - class for parsing XML from file
	///////////////////////////////////////////////////////

	class TXMLParser {
			FILE *fp;
			char Ch;

			class TEOFExc: public TExc {};

			void GetCh() {
				Ch = getc(fp);
				if (Ch == VikEofCh) throw TExc("TXMLParser: ReadToNextTag: Unexpected end of file");
				if (Ch == -1) throw TExc("TXMLParser: ReadToNextTag: Unexpected end of file");
				if ((Ch == 10) || (Ch == 13)) {
					LastLine = "";
				} else LastLine += Ch;
			};

			void GetParameter() {
				while (isspace(Ch) || (Ch == '\n') || (Ch == '\r') || (Ch == '\t')) GetCh();
				TStr p, v;
				while ((isspace(Ch) || (Ch == '\n') || (Ch == '\r') || (Ch == '\t') || (Ch == '=') || (Ch == '>')) == false) {
					p += Ch;
					GetCh();
				};
				p.ToUpper();
				Params.Add(p);
				if (Ch != '=') {
					Values.Add(v);
					return;
				};
				GetCh();
				if (Ch == '\"') {
					GetCh();
					while (Ch != '\"') {
						v += Ch;
						GetCh();
					};
					GetCh();
					v.ToUpper();
					if (v.Len() > 2) { // remove starting and ending quote - if they exist
						if ((v.CharAt(0) == '\"') && (v.CharAt(v.Len() - 1) == '\"')) {
							TStr a = v.Mid(1, v.Len() - 2);
							v = a;
						};
					};
				} else {
					while ((isspace(Ch) || (Ch == '\n') || (Ch == '\r') || (Ch == '\t') || (Ch == '>')) == false) {
						v += Ch;
						GetCh();
					};
					v.ToUpper();
				};
				Values.Add(v);
			};

		public:

			TStr LastTag; // name of last tag
			TStr LastText; // text before last tag
			TStr LastLine; // text of last line
			TVec<TStr> Params; // names of parameters
			TVec<TStr> Values; // parameter's values
			bool TagIsClosing; // does last tag begin with '/'

			TXMLParser(FILE *_fp) {
				fp = _fp;
			};

			void ReadToNextTag() { // read file until and with next tag
				LastText = "";
				GetCh();
				while (Ch != '<') {
					LastText += Ch;
					GetCh();
				};
				GetCh();
				TagIsClosing = false;
				if (Ch == '/') {
					TagIsClosing = true;
					GetCh();
				};
				while (isspace(Ch) || (Ch == '\n') || (Ch == '\r') || (Ch == '\t')) GetCh();
				LastTag = "";
				while ((Ch != ' ') && (Ch != '>')) {
					LastTag += Ch;
					GetCh();
				};
				LastTag.ToUpper();
				Params.Clear();
				Values.Clear();
				while (Ch != '>') GetParameter();
			};

			// first tag must already be read
			// read pair of <tag>...</tag> - exception if there is another tag inbetween
			// text preceeding opening tag is descarded
			// params of closing tag are discarded
			// if first tag is closing, exception is thrown
			void ReadWholePair() {
				if (TagIsClosing != false) {
					TStr s = "TXMLParser: ReadWholePair: first tag is closing. Tag: ";
					s += LastTag;
					throw TExc(s);
				};
				LastText = "";
				GetCh();
				while (Ch != '<') {
					LastText += Ch;
					GetCh();
				};
				GetCh();
				if (Ch != '/') TExc("TXMLParser: ReadWholePair: second tag is not closing.");
				while (isspace(Ch) || (Ch == '\n') || (Ch == '\r') || (Ch == '\t')) GetCh();
				TStr XLastTag = "";
				while ((Ch != ' ') && (Ch != '>')) {
					XLastTag += Ch;
					GetCh();
				};
				XLastTag.ToUpper();
				while (Ch != '>') GetCh();
				if (XLastTag != LastTag) TExc("TXMLParser: ReadWholePair: first and second tag don't match.");
			};

			int FindParameter(TStr &s) {
				for (int i = 0; i < Params.Len(); i++)
					if (Params[i] == s) return i;
				return -1;
			};

			int FindParameter(char *s) {
				for (int i = 0; i < Params.Len(); i++)
					if (Params[i] == s) return i;
				return -1;
			};

			// skip to tag that marks beggining of Castaneda script
			bool ReadToCastanedaTag() {

				try {
					bool go = true;
					while (go) {
						ReadToNextTag();
						if ((LastTag == "SCRIPT") && (TagIsClosing == false)) {
							int i = FindParameter("LANGUAGE");
							if (i >= 0) {
								TStr s = Values[i];
								if ((s == "CASTANEDA") || (s == "\"CASTANEDA\"")) go = false;
							};
						};
					};
				} catch(TEOFExc) {
					throw TExc("Illegal input file, cannot find Castaneda script");
				};

				try {
					GetCh();
					while (Ch != '<') GetCh();
					GetCh();
					if (Ch != '!') throw TExc("Invalid CASTANEDA script. <! excpected at the beggining.");

					ReadToNextTag();
					return true;
				} catch(TEOFExc) {
					throw TExc("Illegal input file, unexpected end of file after Castaneda script");
				};
			};

	};

	/////////////////////////////////////////////////////////////////
	//
	/////////////////////////////////////////////////////////////////

	class TTag {
		public:

			TTag() {
				m_Name = "";
			};
			~TTag() {};

		private:
			TStr m_Name;
		public:
			TVec<TStr> m_Params;
			TStr GetName() {
				return m_Name.GetUpperCase();
			}; // tag name (returns uppercase)
			TStr GetText() { // this is all the stuff between "<" and ">"
				TStr result = m_Name;
				for (int i = 0; i < m_Params.Len(); i++) {
					result += ' ';
					result += m_Params[i];
				};
				return result;
			};
			void SetName(TStr &NewName) {
				m_Name = NewName;
			}; // tag name (returns uppercase)
			void SetText(TStr &text) { // this is all the stuff between "<" and ">"
				int i, k;
				int len;
				bool q1, q2;

				q1 = false;
				q2 = false;
				len = text.Len();

				// getting name
				i = 0;
				while (((i == len) || (text[i] == ' ')) == false) i++;
				m_Name = text.Left(i);

				k = i + 1;
				i = k;
				m_Params.Clear();

				// getting parameters
				while (i < len) {
					if ((text[i] == '\'') || (text[i] == '\"')) {
						if (text[i] == '\"') {
							if (q1 == false) {
								q2 = !q2;
							};
						} else {
							if (q2 == false) {
								q1 = !q1;
							};
						};
						if ((q1 || q2) == false) {
							TStr x = text.Mid(k, i - k + 1);
							x.TrimLeftRight();
							m_Params.Add(x);
							k = i + 1;
						};
					} else if ((text[i] == ' ') && ((q1 || q2) == false)) {
						TStr x = text.Mid(k, i - k + 1);
						x.TrimLeftRight();
						m_Params.Add(x);
						k = i + 1;
					};
					i++;
				};
				if (k < i) {
					TStr x = text.Mid(k, i - k + 1);
					x.TrimLeftRight();
					m_Params.Add(x);
					k = i + 1;
				};
			};
	};



	/////////////////////////////////////

	inline bool XFindNext(TStr &text, char ch, int startfrom, int &pos)
	{
		pos = startfrom;
		while ((pos < text.Len()) && (text[pos] != ch)) pos++;
		if (pos >= text.Len()) return false;
		return (text[pos] == ch);
	};

	inline bool XFindPrev(TStr &text, char ch, int startfrom, int &pos)
	{
		pos = startfrom;
		while ((pos >= 0) && (text[pos] != ch)) pos--;
		if (pos < 0) return false;
		return (text[pos] == ch);
	};

	class THTMLParser {
		public:

			THTMLParser() {
				SetText("");
			};

		private:

			TStr m_Text;
			TStr m_TextBetween;
			TTag m_Tag;
			int m_Pos;               // current position in Text
			int m_TagPos, m_TagLen;  // Tag position and length (including brackets)
			int m_TBPos;             // TextBetween position

			/*  void TagChanged(){
			        TStr s;

			        if (m_TagPos==0) return;

			        Delete(fText, fTagPos+1, fTagLen-2);
			        s:= fTag.Text;
			        if (fTBPos>fTagPos) then Inc(fTBPos, Length(s)+2-fTagLen);
			        fTagLen:= Length(s)+2;
			        Insert(s, fText, fTagPos+1);
			    };*/
			void ClearTag() {
				TStr s ("");
				SetTagText(s);
				m_TagPos = -1;
				m_TagLen = 0;
			};

			void ClearTB() {
				m_TextBetween = "";
				m_TBPos = 0;
			};

			void CheckPos() {
				if (m_Pos < 0) m_Pos = 0;
				else if (m_Pos >= m_Text.Len()) m_Pos = m_Text.Len() - 1;
			};

			void SetTagText(TStr &text) {
				m_Tag.SetText(text);
			};

			bool FindTag(bool next) {
				int tag1, tag2,  // first/last char of the new tag
					tb1, tb2;        // first/last char of new TextBetween
				bool Result;


				if (m_Text.Len() == 0) {
					return false;
				};
				if (m_TagPos >= 0) {
					if (next == true) {
						m_Pos++;
					} else {
						m_Pos--;
					};
				};

				CheckPos();

				if (next == true) {
					// find next tag
					Result = XFindNext(m_Text, '<', m_Pos, tag1) && XFindNext(m_Text, '>', tag1, tag2);
					// find end of current tag
					if (((XFindNext(m_Text, '>', m_Pos, tb1) == true) && (tb1 < tag1)) == true) {
						tb1 = tb1 + 1;
					} else {
						tb1 = m_Pos;
					};
					tb2 = 0; //this is just to get rid of a stupid warning
				} else {
					tb2 = m_Pos;
					// find previous tag
					Result = XFindPrev(m_Text, '>', tb2, tag2) && XFindPrev(m_Text, '<', tag2, tag1);
				};

				if (Result == true) {
					m_Pos = tag1;
					if (next == true) {
						tb2 = tag1 - 1;
					} else {
						tb1 = tag2 + 1;
					};
				} else {
					if (next == true) {
						m_Pos = m_Text.Len();
						tb2 = m_Text.Len();
					} else {
						m_Pos = 1;
						tb1 = 1;
					};
					tag1 = 0;
					tag2 = 0;
				};

				m_TagPos = tag1;
				m_TagLen = tag2 - tag1 + 1;
				TStr ts = m_Text.Mid(m_TagPos + 1, m_TagLen - 2);
				SetTagText(ts);
				m_TBPos = tb1;
				m_TextBetween = m_Text.Mid(m_TBPos, tb2 - tb1 + 1);
				return Result;
			};

		public:

			// current tag
			TTag GetTag() {
				if (m_TagPos < 0) throw TExc("THTMLParser::GetTag: m_TagPos==0.");
				return m_Tag;
			};
			void SetTag(TTag tag);

			// all the HTML file
			TStr GetText();
			void SetText(TStr &NewText) {
				m_Text = NewText;
				GotoBeginning();
			};
			void SetText(char* NewText) {
				TStr s(NewText);
				SetText(s);
			};

			// this is the text between two tags:
			// - the last one - before calling NextTag/PrevTag
			// - and the new (current) one
			TStr GetTextBetween() {
				return m_TextBetween;
			};

			//void SetTextBetween(TStr &text);

			//void RemoveTag;               // remove the current tag
			/*void InsertTag(TTag NewTag){  // insert a new tag BEFORE the current one
			    CheckPos();
			    Insert("<"+NewTag.Text+">", fText, fPos);
			    NextTag();
			};
			*/
			//void InsertText(TStr text);   // insert some text before the current tag
			bool NextTag() {
				return FindTag(true);
			};  // find next tag from current pos.
			bool NextTag(bool ExceptionIfEOF) {  // find next tag from current pos. - throw TExc if EOF
				bool res = FindTag(true);
				if ((ExceptionIfEOF == true) && (res == false))
					throw TExc("THTMLParser::NextTag: unexpected end of file.");
				return res;
			};
			bool PrevTag() {
				return FindTag(false);
			}; // find previous tag from current pos.
			void GotoBeginning() {
				m_Pos = 0;
				ClearTag();
				ClearTB();
			};
			void GotoEnd() {
				m_Pos = m_Text.Len();
				ClearTag();
				ClearTB();
			};

			void LoadFromFile(TStr &filename) {
				TStr file;
				FILE *fp = fopen(filename.c_str(), "r");
				if (fp == NULL)
					throw TExc(TStr("Cannot open input file: ") + filename);

				// read file
				try {
					char bf[1001];
					int read = fread(bf, 1, 1000, fp);
					bf[read] = '\0';
					file += bf;
					while (!feof(fp)) {
						read = fread(bf, 1, 1000, fp);
						bf[read] = '\0';
						file += bf;
					};
				} catch(...) {
					fclose(fp);
					throw;
				};
				SetText(file);
			};

            void SaveToFile(char* filename) {
                FILE *fp = fopen(filename, "w");
				if (fp == NULL) throw TExc("THTMLParser::SaveToFile: Cannot open output file");
				fprintf(fp, "%s", m_Text.CStr());
				fclose(fp);
            };

			void SaveToFile(TStr &filename) {
				SaveToFile(filename.CStr());
			};

			bool FindTagFromBeginning(TStr TagName) { // scan from beginning - true, if tag was found
				GotoBeginning();
				while (NextTag()) {
					if (GetTag().GetName() == TagName) return true;
				};
				return false;
			};
			bool FindTagFromBeginning(char *TagName) {
				return FindTagFromBeginning(TStr(TagName));
			}; // scan from beginning - true, if tag was found

	};
}
