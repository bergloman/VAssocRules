#pragma once


#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>


namespace VikStd {

	/////////////////////////////////////////////////
	// Pair

	template <class C1, class C2>
	class TPair {
		public:
			C1 m_c1;
			C2 m_c2;
			TPair(): m_c1(), m_c2() {};
			TPair(const C1 &tc1, const C2 &tc2): m_c1(tc1), m_c2(tc2) {};
			TPair(const TPair &p): m_c1(p.m_c1), m_c2(p.m_c2) {};

			bool operator==(const TPair &p) {
				return ((m_c1 == p.m_c1) && (m_c2 == p.m_c2));
			};
			bool operator<(const TPair &p) {
				if (m_c1 < p.m_c1) return true;
				if (m_c1 > p.m_c1) return false;
				return (m_c2 < p.m_c2);
			};
			bool operator>(const TPair &p) {
				return ((*this) <= p) == false;
			};
			bool operator<=(const TPair &p) {
				if ((*this) < p) return true;
				if ((*this) == p) return true;
				return false;
			};
			bool operator>=(const TPair &p) {
				return ((*this) < p) == false;
			};

			void operator=(const TPair &p) {
				m_c1 = p.m_c1;
				m_c2 = p.m_c2;
			};
	};
	typedef TPair<double, int> TPairDoubleInt;

	/////////////////////////////////////////////////
	// Vector
	//
	// Stored objects must implement default constructor
	// and operator=(const TVal& Val)
	//

	template <class TVal>
	class TVec {
		protected:

			int m_ArrayLen, m_Vals;
			TVal* m_Array;

			void Resize(const int& NewArrayLen = -1) {
				if (NewArrayLen == -1) {
					if (m_Vals == 0) {
						m_ArrayLen = 16;
					} else {
						m_ArrayLen *= 2;
					};
				} else {
					if (NewArrayLen <= m_ArrayLen) {
						return;
					} else {
						m_ArrayLen = NewArrayLen;
					};
				};

				if (m_Array == NULL) {
					m_Array = new TVal[m_ArrayLen];
				} else {
					TVal* NewValT = new TVal[m_ArrayLen];
					assert(NewValT != NULL);
					for (int ValN = 0; ValN < m_Vals; ValN++) {
						NewValT[ValN] = m_Array[ValN];
					};
					delete[] m_Array;
					m_Array = NewValT;
				};
			};

		public:

			// constructors

			TVec() {
				m_ArrayLen = 0;
				m_Vals = 0;
				m_Array = NULL;
			};

			TVec(const int& NewVals) {
				assert(0 <= NewVals);
				m_ArrayLen = m_Vals = NewVals;
				if (NewVals == 0) {
					m_Array = NULL;
				} else {
					m_Array = new TVal[NewVals];
				};
			};

			// destructor

			~TVec() {
				delete[] m_Array;
			};

			// access operators

			TVal& operator[](const int& ValN) const {
				if (!((0 <= ValN) && (ValN < m_Vals))) {
					assert(false);
				};
				//assert((0<=ValN)&&(ValN<m_Vals));
				return m_Array[ValN];
			};

			TVal& At(const int& ValN) const { // access - 0-based index
				assert((0 <= ValN) && (ValN < m_Vals));
				return m_Array[ValN];
			};

			// generate enough space in advance
			// deletes previous entries!
			void Gen(const int& _Vals) {
				assert(0 <= _Vals);
				delete[] m_Array;
				m_ArrayLen = _Vals;
				m_Vals = 0;
				if (_Vals == 0) {
					m_Array = NULL;
				} else {
					m_Array = new TVal[_Vals];
				};
			};

			// functions to clear the content

			void Clr() {
				delete[] m_Array;
				m_ArrayLen = m_Vals = 0;
				m_Array = NULL;
			};   // clears the content - length==0
			void Clear() {
				delete[] m_Array;
				m_ArrayLen = m_Vals = 0;
				m_Array = NULL;
			}; // clears the content - length==0

			// utility functions

			bool Empty() const {
				return m_Vals == 0;
			};
			int Len() const {
				return m_Vals;
			};         // returns number of items
			int Length() const {
				return m_Vals;
			};      // returns number of items
			int Size() const {
				return m_ArrayLen;
			};  // returns size of reserved space

			TVal& Last() {
				return GetVal(Len() - 1);
			};  // access to the last element

			// functions to dynamically grow the array

			int Add() { // add an empty element to the end - grow if neccesary
				if (m_Vals == m_ArrayLen)
					Resize();
				return m_Vals++;
			};

			int Add(const TVal& Val) { // add an element to the end - grow if neccesary
				if (m_Vals == m_ArrayLen)
					Resize();
				m_Array[m_Vals] = Val;
				return m_Vals++;
			};

			TVal& GetVal(const int& ValN) const { // access to data - by value, not by reference
				return operator[](ValN);
			};

			void Swap(const int& ValN1, const int& ValN2) { // swaps elements at given indexes
				TVal Val = m_Array[ValN1];
				m_Array[ValN1] = m_Array[ValN2];
				m_Array[ValN2] = Val;
			};

			void CopyFrom(const TVec<TVal> &v) { // copies content of parameter vector into itself
				Clr();
				m_ArrayLen = v.m_ArrayLen;
				m_Vals = v.m_Vals;
				if (m_ArrayLen > 0) {
					m_Array = new TVal[m_ArrayLen];
					for (int i = 0; i < m_Vals; i++) m_Array[i] = v.m_Array[i];
				};
			};

			void Delete(const int index) {
				assert((0 <= index) && (index < m_Vals));
				for (int i = index + 1; i < m_Vals; i++) m_Array[i - 1] = m_Array[i];
				m_Vals--;
			};

			void DeleteLast() {
				assert(m_Vals > 0);
				m_Vals--;
			};

			void operator=(const TVec<TVal> &v) {
				CopyFrom(v);
			};

			// sort functions and utilities

			int GetPivotValN(const int& LValN, const int& RValN) {
				int ValN1 = LValN;
				int ValN2 = RValN;
				int ValN3 = (LValN + RValN) / 2;

				TVal& Val1 = m_Array[ValN1];
				TVal& Val2 = m_Array[ValN2];
				TVal& Val3 = m_Array[ValN3];
				if (Val1 < Val2) {
					if (Val2 < Val3) {
						return ValN2;
					} else if (Val3 < Val1) {
						return ValN1;
					} else {
						return ValN3;
					};
				} else {
					if (Val1 < Val3) {
						return ValN1;
					} else if (Val3 < Val2) {
						return ValN2;
					} else {
						return ValN3;
					};
				};
			};

			void BSort(const int& MnLValN, const int& MxRValN, const bool& Ascending) {
				for (int ValN1 = MnLValN; ValN1 <= MxRValN; ValN1++) {
					for (int ValN2 = MxRValN; ValN2 > ValN1; ValN2--) {
						if (Ascending) {
							if (m_Array[ValN2] < m_Array[ValN2 - 1]) Swap(ValN2, ValN2 - 1);
						} else {
							if (m_Array[ValN2] > m_Array[ValN2 - 1]) Swap(ValN2, ValN2 - 1);
						};
					};
				};
			};

			void ISort(const int& MnLValN, const int& MxRValN, const bool& Ascending) {
				if (MnLValN < MxRValN) {
					for (int ValN1 = MnLValN + 1; ValN1 <= MxRValN; ValN1++) {
						TVal Val = m_Array[ValN1];
						int ValN2 = ValN1;
						if (Ascending) {
							while ((ValN2 > MnLValN) && (m_Array[ValN2 - 1] > Val)) {
								m_Array[ValN2] = m_Array[ValN2 - 1];
								ValN2--;
							};
						} else {
							while ((ValN2 > MnLValN) && (m_Array[ValN2 - 1] < Val)) {
								m_Array[ValN2] = m_Array[ValN2 - 1];
								ValN2--;
							};
						};
						m_Array[ValN2] = Val;
					};
				};
			};

			int Partition(const int& MnLValN, const int& MxRValN, const bool& Ascending) {
				int PivotValN = GetPivotValN(MnLValN, MxRValN);
				Swap(PivotValN, MnLValN);
				TVal PivotVal = m_Array[MnLValN];
				int LValN = MnLValN;
				int RValN = MxRValN + 1;
				do {
					if (Ascending) {
						do {
							RValN--;
						} while ((RValN > MnLValN) && (m_Array[RValN] >= PivotVal));
						do {
							LValN++;
						} while ((LValN <= MxRValN) && (m_Array[LValN] <= PivotVal));
					} else {
						do {
							RValN--;
						} while ((RValN > MnLValN) && (m_Array[RValN] <= PivotVal));
						do {
							LValN++;
						} while ((LValN <= MxRValN) && (m_Array[LValN] >= PivotVal));
					}
					if (LValN < RValN) {
						Swap(LValN, RValN);
					} else {
						if (RValN < MnLValN) return -1;
						return RValN;
					};
				} while (true);
			};

			void QSort(const int& MnLValN, const int& MxRValN, const bool& Ascending) {
				if (MnLValN < MxRValN) {
					if (MxRValN - MnLValN < 20) {
						ISort(MnLValN, MxRValN, Ascending);
					} else {
						int SplitValN = Partition(MnLValN, MxRValN, Ascending);
						if (SplitValN >= 0) {
							if (SplitValN == MxRValN) SplitValN--;
							QSort(MnLValN, SplitValN, Ascending);
							QSort(SplitValN + 1, MxRValN, Ascending);
						};
					};
				};
			};

			void Sort(const bool& Ascending = true) { // sorts the array
				QSort(0, Len() - 1, Ascending);
			};

			int Search(const TVal& Val) const { // find index of given value. -1 if not found.
				for (int ValN = 0; ValN < m_Vals; ValN++)
					if (Val == m_Array[ValN])
						return ValN;
				return -1;
			};

			int SearchBisect(TVal& Val) const { // find (with bisection) index of given value. -1 if not found.
				int l = 0, r = Len() - 1, s;
				while (l < r) {
					if (Val == m_Array[l]) return l;
					if (Val == m_Array[r]) return r;
					s = (l + r) / 2;
					if (Val < m_Array[s]) {
						r = s;
					} else {
						if (l < s) {
							l = s;
						} else {
							l++;
						};
					};
				};
				return -1;
			};

			void QuickCopy(TVal *p, int l) { // quick copying - for basic types only
				Clr();
				if (l <= 0) return;
				m_Vals = m_ArrayLen = l;
				m_Array = new TVal[l];
				memcpy(m_Array, p, l);
			};

			TVal *GetArray() const {
				return m_Array;
			};
			int GetArrayLen() const {
				return m_ArrayLen;
			};

			TVal Max() { // find max value
				assert(0 < m_Vals);
				TVal res = m_Array[0];
				for (int i = 0; i < m_Vals; i++)
					if (m_Array[i] > res) res = m_Array[i];
				return res;
			};

			TVal Min() { // find min value
				assert(0 < m_Vals);
				TVal res = m_Array[0];
				for (int i = 0; i < m_Vals; i++)
					if (m_Array[i] < res) res = m_Array[i];
				return res;
			};

			TVal MaxIndex(int &m) { // find max value and position
				assert(0 < m_Vals);
				TVal res = m_Array[0];
				m = 0;
				for (int i = 0; i < m_Vals; i++) {
					if (m_Array[i] > res) {
						res = m_Array[i];
						m = i;
					};
				};
				return res;
			};

			TVal MinIndex(int &m) { // find min value and position
				assert(0 < m_Vals);
				TVal res = m_Array[0];
				m = 0;
				for (int i = 0; i < m_Vals; i++) {
					if (m_Array[i] < res) {
						res = m_Array[i];
						m = i;
					};
				};
				return res;
			};

			void ResizeX(const int NewArrayLen = -1) {
				Resize(NewArrayLen);
				m_Vals = NewArrayLen;
			};
	};

	/////////////////////////////////////////////////////////////////////////////////

	typedef TVec<bool> TBoolVec;
	typedef TVec<char> TChVec;
	typedef TVec<int> TIntVec;
	typedef TVec<double> TDoubleVec;

	/////////////////////////////////////////////////////////////////////////////////
	// 2D-Vector
	/////////////////////////////////////////////////////////////////////////////////

	template <class TVal>
	class TVVec {
		private:

			int XDim, YDim;
			TVec<TVal> ValV;

		public:

			TVVec(): XDim(), YDim(), ValV() {};

			TVVec(const TVVec& Vec):
				XDim(Vec.XDim), YDim(Vec.YDim), ValV(Vec.ValV) {};

			TVVec(const int& _XDim, const int& _YDim): XDim(), YDim(), ValV() {
				Gen(_XDim, _YDim);
			};

			TVVec<TVal>& operator=(const TVVec<TVal>& Vec) {
				XDim = Vec.XDim;
				YDim = Vec.YDim;
				ValV = Vec.ValV;
				return *this;
			};

			bool operator==(const TVVec& Vec) const {
				return (XDim == Vec.XDim) && (YDim == Vec.YDim) && (ValV == Vec.ValV);
			};

			void Gen(const int& _XDim, const int& _YDim) {
				assert((_XDim >= 0) && (_YDim >= 0));
				XDim = _XDim;
				YDim = _YDim;
				ValV.Gen(XDim * YDim);
			};

			TVal& At(const int& X, const int& Y) const {
				if (((0 <= X) && (X < (int)XDim) && (0 <= Y) && (Y < (int)YDim)) == false)
					assert(false);
				return ValV[X * YDim + Y];
			};

			int GetXDim() const {
				return XDim;
			}
			int GetYDim() const {
				return YDim;
			}

			void PutToAll(const TVal &val) {
				for (int i = 0; i < XDim * YDim; i++) ValV[i] = val;
			};

	};

	///////////////////////////////////////////////////////////////////////
	// Stack
	///////////////////////////////////////////////////////////////////////
	/*
	template <class TVal>
	class TStack{
	private:

	  TVec<TVal> ValV;

	public:

	    TStack(): ValV(){};

	  TStack(const TStack& Stack): ValV(Stack.ValV){};

	  bool Empty(){ return ValV.Len()==0; };
	  int Len(){ return ValV.Len(); };
	  TVal& Top(){
	        assert(0<ValV.Len());
	        return ValV[ValV.Len()-1];
	    };
	  void Push(){ ValV.Add(); };
	  void Push(const TVal& Val){ ValV.Add(Val); };
	  void Pop(){
	        assert(0<ValV.Len());
	        ValV.Del(ValV.Len()-1);
	    };
	};
	*/
	///////////////////////////////////////////////////////////////////////
	// Random numbers generator
	///////////////////////////////////////////////////////////////////////

	class TRnd {
		public:

			int RndSeed;

		private:
			int a;
			int m;
			int q; // m DIV a
			int r; // m MOD a

			void InitNums() {
				RndSeed = 0;
				a = 16807;
				m = 2147483647;
				q = 127773; // m DIV a
				r = 2836; // m MOD a
			};

			int Seed;
			int GetNextSeed() {
				if ((Seed = a * (Seed % q) - r * (Seed / q)) > 0) {
					return Seed;
				} else {
					return Seed += m;
				};
			};
		public:

			TRnd() {
				InitNums();
				time_t x;
				PutSeed(time(&x));
			};
			TRnd(const int& _Seed) {
				InitNums();
				PutSeed(_Seed);
			};

			TRnd& operator=(const TRnd& Rnd) {
				InitNums();
				Seed = Rnd.Seed;
				return *this;
			};

			int GetInt(const int& Range) { // returns integer in interval [0,..,range-1]
				if (Range > 0) {
					return GetNextSeed() % Range;
				} else return 0;
			};

			double GetFlt() {
				return (double)GetNextSeed() / (double)m;
			}; // returns double in interval [0,1]

			double GetNrmFlt() { // normal distribution
				double v1, v2, rsq;
				do {
					v1 = 2.0 * GetFlt() - 1.0; // pick two uniform numbers in the square
					v2 = 2.0 * GetFlt() - 1.0; // extending from -1 to +1 in each direction
					rsq = v1 * v1 + v2 * v2; // see if they are in the unit cicrcle
				} while ((rsq >= 1.0) || (rsq == 0.0)); // and if they are not, try again
				double fac = sqrt(-2.0 * log(rsq) / rsq); // Box-Muller transformation
				return v1 * fac;
				//  return v2*fac; // second deviate
			};

			double GetNrmFlt(const double& Mean, const double& SDev, const double& Mn, const double& Mx) { // custom normal distribution
				double Val = Mean + GetNrmFlt() * SDev;
				if (Val < Mn) Val = Mn;
				if (Val > Mx) Val = Mx;
				return Val;
			};

			void PutSeed(const int& _Seed) {
				if (_Seed <= 0) {
					Seed = (int)time(NULL);
				} else {
					Seed = _Seed;
				}
			}
			int GetSeed() const {
				return Seed;
			};
			void Randomize() {
				PutSeed(RndSeed);
			};

			bool Check() {
				int PSeed = Seed;
				Seed = 1;
				for (int SeedN = 0; SeedN < 10000; SeedN++)
					GetNextSeed();
				bool Ok = (Seed == 1043618065);
				Seed = PSeed;
				return Ok;
			};
	};

	/////////////////////////////////////////////////
	// Subsets enumerator

	class TSubsets {
			TVec<bool> v;
			int s;
			bool finished;
		public:
			TSubsets(int i): v(i), s(i), finished(false) {};

			void Init() {
				for(int i = 0; i < s; i++) v[i] = true;
				finished = false;
			};

			bool Next() {
				if (finished) return false;
				bool c = true;
				int i = s - 1;
				while ((c) && (i >= 0)) {
					if (v[i]) {
						v[i] = false;
						c = false;
					} else {
						v[i] = true;
					};
					i--;
				};
				if (c) finished = true;
				return true;
			};

			void SetAll() {
				for(int i = 0; i < s; i++) v[i] = true;
			};

			bool IsSet(int i) {
				assert((i >= 0) && (i < s));
				return v[i];
			};

			void Put(int i, bool val) {
				assert((i >= 0) && (i < s));
				v[i] = val;
			};

			bool operator[](int i) {
				assert((i >= 0) && (i < s));
				return v[i];
			};
	};


	/////////////////////////////////////////////////
	// Reference Count

	template  <class TC>
	class TCRef {
		private:
			int Refs;
		public:
			TC *Val;

			TCRef(): Refs(0), Val(NULL) {};
			~TCRef() {
				assert(Refs == 0);
				delete Val;
			};

			TCRef& operator=(const TCRef&) {
				assert(false);
				return *this;
			};

			void MkRef() {
				Refs++;
			};
			void UnRef() {
				assert(Refs > 0);
				Refs--;
			};
			bool NoRef() const {
				return Refs == 0;
			};
			int GetRefs() const {
				return Refs;
			};
	};

	/////////////////////////////////////////////////
	// Smart Pointer

	template <class TC>
	class TPt {
		private:
			TCRef<TC>* Addr;

			void MkRef() {
				if (Addr != NULL)
					Addr->MkRef();
			};

			void UnRef() {
				if (Addr != NULL) {
					Addr->UnRef();
					if (Addr->NoRef())
						delete Addr;
				};
			};

		public:

			TPt(): Addr(NULL) {};

			TPt(const TPt& Pt): Addr(Pt.Addr) {
				MkRef();
			};

			TPt(TC* _Addr) {
				Addr = new TCRef<TC>;
				Addr->Val = _Addr;
				MkRef();
			};

			~TPt() {
				UnRef();
			};

			TPt& operator=(const TPt& Pt) {
				if (this != &Pt) {
					UnRef();
					Addr = Pt.Addr;
					MkRef();
				}
				return *this;
			};
			bool operator==(const TPt& Pt) const {
				return Addr == Pt.Addr;
			};

			TC* operator->() const {
				assert(Addr != NULL);
				return Addr->Val;
			};
			//TC& operator*() const {Assert(Addr!=NULL); return *Addr;};
			//TC& operator[](const int& RecN) const {Assert(Addr!=NULL); return Addr[RecN];};
			//TC* operator()() const {return Addr;};

			bool Empty() const {
				return (Addr == NULL);
			};
			int GetRefs() const {
				if (Addr == NULL) return -1;
				return Addr->GetRefs();
			};
	};


	/////////////////////////////////////////////////

}
