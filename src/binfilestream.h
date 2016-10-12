#pragma once

#ifndef BINFILESTREAM_H
#define BINFILESTREAM_H

//////////////////////////////////////////////////////////////////////
// Module: BinFileStream
// Author: Viktor Jovanoski
// Desc:   implements classes for input/output binary file interraction
//////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "str.h"
#include "ds.h"

namespace VikStd {

	///////////////////////////////////////////////////////////////////////

	typedef union {
		int i;
		double d;
		char c;
	} TBuff;

	typedef unsigned char UChar;

	///////////////////////////////////////////////////////////////////////

	class TIBinStream {
		protected:

			TBuff m_buf;
			virtual bool GetBf(int size, void* dest) = 0;

		public:

			virtual bool AtEof() = 0;
			virtual void AddData(UChar* pData, int DataLength) {}; // default does not do anything
			virtual void GetCh(char &ch) {
				GetBf(sizeof(char), &m_buf);
				ch = m_buf.c;
			};
			virtual void GetStr(TStr &s) {
				int i, l;
				char c;
				GetInt(l);
				s = "";
				for (i = 0; i < l; i++) {
					GetCh(c);
					s += c;
				};
			};
			virtual void GetInt(int &i) {
				GetBf(sizeof(int), &m_buf);
				i = m_buf.i;
			};
			virtual void GetDouble(double &d) {
				GetBf(sizeof(double), &m_buf);
				d = m_buf.d;
			};
			virtual void GetBoolean(bool &b) {
				GetBf(sizeof(int), &m_buf);
				if (m_buf.i == 0) {
					b = false;
				} else {
					b = true;
				};
			};
			virtual void GetBuffer(int len, void* dest) {
				GetBf(len, dest);
			};
	};

	///////////////////////////////////////////////////////////////////////

	class TIBinFileStream: public TIBinStream {
		protected:

			FILE *m_fp;
			bool m_active;

			bool GetBf(int size, void* dest) {
				if (fread(dest, size, 1, m_fp) != 1) return false;
				return true;
			};

		public:
			TIBinFileStream() {
				m_fp = NULL;
				m_active = false;
			};
			virtual ~TIBinFileStream() {
				if (m_active) Close();
			};

			bool Open(const char *FileName) {
				if (m_active) Close();
				m_fp = fopen(FileName, "rb");
				if (m_fp == NULL) return false;
				m_active = true;
				return true;
			};

			bool AtEof() {
				return (feof(m_fp) == 0);
			};
			void Close() {
				if (m_active) fclose(m_fp);
			};
	};

	////////////////////////////////////////////////////////////////////////////////

	class TIBinMemStream: public TIBinStream {
		protected:

			// length of the buffer
#define cBuffLen 1024*16

			TVec<UChar*> m_BuffArray;
			int m_CurrBuff;  // index of the current buffer
			int m_BuffIndex; // position in current buffer
			int m_LastBuffLen; // length of data in last buffer

			bool GetBf(int size, void* dest) {
				// sanity check
				if (AtEof()) throw TExc("Trying to read past stream's EOF.");
				UChar *tmp = (UChar*)dest;

				while (AtEof() == false) {
					if (size == 0) return true;
					int ToCopy;

					if (m_CurrBuff < m_BuffArray.Len()) // current buffer is not the last one
						ToCopy = MIN(size, cBuffLen - m_BuffIndex);
					else // current buffer is the last one
						ToCopy = MIN(size, m_LastBuffLen - m_BuffIndex);

					memcpy(tmp, m_BuffArray[m_CurrBuff] + m_BuffIndex, ToCopy);
					tmp += ToCopy;
					size -= ToCopy;
					m_BuffIndex += ToCopy;
					if (m_CurrBuff < m_BuffArray.Len()) { // current buffer is not the last one
						if (m_BuffIndex == cBuffLen) { // switch to next buffer
							m_BuffIndex = 0;
							m_CurrBuff++;
						};
					};
				};
				if (size == 0) return true;
				throw TExc("Trying to read past stream's EOF.");
			};

			void Clear() {
				for (int i = 0; i < m_BuffArray.Len(); i++) {
					delete[] m_BuffArray[i];
					m_BuffArray[i] = NULL;
				};
				m_BuffArray.Clr();
			};

		public:

			TIBinMemStream() {};
			virtual ~TIBinMemStream() {};

			void AddData(UChar* pData, int DataLength) {
				// inserts the data into the current buffer
				UChar* pTempData = pData;
				int TmpOffset = 0;
				while (pTempData < pData + DataLength) {

					if ((m_LastBuffLen == cBuffLen) || (m_BuffArray.Len() == 0)) {
						// create a new buffer
						m_BuffArray.Add();
						m_BuffArray.Last() = new UChar[cBuffLen];
						m_LastBuffLen = 0;
					};

					int CopyLen = MIN(cBuffLen - m_LastBuffLen, DataLength - TmpOffset);
					memcpy(m_BuffArray.Last(), pTempData, CopyLen);
					m_LastBuffLen += m_LastBuffLen;
					pTempData += CopyLen;
					TmpOffset += CopyLen;
				};
			};

			bool AtEof() {
				return ((m_CurrBuff == m_BuffArray.Len()) && (m_BuffIndex >= m_LastBuffLen));
			};
	};

	////////////////////////////

	class TOBinStream  {
		protected:

			TBuff m_buf;
			virtual bool PutBf(int size) = 0;

		public:

			virtual bool AtEof() = 0;

			virtual void PutCh(char ch) {
				m_buf.c = ch;
				PutBf(sizeof(char));
			};
			virtual void PutStr(TStr &s) {
				int l = s.GetLength();
				PutInt(l);
				for (int i = 0; i < l; i++) {
					PutCh(s[i]);
				};
			};
			virtual void PutInt(int i) {
				m_buf.i = i;
				PutBf(sizeof(int));
			};
			virtual void PutDouble(double d) {
				m_buf.d = d;
				PutBf(sizeof(double));
			};
			virtual void PutBoolean(bool b) {
				if (b) {
					m_buf.i = 1;
				} else {
					m_buf.i = 0;
				};
				PutBf(sizeof(int));
			};
	};

	////////////////////////////////////////////////////////////////////////////////

	class TOBinFileStream : public TOBinStream {
		protected:

			FILE *m_fp;
			bool m_active;
			bool PutBf(int size) {
				fwrite(&m_buf, size, size, m_fp);
				return true;
			};

		public:

			TOBinFileStream() {
				m_fp = NULL;
				m_active = false;
			};
			virtual ~TOBinFileStream() {
				if (m_active) Close();
			};

			bool Open(const char *FileName) {
				if (m_active) Close();
				m_fp = fopen(FileName, "wb");
				if (m_fp == NULL) return false;
				m_active = true;
				return true;
			};

			bool AtEof() {
				return (feof(m_fp) == 0);
			};
			void Close() {
				if (m_active) fclose(m_fp);
			};

	};

	////////////////////////////////////////////////////////////////////////////

#endif

}
