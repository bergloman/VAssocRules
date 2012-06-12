#ifndef CLUSTERING_H
#define CLUSTERING_H

//////////////////////////////////////////////////////////////////////////////

#include "stdvik.h"

namespace VikClustering{

//////////////////////////////////////////////////////////////////////////////

class TClusterItem{
public:
	  
	VikStd::TAttrType m_AttrType;

	// discrete
	VikStd::TVec<double> m_Probs;
	
	// countinuous
	double m_Val;

	// constructors
	TClusterItem(){ m_Probs.Clear(); m_Val=0; };

	TClusterItem(const TClusterItem &ci){
		m_AttrType=ci.m_AttrType;
		m_Val=ci.m_Val;
		m_Probs.CopyFrom(ci.m_Probs);
	};

	void operator=(const TClusterItem &ci){
		m_AttrType=ci.m_AttrType;
		m_Val=ci.m_Val;
		m_Probs.CopyFrom(ci.m_Probs);
	};

	void InitDiscr(int count){ // initialise probabilities for different values
		m_AttrType=VikStd::atDiscr;
		VikStd::TRnd rnd;
		m_Probs.Clear();
		for (int i=0; i<count; i++) m_Probs.Add(0);
		m_Probs[rnd.GetInt(count)]=rnd.GetFlt();
	};

	void InitCon(){ // initialise value somewhere between -1 and 1
		VikStd::TRnd rnd;
		m_AttrType=VikStd::atCon;
		m_Val=2*rnd.GetFlt()-1;
	};

	void Reset(){
		m_Val=0;
		for (int i=0; i<m_Probs.Len(); i++) m_Probs[i]=0;
	};

	void Finalise(int count){
		if (count>0){
			m_Val/=count;
			for (int i=0; i<m_Probs.Len(); i++) m_Probs[i]/=count;
		}
		else{ // this cluster diverged - update data with random values
			if (m_AttrType==VikStd::atDiscr){ InitDiscr(m_Probs.Len()); }
			else{ InitCon(); };
		};
	};

	int AddValue(VikStd::TPair<int,double> pair){		
		if (m_AttrType==VikStd::atDiscr){
			if (pair.m_c1>=0){
				m_Probs[pair.m_c1]++; 
				return 1; 
			};		
		}
		else{
			if (pair.m_c1>=0){
				m_Val+=pair.m_c2; 
				return 1; 
			};		
		};
		return 0;
	};
	/*int AddValue(TDataTable *tab, int ex, int attr){
		if (m_AttrType==atErr) throw TExc("TClusterItem.AddValue: item not initialised, m_AttrType==atErr.");
		if (m_AttrType==atDiscr){
			if (tab->Attrs[attr].iVec[ex]>=0){ m_Probs[tab->Attrs[attr].iVec[ex]]++; return 1; };
		}
		else{
			if (tab->Attrs[attr].DoubleIsMissing(ex)==false){ m_Val+=tab->Attrs[attr].dVec[ex]; return 1; };
		};
		return 0;
	};*/

};

//////////////////////////////////////////////////////////////////////////////

class TCluster{
public:
	VikStd::TVec<TClusterItem> m_Items; // descriptions for different attributes
	TCluster(){};
	TCluster(const TCluster &cl){ m_Items.CopyFrom(cl.m_Items); };
};

//////////////////////////////////////////////////////////////////////////////

class TTransformAttr{
public:

	// for continuous attribute
	double m_Mean;
	double m_Dev;

	// discrete attribute
	int m_ValCount;
	
	// common data
	int m_Attr;
	VikStd::TAttrType m_AttrType;
	double m_Weight;

	// for AssignToClusters only
	VikStd::TStr m_AttrName;
	VikStd::TVec<VikStd::TStr> m_ValueNames;
	VikStd::TVec<int> m_ValueIndex;

	///////////////////////

	TTransformAttr(){
		m_Mean=0.0;
		m_Dev=0.0;
		m_Weight=1.0;
		m_ValCount=0;
		m_Attr=-1;
		m_AttrType=VikStd::atErr;
	};

	///////////////////////

	TTransformAttr(const TTransformAttr &ta){
		m_Mean=ta.m_Mean;
		m_Dev=ta.m_Dev;
		m_Weight=ta.m_Weight;
		m_ValCount=ta.m_ValCount;
		m_Attr=ta.m_Attr;
	};
	
	///////////////////////

	void Init(VikStd::TAttribute &at, int index, double weight){
		if (at.AttrType==VikStd::atDiscr){ InitDiscrete(at,index,weight); }
		else{ InitCon(at,index,weight); };
	};

	///////////////////////

	void InitDiscrete(VikStd::TAttribute &at, int index, double weight){
		if (at.AttrType!=VikStd::atDiscr)
			throw VikStd::TExc("TClusterItem.InitDiscrete: supplied attribute is not discrete.");
		m_Attr=index;
		m_Weight=weight;
		m_ValCount=at.ValNames.Len();
		m_AttrType=VikStd::atDiscr;
	};

	///////////////////////

	void InitCon(VikStd::TAttribute &at, int index, double weight){ // init m_Mean and m_Dev
		if (at.AttrType!=VikStd::atCon) 
            throw VikStd::TExc("TClusterItem.InitCon: supplied attribute is not continuous.");
		m_Attr=index;
		m_Weight=weight;
		m_Mean=0;
		m_AttrType=VikStd::atCon;
		int Valid=0;
		for (int i=0; i<at.dVec.Len(); i++){
			if (at.DoubleIsMissing(i)==false){ m_Mean+=at.dVec[i]; Valid++; };
		};
		m_Mean/=Valid;
		
		double F=0,S=0;
        int i;
		for (i=0; i<at.dVec.Len(); i++){
			if (at.DoubleIsMissing(i)==false){
				F+=SQR(at.dVec[i]);
				S+=at.dVec[i];
			};
		};
		S=SQR(S);
		m_Dev=(Valid*F-S)/(Valid*(Valid-1));
		m_Dev=sqrt(m_Dev);
	};

	///////////////////////

	VikStd::TPair<int,double> GetPair(VikStd::TDataTable *tab,int ex){
		VikStd::TPair<int,double> res;
		res.m_c1=-1;
		if (m_AttrType==VikStd::atDiscr){
			res.m_c1=tab->Attrs[m_Attr].iVec[ex];
		}
		else{
			if (tab->Attrs[m_Attr].DoubleIsMissing(ex)==false){
				res.m_c1=1;
				res.m_c2=(tab->Attrs[m_Attr].dVec[ex]-m_Mean)/m_Dev;
			};
		};
		return res;
	};

	///////////////////////

	double GetSQRDistance(TClusterItem &cl, VikStd::TDataTable *tab, int ex){ // returns squared distance of centroid from example
		double res;
		if (m_AttrType==VikStd::atDiscr){
			res=0;
			int t=tab->Attrs[m_Attr].iVec[ex];
			if (t>=0){
				for (int i=0; i<cl.m_Probs.Len(); i++){
					if (i==t){ res+=SQR(1-cl.m_Probs[i]); }
					else{ res+=SQR(cl.m_Probs[i]); };
				};
			};
		}
		else{
			if (tab->Attrs[m_Attr].DoubleIsMissing(ex)==false){
				res=tab->Attrs[m_Attr].dVec[ex]-m_Mean;
				res/=m_Dev;
				res=SQR(res-cl.m_Val);
			};
		};
		return m_Weight*res;
	};

	///////////////////////

	// returns squared distance of centroid from example - for assigning to clusters

	double GetSQRDistance2(TClusterItem &cl, VikStd::TDataTable *tab, int ex){
		double res;
		if (m_AttrType==VikStd::atDiscr){
			res=0;
			int t=tab->Attrs[m_Attr].iVec[ex];
			if (t>=0){ // if value not missing
				for (int i=0; i<cl.m_Probs.Len(); i++){
					if (m_ValueIndex[i]==t){ res+=SQR(1-cl.m_Probs[i]); }
					else{ res+=SQR(cl.m_Probs[i]); };
				};
			};
		}
		else{
			if (tab->Attrs[m_Attr].DoubleIsMissing(ex)==false){
				res=tab->Attrs[m_Attr].dVec[ex]-m_Mean;
				res/=m_Dev;
				res=SQR(res-cl.m_Val);
			};
		};
		return m_Weight*res;
	};

	///////////////////////

	VikStd::TStr GetDescription(VikStd::TDataTable *tab){
		VikStd::TStr res;

		if (m_AttrType==VikStd::atDiscr){
			res="<ClusterAttr> <AttrType>d</AttrType> <weight>";
			res+=m_Weight;
			res+="</weight> <name>";
			res+=tab->Attrs[m_Attr].AttrName;
			res+="</name> <dvalues>";
			for (int i=0; i<m_ValCount; i++){
				res += tab->Attrs[m_Attr].ValNames[i];
				if (i<m_ValCount-1) res+=" ";
			};
			res+="</dvalues> <mean>0</mean> <dev>0</dev>";
			res+="</ClusterAttr>";
			
		} else if (m_AttrType==VikStd::atCon){

			res="<ClusterAttr> <AttrType>c</AttrType> <weight>";
			res+=m_Weight;
			res+="</weight> <name>";
			res+=tab->Attrs[m_Attr].AttrName;
			res+="</name> <dvalues>";
			for (int i=0; i<m_ValCount; i++){
				res += tab->Attrs[m_Attr].ValNames[i];
				if (i<m_ValCount-1) res+=" ";
			};
			res+="</dvalues> <mean>";
			res+=VikStd::TStr(m_Mean)+"</mean> <dev>";
			res+=VikStd::TStr(m_Dev)+"</dev>";
			res+="</ClusterAttr>";
		}		else { 
            res="<ClusterAttr><AttrType>error</AttrType></ClusterAttr>"; 
        };
		return res;
	};
};

/////////////////////////////////////////////////////////////////////
// Object that performs K-Means clustering
/////////////////////////////////////////////////////////////////////

class TKMeans{

private:

	VikStd::TDataTable *m_Tab;
	VikStd::TVec<TTransformAttr> m_Transformations;
	VikStd::TVec<TCluster> m_Clusters;
	VikStd::TVec<int> m_ClusterMembership;
	VikStd::TVec<int> m_ClusterCount; // number of members of each cluster

	void InitialiseClusters(); // initialise clusters with given data properties

	double GetDistance(int ex, int cl){
		double res=0;
		for (int i=0; i<m_Transformations.Len(); i++)
			res+=m_Transformations[i].GetSQRDistance(m_Clusters[cl].m_Items[i],m_Tab,ex);
		return res;
	};

	double GetDistance2(int ex, int cl){
		double res=0;
		for (int i=0; i<m_Transformations.Len(); i++)
			res+=m_Transformations[i].GetSQRDistance2(m_Clusters[cl].m_Items[i],m_Tab,ex);
		return res;
	};

	bool ReassignToClusters();	
	void UpdateCentroids();

	void ParseAtt(VikStd::THTMLParser &HtmlParser);
	void ParseCluster(VikStd::THTMLParser &HtmlParser);

public:
	
	int m_NumOfClusters;
	int m_MaxIterations;
	bool m_DoCV;

	TKMeans(VikStd::TDataTable *Tab);
	void DoClustering();
	VikStd::TStr GetDescAsXML();

	void ReadFromXMLFile(VikStd::TStr &ClustersFile);
	void AssignToClusters(VikStd::TStr &AttrName); // add new column to m_Tab that indicates cluster membership
	bool SaveTabToFile(VikStd::TStr &FileName){ // saved m_Tab to specified file
		return m_Tab->SaveToFileTab(FileName);
	};
};

//////////////////////////////////////////////////////////

}

#endif
