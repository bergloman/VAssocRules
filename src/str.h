#pragma once

/////////////////////////////////////////////////////////////
//
// Module: implementation of TStr
// Author: Viktor Jovanoski
// Desc:   implements class TStr, which is basically the same
//         as CString in VisualC++ or string in STL
//         it also implements some utility classes like
//           TExc - custom exceptions
//           TOut - output (and its derivations)
//
/////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h> // for sprintf

#define MIN(X,Y) ((X>Y) ? (Y) : (X))

namespace VikStd {

	////////////////////////////////////////////////////////////
	// Representation of a string //////////////////////////////

	class TStrRep {
		public:
			char* m_Buff;       // Pointer to elements
			int m_Size;     // Buffer length
			int m_RefCount; // Reference count

			TStrRep(int NewSize, const char* p) {
				m_RefCount = 1;
				m_Size = NewSize;
				m_Buff = new char[m_Size + 1];
				strcpy(m_Buff, p);
			};

			~TStrRep() {
				delete [] m_Buff;
			}; // delete string

			TStrRep* GetOwnCopy() { // clone if necessary
				if (m_RefCount == 1)
					return this;
				m_RefCount--;
				return new TStrRep(m_Size, m_Buff);
			};

			void Assign(int NewSize, const char* p) {
				if (m_Size != NewSize) {
					delete [] m_Buff;
					m_Size = NewSize;
					m_Buff = new char[m_Size + 1];
				};
				strcpy(m_Buff, p);
			};

	};

	/////////////////////////////////////////////////////////////
	// String class /////////////////////////////////////////////
	/////////////////////////////////////////////////////////////

	class TStr {
		private:

			TStrRep *m_rep;

		public:

			TStr() {
				m_rep = new TStrRep(0, "");
			};

			TStr(const char* s) {
				m_rep = new TStrRep(strlen(s), s);
			};

			TStr(const TStr& x) {
				x.m_rep->m_RefCount++;
				m_rep = x.m_rep;
			}; // Create new reference to existing TStrRep

			TStr(const bool& b, bool bit_TrueFalse = false) {
				if (b) {
					if (bit_TrueFalse == false) {
						m_rep = new TStrRep(1, "1");
					} else {
						m_rep = new TStrRep(4, "true");
					};
				} else {
					if (bit_TrueFalse == false) {
						m_rep = new TStrRep(1, "0");
					} else {
						m_rep = new TStrRep(5, "false");
					};
				};
			};

			TStr(const int& i) {
				char t[33];
				sprintf(t, "%d", i);
				m_rep = new TStrRep(strlen(t), t);
			};

			TStr(const char& c) {
				char t[33];
				sprintf(t, "%c", c);
				m_rep = new TStrRep(strlen(t), t);
			};

			TStr(const double& d) {
				char t[33];
				sprintf(t, "%f", d);
				m_rep = new TStrRep(strlen(t), t);
			};

			// = operators

			void operator=(const TStr& x) {
				x.m_rep->m_RefCount++;
				if (--m_rep->m_RefCount == 0)
					delete m_rep;
				m_rep = x.m_rep;
			};

			void operator=(const char *s) {
				if (m_rep->m_RefCount == 1) {
					m_rep->Assign(strlen(s), s);
				} else {
					m_rep->m_RefCount--;
					m_rep = new TStrRep(strlen(s), s);
				};
			};

			void operator=(const bool& b) {
				TStr temp(b);
				operator=(temp);
			};

			void operator=(const int& i) {
				TStr temp(i);
				operator=(temp);
			};

			void operator=(const char& c) {
				TStr temp(c);
				operator=(temp);
			};

			void operator=(const double& d) {
				TStr temp(d);
				operator=(temp);
			};

			// += operators

			void operator+=(const char* s) {
				int NewLen = Len() + strlen(s);
				char *old, *temp;
				old = CStr();
				temp = new char[NewLen + 1];
				strcpy(temp, old);
				strcat(temp, s);
				operator=(temp);
				delete [] temp;
			};

			void operator+=(const TStr& s) {
				operator+=(s.CStr());
			};

			void operator+=(const bool& b) {
				TStr temp(b);
				operator+=(temp);
			};

			void operator+=(const int& i) {
				TStr temp(i);
				operator+=(temp);
			};

			void operator+=(const char& c) {
				TStr temp(c);
				operator+=(temp);
			};

			void operator+=(const double& d) {
				TStr temp(d);
				operator+=(temp);
			};

			// + operators

			TStr operator+(const char* s) {
				TStr ts;
				ts = (*this);
				ts += s;
				return ts;
			};

			TStr operator+(const TStr& s) {
				TStr ts;
				ts = (*this);
				ts += s;
				return ts;
			};

			TStr operator+(const bool& b) {
				TStr ts;
				ts = (*this);
				ts += b;
				return ts;
			};

			TStr operator+(const int& i) {
				TStr ts;
				ts = (*this);
				ts += i;
				return ts;
			};

			TStr operator+(const char& c) {
				TStr ts;
				ts = (*this);
				ts += c;
				return ts;
			};

			TStr operator+(const double& d) {
				TStr ts;
				ts = (*this);
				ts += d;
				return ts;
			};

			// destructor

			~TStr() {
				m_rep->m_RefCount--;
				if (m_rep->m_RefCount == 0)
					delete m_rep;
			};

			// utility functions

			bool operator<(const TStr &s) {
				int min = MIN(Len(), s.Len());
				bool min_this = (Len() < s.Len());
				for (int i = 0; i < min; i++)
					if (read(i) < s.read(i)) {
						return true;
					} else if (read(i) > s.read(i)) {
						return false;
					}
				return min_this;
			};

			bool operator>(const TStr &s) {
				int min = MIN(Len(), s.Len());
				bool min_this = (Len() < s.Len());
				for (int i = 0; i < min; i++)
					if (read(i) > s.read(i)) {
						return true;
					} else if (read(i) < s.read(i)) {
						return false;
					}
				return min_this;
			};

			bool operator>=(const TStr &s) {
				return (operator<(s) == false);
			};
			bool operator<=(const TStr &s) {
				return (operator>(s) == false);
			};


			char* c_str() const {
				return m_rep->m_Buff;
			}; // access to plain C-style string
			char* CStr() const {
				return m_rep->m_Buff;
			};  // access to plain C-style string

			int Len() const {
				return strlen(m_rep->m_Buff);
			};       // length of the string (number of characters - like strlen)
			int Length() const {
				return strlen(m_rep->m_Buff);
			};    // length of the string (number of characters - like strlen)
			int length() const {
				return strlen(m_rep->m_Buff);
			};    // length of the string (number of characters - like strlen)
			//int length() const { return strlen(m_rep->m_Buff); };    // length of the string (number of characters - like strlen)
			int size() const {
				return strlen(m_rep->m_Buff);
			};    // length of the string (number of characters - like strlen)
			int GetLength() const {
				return strlen(m_rep->m_Buff);
			}; // length of the string (number of characters - like strlen)

			typedef int size_type;
			//static const size_type npos =-1;


			void Clear() { // sets string to ""
				if (m_rep->m_RefCount == 1) {
					m_rep->Assign(0, "");
				} else {
					m_rep->m_RefCount--;
					m_rep = new TStrRep(0, "");
				};
			};

			void CreateUniqueCopy() { // create unique copy of the string
				if (m_rep->m_RefCount > 1) {
					m_rep->m_RefCount--;
					char* temp = m_rep->m_Buff;
					m_rep = new TStrRep(strlen(temp), temp);
				};
			};

			TStr Left(int len) { // returns first len characters - less if len>Len()
				TStr temp;
				for (int i = 0; ((i < Len()) && (i < len)); i++) temp += read(i);
				return temp;
			};

			TStr Mid(int start, int len) { // returns first len characters, starting at start - less if start+len>=Len()
				TStr temp;
				for (int i = start, j = 0; ((i < Len()) && (j < len)); i++, j++) temp += read(i);
				return temp;
			};

			int FindCharPos(const char c) { // find index of first occurance of character c
				for (int i = 0; i < Len(); i++)
					if (read(i) == c) return i;

				return -1;
			};

			void Replace(const char target, const char replacement) {
				for (int i = 0; i < Len(); i++)
					if (read(i) == target) write(i, replacement);
			};

			// file name manipulation functions

			void ClipFileEx() { // clips off file extension - if any
				int i = Len() - 1;
				while ((i > 0) && (operator[](i) != '.') && (operator[](i) != '/') && (operator[](i) != '\\'))
					i--;
				if (operator[](i) == '.') {
					TStr temp;
					for (int j = 0; j < i; j++)
						temp += operator[](j);
					operator=(temp);
				};
			};

			TStr GetFileEx() { // extracts file extension - if any
				int i = Len() - 1;
				while ((i >= 0) && (m_rep->m_Buff[i] != '.') && (m_rep->m_Buff[i] != '/') && (m_rep->m_Buff[i] != '\\'))
					i--;
				if (m_rep->m_Buff[i] == '.') {
					TStr temp;
					int j;
					if (i > 0) {
						j = i + 1;
					} else {
						j = 0;
					};
					for (; j < Len(); j++)
						temp += m_rep->m_Buff[j];
					return temp;
				};
				return TStr(); // return empty string
			};

			TStr GetFileName() { // extracts file name
				int i = Len() - 1;
				while ( (i >= 0) &&
						(m_rep->m_Buff[i] != '/') &&
						(m_rep->m_Buff[i] != '\\')
					  ) i--;
				TStr temp;
				int j;
				if (i > 0) {
					j = i + 1;
				} else {
					j = 0;
				};
				for (; j < Len(); j++)
					temp += m_rep->m_Buff[j];
				return temp;
			};

			// char access

			void check(int i) const { // check if i is valid index
				if (((i >= 0) && (i < m_rep->m_Size)) == false) {
					assert(false);
				};
			};
			char read(int i) const {
				return m_rep->m_Buff[i];
			};
			void write(int i, char c) {
				check(i);
				m_rep = m_rep->GetOwnCopy();
				m_rep->m_Buff[i] = c;
			};
			char operator[](int i) const {
				check(i);
				return m_rep->m_Buff[i];
			};
			char CharAt(int i) const {
				check(i);
				return m_rep->m_Buff[i];
			};

			// == operators

			friend bool operator==(const TStr& x, const char* s) {
				return ( strcmp( x.m_rep->m_Buff, s) == 0 );
			};

			friend bool operator==(const TStr& x, const TStr& y) {
				return ( strcmp( x.m_rep->m_Buff, y.m_rep->m_Buff) == 0);
			};

			int compare(char *s) {
				return strcmp( m_rep->m_Buff, s);
			};
			int compare(TStr &s) {
				return strcmp( m_rep->m_Buff, s.c_str());
			};


			// != operators

			friend bool operator!=(const TStr& x, const char* s) {
				return ( strcmp( x.m_rep->m_Buff, s) != 0 );
			};

			friend bool operator!=(const TStr& x, const TStr& y) {
				return ( strcmp( x.m_rep->m_Buff, y.m_rep->m_Buff) != 0);
			};

			// ToUpper case
			void ToUpper() { // changes string to uppercase
				for (int i = 0; i < Len(); i++) {
					char x = read(i);
					if ((x >= 'a') && (x <= 'z'))
						write(i, x - ('a' - 'A'));
				};
			};

			TStr GetUpperCase() { // returns string that is upper case of this
				char *bf = new char[Len() + 1];
				for (int i = 0; i < Len(); i++) {
					bf[i] = read(i);
					if ((bf[i] >= 'a') && (bf[i] <= 'z')) bf[i] = bf[i] - ('a' - 'A');
				};
				bf[Len()] = '\0';
				TStr res(bf);
				delete bf;
				return res;
			};

			bool empty() {
				return (size() == 0);
			};

			// trimming functions

			void TrimRight() {
				TStr r;
				int i = Len() - 1;
				while ((i >= 0) && ( (read(i) == ' ') || (read(i) == 9) )) i--;
				r = Left(i + 1);
				operator=(r);
			};

			void TrimLeft() {
				TStr r;
				int i = 0;
				while ((i < Len()) && ( (read(i) == ' ') || (read(i) == 9) )) i++;
				r = Mid(i, Len() - i);
				operator=(r);
			};

			void TrimLeftRight() {
				TrimLeft();
				TrimRight();
			};

			// Find
			int find_first_of(const char *s) {
				int l = strlen(s);
				for (int i = 0; i < size(); i++) {
					for (int j = 0; j < l; j++)
						if (c_str()[i] == s[j])
							return i;
				};
				return -1;
			};

			int find_first_not_of(const char *s) {
				int l = strlen(s);
				for (int i = 0; i < size(); i++) {
					for (int j = 0; j < l; j++)
						if (c_str()[i] != s[j])
							return i;
				};
				return -1;
			};

			// substr
			TStr substr(int pos, int n) {
				TStr s;
				for (int i = pos; ((i < n) && (i < size())); i++)
					s += CharAt(i);
				return s;
			};
	};

	//////////////////////////////////////////////////////////

	class TExc {
			TStr m_Msg;
		public:
			TExc() {
				m_Msg = "Unknown exception";
			};
			TExc(TStr s) {
				m_Msg = s;
			};
			TExc(const char *s) {
				m_Msg = s;
			};
			TStr GetMsg() const {
				return m_Msg;
			};
	};

	//////////////////////////////////////////////////////////

	class TErrorLog {
		public:
			TStr m_Msg;
			int m_Code;
	};

	extern TErrorLog StdErrorLog; // standard error log

	//////////////////////////////////////////////////////////

	class TOut {
		public:
			virtual void Put(const char *s) = 0;
			void Put(TStr &s) {
				Put(s.CStr());
			};
			void Put(int a) {
				TStr s(a);
				Put(s.CStr());
			};
			void Put(double a) {
				TStr s(a);
				Put(s.CStr());
			};
			void Put(char a) {
				TStr s(a);
				Put(s.CStr());
			};
			void Put(unsigned char a) {
				TStr s(a);
				Put(s.CStr());
			};
			void Put(bool a) {
				TStr s(a);
				Put(s.CStr());
			};
			void Put(float a) {
				TStr s(a);
				Put(s.CStr());
			};
	};

	//////////////////////////////////////////////////////////

	class TOutStd : public TOut {
		public:
			virtual void Put(const char *s) {
				printf("%s", s);
			};
	};

	//////////////////////////////////////////////////////////

	class TOutFile: public TOut {
			FILE *fp;
		public:
			TOutFile() {
				fp = NULL;
			}
			TOutFile(TStr &FileName) {
				fp = fopen(FileName.CStr(), "w");
			};
			TOutFile(const char *FileName) {
				fp = fopen(FileName, "w");
			};
			~TOutFile() {
				if (fp != NULL) fclose(fp);
			};
			virtual void Put(const char *s) {
				if (fp != NULL) fprintf(fp, "%s", s);
			};
	};

	//////////////////////////////////////////////////////////

	class TOutDouble: public TOut {
			TOut *p1, *p2;
		public:
			TOutDouble() {
				p1 = p2 = NULL;
			};
			TOutDouble(TOut *tp1, TOut *tp2) {
				p1 = tp1;
				p2 = tp2;
			};
			virtual void Put(const char *s) {
				if (p1 != NULL) {
					p1->Put(s);
					p2->Put(s);
				};
			};
	};

	//////////////////////////////////////////////////////////

}
