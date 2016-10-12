#pragma once

#include <stdlib.h>
#include <assert.h>
#include "stdvik.h"

namespace VikApriori {

	////////////////////////////////////////////////////////////////////
	// Definition of classes TItemset, TItemsetRef, TItemsetManager
	// and TListOfItemsets
	//
	// uses constant ITEMSETS_DEBUG to check lengths and indexing
	////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////
	// Some predefined constants
	////////////////////////////////////////////////////////////////////

	#define INITIAL_LIST_LEN			// initial length of TListOfItemsets
	#define IM_CHUNK_SIZE 1000		// chunk size - for TItemsetManager

	#define ITEMSETS_DEBUG
	////////////////////////////////////////////////////////////////////

	typedef unsigned char UCHAR;
	extern UCHAR mask2[];

	///////////////////

	class TSetBitRet{
		int array[256];
	public:
		TSetBitRet();
		int operator[](unsigned char &ch) { return array[ch]; };
	};

	/////////////////////////////////////////////////////////////////////
	// struct TIMChunk
	/////////////////////////////////////////////////////////////////////

	struct TIMChunk{
		
		VikStd::TVec<unsigned char> m_v;
		VikStd::TVec<int> m_NextFreeItemset;
		int m_ItemsetSize;
		int m_ItemsetSizeB;
		int m_FirstFreeItemset;
		int m_NextFreeChunk;

		TIMChunk(){
			m_ItemsetSize=m_ItemsetSizeB=m_FirstFreeItemset=m_NextFreeChunk=-1;
		};

		void Init(int is){		
			m_ItemsetSize=is;
			m_ItemsetSizeB=(is/8)+1;
			if (is%8==0) m_ItemsetSizeB--;
			for (int i=0; i<IM_CHUNK_SIZE; i++){
				for (int j=0; j<m_ItemsetSizeB; j++) m_v.Add(0);
				m_NextFreeItemset.Add(i-1);
			};
			m_FirstFreeItemset=IM_CHUNK_SIZE-1;
			m_NextFreeChunk=-1;
		};

		unsigned char* GetFirstByte(int i){	return &(m_v[i*m_ItemsetSizeB]); };
	};
	typedef TIMChunk* PIMChunk;

	////////////////////////////////////////////////////////////////////
	// class TUCharManager
	////////////////////////////////////////////////////////////////////

	class TUCharManager{

		VikStd::TVec<TIMChunk> m_Chunks;	// vector of chunks
		int m_FirstFreeChunk;			// index of first free Chunk (-1 if none)
		int m_ItemsetSize;				// size of itemset
		int m_ItemsetSizeB;				// size of itemset in bytes
		TSetBitRet SetBitRet;			// for fast counting of the set items


		int FindFreeChunk(){			// returns index of first free chunk
			for (int i=0; i<m_Chunks.Len(); i++){
				if (m_Chunks[i].m_FirstFreeItemset>0)
					return i;
			};
			return -1;
		};

		/*void AssertValid(int c, int i){
			if ((c<0) || (c>=m_Chunks.Len()) || (i<0) || (i>=IM_CHUNK_SIZE)){
				char bf[400];
				sprintf(bf,"Illegal itemset reference: Index overflow: Chunk=%d Index=%d",c,i);
				throw TExc(bf);
			};
			if (m_Chunks[c].m_NextFreeItemset[i]>=-1){
				char bf[400];
				sprintf(bf,"Illegal itemset reference: Itemset not initialised: Chunk=%d Index=%d",c,i);
				throw TExc(bf);
			};
		};*/
		void AssertValid(int c, int i);

		bool Assign(int& nc, int& ni, int ci){ // assignes itemset to chunk
			PIMChunk c=&(m_Chunks[ci]);
			if (c->m_FirstFreeItemset<0) 
				return false; // some error

			ni=c->m_FirstFreeItemset; // assign new itemset
			nc=ci;

			// dequeue this itemset form the list of free itemsets
			c->m_FirstFreeItemset=c->m_NextFreeItemset[ni]; 
			
			// if chunk becomes fully loaded, dequeue this chunk from list of free chunks
			if (c->m_FirstFreeItemset<0) m_FirstFreeChunk=c->m_NextFreeChunk;

			c->m_NextFreeItemset[ni]=-2;
			return true;
		};

	public:

		VikStd::TCommon *m_Common;
		
		TUCharManager(){ // constructor
			m_FirstFreeChunk=-1; 
			m_ItemsetSize=0; 
			m_ItemsetSizeB=0;		
		};

		~TUCharManager(){};

		void InitItemsetSize(int is, VikStd::TCommon *common){ // should be called immidiately when this parameter is known - before any TItemset is created
			m_FirstFreeChunk=-1; 
			m_ItemsetSize=is; 
			m_ItemsetSizeB=(is/8)+1;
			if (is%8==0) m_ItemsetSizeB--;
			m_Common=common;
		};

		int GetItemsetSize(){ return m_ItemsetSize; };
		int GetItemsetSizeB(){ return m_ItemsetSizeB; };

		bool NewItemset(int& nc, int& ni){ // reserves space for new itemset
			if (m_FirstFreeChunk>=0)
				return Assign(nc,ni,m_FirstFreeChunk);
			m_Chunks.Add();
			m_Chunks.Last().Init(m_ItemsetSize);
			m_FirstFreeChunk=m_Chunks.Len()-1;
			return Assign(nc,ni,m_FirstFreeChunk);
		};

		void RemoveItemset(int& cn, int& i){ // frees space of specified itemset
			#ifdef ITEMSETS_DEBUG
				AssertValid(cn,i);
			#endif

			PIMChunk c=&(m_Chunks[cn]);				
			if (c->m_FirstFreeItemset<0){ // this chunk has become free
				c->m_NextFreeChunk=m_FirstFreeChunk;
				m_FirstFreeChunk=cn;
			};
			c->m_NextFreeItemset[i]=c->m_FirstFreeItemset;
			c->m_FirstFreeItemset=i;
		};

		void Clear(int c, int i){
			#ifdef ITEMSETS_DEBUG
				AssertValid(c,i);
			#endif
			UCHAR *ch=m_Chunks[c].GetFirstByte(i);
			memset(ch,0,m_ItemsetSizeB);
		};
		
		void Copy(int tc, int ti, int sc, int si){
			#ifdef ITEMSETS_DEBUG
				AssertValid(tc,ti);
				AssertValid(sc,si);
			#endif
			UCHAR *cht=m_Chunks[tc].GetFirstByte(ti);
			UCHAR *chs=m_Chunks[sc].GetFirstByte(si);
			memcpy(cht,chs,m_ItemsetSizeB);
		};
		
		bool Eq(int tc, int ti, int sc, int si){
			#ifdef ITEMSETS_DEBUG
				AssertValid(tc,ti);
				AssertValid(sc,si);
			#endif
			UCHAR *cht=m_Chunks[tc].GetFirstByte(ti);
			UCHAR *chs=m_Chunks[sc].GetFirstByte(si);
			return (memcmp(cht,chs,m_ItemsetSizeB)==0);
		};

		bool Lwr(int tc, int ti, int sc, int si){
			#ifdef ITEMSETS_DEBUG
				AssertValid(tc,ti);
				AssertValid(sc,si);
			#endif
			UCHAR *cht=m_Chunks[tc].GetFirstByte(ti);
			UCHAR *chs=m_Chunks[sc].GetFirstByte(si);
			for (int i=0; i<m_ItemsetSizeB; i++){
				if (cht[i]<chs[i]) return true;
				if (cht[i]>chs[i]) return false;
			};
			return false;
		};

		bool LwrEq(int tc, int ti, int sc, int si){
			#ifdef ITEMSETS_DEBUG
				AssertValid(tc,ti);
				AssertValid(sc,si);
			#endif
			UCHAR *cht=m_Chunks[tc].GetFirstByte(ti);
			UCHAR *chs=m_Chunks[sc].GetFirstByte(si);
			for (int i=0; i<m_ItemsetSizeB; i++){
				if (cht[i]<chs[i]) return true;
				if (cht[i]>chs[i]) return false;
			};
			return true;
		};

		bool Grt(int tc, int ti, int sc, int si){
			#ifdef ITEMSETS_DEBUG
				AssertValid(tc,ti);
				AssertValid(sc,si);
			#endif
			UCHAR *cht=m_Chunks[tc].GetFirstByte(ti);
			UCHAR *chs=m_Chunks[sc].GetFirstByte(si);
			for (int i=0; i<m_ItemsetSizeB; i++){
				if (cht[i]>chs[i]) return true;
				if (cht[i]<chs[i]) return false;
			};
			return false;
		};
		
		bool GrtEq(int tc, int ti, int sc, int si){
			#ifdef ITEMSETS_DEBUG
				AssertValid(tc,ti);
				AssertValid(sc,si);
			#endif
			UCHAR *cht=m_Chunks[tc].GetFirstByte(ti);
			UCHAR *chs=m_Chunks[sc].GetFirstByte(si);
			for (int i=0; i<m_ItemsetSizeB; i++){
				if (cht[i]>chs[i]) return true;
				if (cht[i]<chs[i]) return false;
			};
			return true;
		};	

		bool IsSubsetOf(int tc, int ti, int sc, int si){
			#ifdef ITEMSETS_DEBUG
				AssertValid(tc,ti);
				AssertValid(sc,si);
			#endif
			UCHAR *cht=m_Chunks[tc].GetFirstByte(ti);
			UCHAR *chs=m_Chunks[sc].GetFirstByte(si);
			for (int i=0; i<m_ItemsetSizeB; i++)
				if ((cht[i] & chs[i]) != cht[i])
					return false;
			return true;
		};

		bool IntersectionIsEmpty(int tc, int ti, int sc, int si){
			#ifdef ITEMSETS_DEBUG
				AssertValid(tc,ti);
				AssertValid(sc,si);
			#endif
			UCHAR *cht=m_Chunks[tc].GetFirstByte(ti);
			UCHAR *chs=m_Chunks[sc].GetFirstByte(si);
			for (int i=0; i<m_ItemsetSizeB; i++)
				if ((cht[i] & chs[i])!=0)
					return false;
			return true;
		};

		void Intersect(int tc, int ti, int sc, int si){
			#ifdef ITEMSETS_DEBUG
				AssertValid(tc,ti);
				AssertValid(sc,si);
			#endif
			UCHAR *cht=m_Chunks[tc].GetFirstByte(ti);
			UCHAR *chs=m_Chunks[sc].GetFirstByte(si);
			for (int i=0; i<m_ItemsetSizeB; i++)
				cht[i] = cht[i] & chs[i];
		};

		void Union(int tc, int ti, int sc, int si){
			#ifdef ITEMSETS_DEBUG
				AssertValid(tc,ti);
				AssertValid(sc,si);
			#endif
			UCHAR *cht=m_Chunks[tc].GetFirstByte(ti);
			UCHAR *chs=m_Chunks[sc].GetFirstByte(si);
			for (int i=0; i<m_ItemsetSizeB; i++)
				cht[i] = cht[i] | chs[i];
		};

		void CreateComplement(int tc, int ti, int sc, int si){
			#ifdef ITEMSETS_DEBUG
				AssertValid(tc,ti);
				AssertValid(sc,si);
			#endif
			UCHAR *cht=m_Chunks[tc].GetFirstByte(ti);
			UCHAR *chs=m_Chunks[sc].GetFirstByte(si);
			for (int i=0; i<m_ItemsetSizeB; i++) cht[i] = 255 ^ chs[i];
		};

		int TrueCount(int c, int i){
			#ifdef ITEMSETS_DEBUG
				AssertValid(c,i);
			#endif
			UCHAR *ch=m_Chunks[c].GetFirstByte(i);
			int sum=0;
			for (int j=0; j<m_ItemsetSizeB; j++) sum += SetBitRet[ch[j]];
			return sum;
		};

		void RemoveElems(int tc, int ti, int sc, int si){
			#ifdef ITEMSETS_DEBUG
				AssertValid(tc,ti);
				AssertValid(sc,si);
			#endif
			UCHAR *cht=m_Chunks[tc].GetFirstByte(ti);
			UCHAR *chs=m_Chunks[sc].GetFirstByte(si);
			for (int i=0; i<m_ItemsetSizeB; i++)
				cht[i] = cht[i] & (255 ^ chs[i]);
		};
		
		bool Empty(int c, int i){
			#ifdef ITEMSETS_DEBUG
				AssertValid(c,i);
			#endif
			UCHAR *ch=m_Chunks[c].GetFirstByte(i);
			for (int j=0; j<m_ItemsetSizeB; j++)
				if (ch[j]!=0) return false;
			return true;
		};

		void SetItem(int c, int i, int b){
			#ifdef ITEMSETS_DEBUG
				AssertValid(c,i);
			#endif
			UCHAR *ch=m_Chunks[c].GetFirstByte(i);
			ch[b/8] = ch[b/8] | mask2[b%8];
		};

		void ResetItem(int c, int i, int b){
			#ifdef ITEMSETS_DEBUG
				AssertValid(c,i);
			#endif
			UCHAR *ch=m_Chunks[c].GetFirstByte(i);
			ch[b/8] = ch[b/8] & (255 ^ mask2[b%8]);
		};

		bool IsSetItem(int c, int i, int b){
			#ifdef ITEMSETS_DEBUG
				AssertValid(c,i);
			#endif
			UCHAR *ch=m_Chunks[c].GetFirstByte(i);
			if ((ch[b/8] & mask2[b%8]) == 0) return false;
			return true;
		};

		void SetAllTrue(int c, int i){
			#ifdef ITEMSETS_DEBUG
				AssertValid(c,i);
			#endif
			UCHAR *ch=m_Chunks[c].GetFirstByte(i);
			memset(ch,255,m_ItemsetSizeB);
		};

		int FirstItem(int c, int i){		
			#ifdef ITEMSETS_DEBUG
				AssertValid(c,i);
			#endif
			UCHAR *ch=m_Chunks[c].GetFirstByte(i);
			for (int j=0; j<m_ItemsetSizeB; j++){
				if (ch[j]!=0){
					for (int k=0; k<8; k++)
						if ((ch[j] & mask2[k])!=0) return j*8+k;
				};
			};
			return -1;
		};

		friend struct TItemset;
	};

	extern TUCharManager m_glbUCharManager; // global variable - should be initialised before any TItemset if created

	///////////////////////////////////////////////////////////////
	// struct TItemset
	///////////////////////////////////////////////////////////////

	struct TItemset{
		int m_Freq;			// frequency of this itemset
		int m_Chunk;		// chunk index
		int m_Index;		// uchar index in a chunk

		TItemset(){
			//if (m_glbUCharManager.NewItemset(m_Chunk,m_Index)==false){ m_Chunk=m_Index=-1; };
			m_Chunk=m_Index=-1;
			m_Freq=0;
		};

		TItemset(const TItemset &x){
			if (m_glbUCharManager.NewItemset(m_Chunk,m_Index)==false){
				m_Chunk=m_Index=-1; 
				m_Freq=0;
			}
			else{ Copy(x); };
		};

		~TItemset(){
			Reset();		
		};

		void Init(){
			if ((m_Chunk==-1)&&(m_Index==-1)){
				m_glbUCharManager.NewItemset(m_Chunk,m_Index);
				Clear();
			};
		};

		void Reset(){
			if ((m_Chunk!=-1)&&(m_Index!=-1)){
				m_glbUCharManager.RemoveItemset(m_Chunk,m_Index);
				m_Chunk=m_Index=-1;
			};
		};

		void Clear(){                                   // sets everything to 0
			m_glbUCharManager.Clear(m_Chunk,m_Index);
			m_Freq=0;
		};

		void Copy(const TItemset &x){                   // copy data from r
			Init();
			m_glbUCharManager.Copy(m_Chunk,m_Index,x.m_Chunk,x.m_Index);
			m_Freq=x.m_Freq; 
		};

		bool operator==(const TItemset &x){
			return m_glbUCharManager.Eq(m_Chunk,m_Index,x.m_Chunk,x.m_Index);
		};

		bool operator<(const TItemset &x){
			return m_glbUCharManager.Lwr(m_Chunk,m_Index,x.m_Chunk,x.m_Index);
		};

		bool IsLwr(const TItemset &x){
			return m_glbUCharManager.Lwr(m_Chunk,m_Index,x.m_Chunk,x.m_Index);
		};

		bool operator<=(const TItemset &x){
			return m_glbUCharManager.LwrEq(m_Chunk,m_Index,x.m_Chunk,x.m_Index);
		};

		bool operator>(const TItemset &x){
			return m_glbUCharManager.Grt(m_Chunk,m_Index,x.m_Chunk,x.m_Index);
		};

		bool operator>=(const TItemset &x){
			return m_glbUCharManager.GrtEq(m_Chunk,m_Index,x.m_Chunk,x.m_Index);
		};

		TItemset& operator=(const TItemset &x){ Copy(x); return *this; };

		bool IsSubsetOf(const TItemset &x){ // returns true if this is a subset of r
			return m_glbUCharManager.IsSubsetOf(m_Chunk,m_Index,x.m_Chunk,x.m_Index);
		};

		bool IntersectionIsEmpty(const TItemset &x){ // returns true if intersection of this and r is empty
			return m_glbUCharManager.IntersectionIsEmpty(m_Chunk,m_Index,x.m_Chunk,x.m_Index);
		};

		void Intersect(const TItemset &x){
			m_glbUCharManager.Intersect(m_Chunk,m_Index,x.m_Chunk,x.m_Index);
		};

		void Union(const TItemset &x){
			m_glbUCharManager.Union(m_Chunk,m_Index,x.m_Chunk,x.m_Index);
		};
		
		void CreateComplement(const TItemset &x){
			m_glbUCharManager.CreateComplement(m_Chunk,m_Index,x.m_Chunk,x.m_Index);
		};

		int TrueCount(){
			return m_glbUCharManager.TrueCount(m_Chunk,m_Index);
		};

		void RemoveElems(const TItemset &x){
			m_glbUCharManager.RemoveElems(m_Chunk,m_Index,x.m_Chunk,x.m_Index);
		};

		bool Empty(){
			return m_glbUCharManager.Empty(m_Chunk,m_Index);
		};

		void SetItem(int i){
			m_glbUCharManager.SetItem(m_Chunk,m_Index,i);
		};

		void ResetItem(int i){
			m_glbUCharManager.ResetItem(m_Chunk,m_Index,i);
		};

		bool IsSetItem(int i){
			return m_glbUCharManager.IsSetItem(m_Chunk,m_Index,i);
		};

		void SetAllTrue(){
			m_glbUCharManager.SetAllTrue(m_Chunk,m_Index);
		};

		int FirstItem(){
			return m_glbUCharManager.FirstItem(m_Chunk,m_Index);
		};

		int ItemsCount(){
			return m_glbUCharManager.GetItemsetSize();
		};

		UCHAR GetByte(int i) const{
			return m_glbUCharManager.m_Chunks[m_Chunk].GetFirstByte(m_Index)[i];
		};

		// VJIDBG
		void PrintF(){
		int i;
			for (i=0; i<m_glbUCharManager.GetItemsetSize(); i++){
				if (IsSetItem(i)){ m_glbUCharManager.m_Common->Notify("1"); }
				else{ m_glbUCharManager.m_Common->Notify("0"); }
			};
			m_glbUCharManager.m_Common->Notify("x");
			for (i=m_glbUCharManager.GetItemsetSize(); i<m_glbUCharManager.GetItemsetSizeB()*8; i++){
				if (IsSetItem(i)){ m_glbUCharManager.m_Common->Notify("1"); }
				else{ m_glbUCharManager.m_Common->Notify("0"); }
			};
			char bf[100];
			sprintf(bf," freq=%d chunk=%d index=%d FirstByte=%d\n",m_Freq,m_Chunk,m_Index, m_glbUCharManager.m_Chunks[m_Chunk].GetFirstByte(m_Index)[0]);
			m_glbUCharManager.m_Common->Notify(bf);
		};
		// end VJIDBG
	};

	//////////////////////////////////////////////////////////////////////////////

	inline void SwapItemsets(TItemset &x, TItemset &y){
		int c=y.m_Chunk;
		int i=y.m_Index;
		int f=y.m_Freq;
		y.m_Chunk=x.m_Chunk;
		y.m_Index=x.m_Index;
		y.m_Freq=x.m_Freq;
		x.m_Chunk=c;
		x.m_Index=i;
		x.m_Freq=f;
	};


	////////////////////////////////////////////////////////////////////

	struct TIndexNode{

		// data members
		int m_TargetByte;
		int m_LastByte;
		VikStd::TVec<TIndexNode*> m_v;
		VikStd::TVec<int> m_vb;

		// constructio, destruction
		TIndexNode(){ m_TargetByte=-1; m_LastByte=-1; };

		~TIndexNode(){
			if (m_TargetByte<m_LastByte)
				for (int i=0; i<255; i++)
					if (m_v[i]!=NULL) 
						delete m_v[i]; 
		};

		// methods
		
		void Init(int b, int last){
			m_v.Clr();
			m_vb.Clr();
			m_TargetByte=b;
			m_LastByte=last;
			if (b<last){ for (int i=0; i<255; i++) m_v.Add(NULL); }
			else{ for (int i=0; i<255; i++) m_vb.Add(-1); };
		};

		bool Add(const TItemset &r, int index){
			if (m_TargetByte<0) return false;
			UCHAR b=r.GetByte(m_TargetByte);
			if (m_TargetByte<m_LastByte){
				if (m_v[b]==NULL){
					m_v[b]=new TIndexNode();
					m_v[b]->Init(m_TargetByte+1,m_LastByte);
				};
				return m_v[b]->Add(r,index);
			}
			else{
				bool res=false;
				if (m_vb[b]>=0) res=true;
				m_vb[b]=index;
				return res;
			};
		};

		int Find(const TItemset &r){
			if (m_TargetByte<0) return -1;
			UCHAR b=r.GetByte(m_TargetByte);
			if (m_TargetByte<m_LastByte){
				if (m_v[b]==NULL) return -1;
				return m_v[b]->Find(r);
			}
			else return m_vb[b];
		};

		void Clear(){
			if (m_TargetByte<0) return;
			if (m_TargetByte<m_LastByte){
				for (int i=0; i<255; i++)
					if (m_v[i]!=NULL) delete m_v[i]; 
			};
			if (m_TargetByte<m_LastByte){ for (int i=0; i<255; i++) m_v[i]=NULL; }
			else{ for (int i=0; i<255; i++) m_vb[i]=-1; };
		};
	};

	//////////////////////

	struct TIndex{
		TIndexNode m_Root;
		
		//TIndex(int AllBytes): m_Root(0,AllBytes-1){};
		TIndex(){};

		void Init(int AllBytes){ m_Root.Init(0,AllBytes); };
		bool Add(const TItemset &r, int index){ return m_Root.Add(r,index); }
		int Find(const TItemset &r){ return m_Root.Find(r); }
		void Clear(){ m_Root.Clear(); };
	};

	////////////////////////////////////////////////////////////////////
	// ListOfItemsets - holds itemsets, that must be proccessed, in a vector
	// special sort rutines are provided
	////////////////////////////////////////////////////////////////////

	class TListOfItemsets{

		TIndex *m_Index;

		int GetPivotValN(const int& LValN, const int& RValN){
			int ValN1=LValN;
			int ValN2=RValN;
			int ValN3=(LValN+RValN)/2;
			if (m_v[ValN1]<m_v[ValN2]){
				if (m_v[ValN2]<m_v[ValN3]){ return ValN2; }
				else if (m_v[ValN3]<m_v[ValN1]){ return ValN1; }
				else { return ValN3; };
			} 
			else {
				if (m_v[ValN1]<m_v[ValN3]){ return ValN1; }
				else if (m_v[ValN3]<m_v[ValN2]){ return ValN2; }
				else { return ValN3; };
			};
		};

		void DumbSort(int l, int r){ // bubble-sort
			bool AnyChange=true;
			int i=l;
			while ((i<r)&&(AnyChange)){
				AnyChange=false;
				for (int j=r; j>i; j--){
					if (m_v[j]<m_v[j-1]){
						SwapItemsets(m_v[j],m_v[j-1]);
						AnyChange=true;
					};
				};
				i++;
			};
		};

	  void ISort(const int& MnLValN, const int& MxRValN){ // iterative sort
			if (MnLValN<MxRValN){
				for (int ValN1=MnLValN+1; ValN1<=MxRValN; ValN1++){
					int ValN2=ValN1;
					while ((ValN2>MnLValN)&&(m_v[ValN2-1]>m_v[ValN2])){
						SwapItemsets(m_v[ValN2],m_v[ValN2-1]); 
						ValN2--;
					};
				};
			};
		};

	  int Partition(const int& MnLValN, const int& MxRValN){
			int PivotValN=GetPivotValN(MnLValN, MxRValN);
			SwapItemsets(m_v[PivotValN],m_v[MnLValN]);
			TItemset PivotVal;
			PivotVal.Init();
			PivotVal=m_v[MnLValN];

			int LValN=MnLValN; int RValN=MxRValN+1;
			do {
				do {RValN--;} while ((RValN>MnLValN)&&(m_v[RValN]>=PivotVal));
				do {LValN++;} while ((LValN<=MxRValN)&&(m_v[LValN]<=PivotVal));

				if (LValN<RValN){ SwapItemsets(m_v[LValN],m_v[RValN]); }
				else {
					if (RValN<MnLValN) return -1;
					//if (LValN>=MxRValN) return -1;
					//else{ return RValN; };
					return RValN;
				};
			} 
			while (true);
		};

		/*int Partition2(const int& MnLValN, const int& MxRValN){

			int PivotValN=GetPivotValN(MnLValN, MxRValN);
			SwapItemsets(m_v[PivotValN],m_v[MnLValN]);
			
			if ((MnLValN==2286)||(MxRValN==2343)||(MnLValN==2285)||(MxRValN==2344))
				int zz=0;

			int LValN=MnLValN+1; 
			int RValN=MxRValN;
			bool Go=true;
			while (Go){		
				while ((RValN>MnLValN)&&(m_v[RValN]>m_v[MnLValN])) RValN--;
				while ((LValN<=MxRValN)&&(m_v[LValN]<m_v[MnLValN])) LValN++;

				if (LValN<RValN){ SwapItemsets(m_v[LValN],m_v[RValN]); }
				else {
					if (RValN<MnLValN) return RValN;
					if (LValN>=MxRValN) return LValN;
					return RValN;
				};
			};
			return -1;
		};

		void QSort2(const int& MnLValN, const int& MxRValN){ // quick sort
			if (MnLValN<MxRValN){
				if (MxRValN-MnLValN<20){
					DumbSort(MnLValN, MxRValN);
				} 
				else {
					int l;
					l = Partition2(MnLValN, MxRValN);
					QSort(MnLValN,l);
					QSort(l+1,MxRValN);
				};
			};
		};*/


	  void QSort(const int& MnLValN, const int& MxRValN){ // quick sort
			if (MnLValN<MxRValN){
				if (MxRValN-MnLValN<20){
					ISort(MnLValN, MxRValN);
				} 
				else {
					int SplitValN=Partition(MnLValN, MxRValN);
					if (SplitValN>=0){
						if (SplitValN==MxRValN) SplitValN--;
						QSort(MnLValN,SplitValN);
						QSort(SplitValN+1,MxRValN);
					};
				};
			};
		};

		int SearchBisect(TItemset &Val) const{ // find (with bisection) index of given value. -1 if not found.
			int l=0, r=m_v.Len()-1, s;
			while (l<r){
				if (Val==m_v[l]) return l;
				if (Val==m_v[r]) return r;
				s=(l+r)/2;
				if (Val.IsLwr(m_v[s])!=false){ r=s; }
				else{
					if (l<s){ l=s; }
					else{ l++; };
				};
			};		
			return -1;
		};

		int SearchLin(TItemset &Val){ // find (with linear search) index of given value. -1 if not found.
			/*for (int i=0; i<m_v.Len(); i++)
				if (Val==m_v[i])
					return 1;
			return -1;*/
			return m_Index->Find(Val);
		};

		VikStd::TVec<TItemset> m_v; // vector of itemsets

	public:

		TListOfItemsets(){ m_Index=NULL; };
		~TListOfItemsets(){
			if (m_Index!=NULL) delete m_Index;
		};

		void Add(TItemset &t){
			m_v.Add();
			m_v.Last().Init();
			m_v.Last()=t;
		};

		void Add(){
			m_v.Add();
			m_v.Last().Init();
		}; 

		TItemset& operator[](int i){ return m_v[i]; };
		TItemset& At(int i){ return m_v[i]; };

		TItemset& Last(){ return m_v.Last(); }; // get reference to last itemset
		
		void DeleteLast(){
			m_v.Last().Reset();
			m_v.DeleteLast();
		};
		
		int Len(){ return m_v.Len(); }; // length of list

		void Sort(){ // calls specialised sorting rutines
			QSort(0,m_v.Len()-1);
			//DumbSort(0,m_v.Len()-1);
			//QSort2(0,m_v.Len()-1);
		}; 
		
		int Find(TItemset &r){
			//return SearchLin(r);
			return SearchBisect(r); // find itemset with bisection
		};
		
		void SetAllFreqZero(){ // sets frequencies of all itemsets to zero
			for (int i=0; i<m_v.Len(); i++) 
				m_v[i].m_Freq=0; 
		};

		bool Empty(){ return m_v.Empty(); };

		void Clear(){ m_v.Clear(); };

		void CreateIndex(){
			/*if (m_Index!=NULL) delete m_Index;
			m_Index = new TIndex;
			m_Index->Init(m_glbUCharManager.GetItemsetSizeB()-1);
			m_Index->Clear();
			for (int i=0; i<m_v.Len(); i++)
				m_Index->Add(m_v[i],i);
			*/
		};

		void PrintF(){
			for (int i=0; i<m_v.Len(); i++){
				VikStd::TStr s;
				s=i;
				s+=": ";
				m_glbUCharManager.m_Common->Notify(s);
				m_v[i].PrintF();
			};
		};
	};


	////////////////////////////////////////////////////////////////////

}
