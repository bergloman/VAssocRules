#include "Clustering.h"

namespace VikClustering{

/////////////////////////////////////////////////////////////////

void TKMeans::InitialiseClusters(){ // initialise clusters with given data properties
	VikStd::TRnd rnd;
	for (int i=0; i<m_NumOfClusters; i++){			
		int ex=rnd.GetInt(m_Tab->Attrs[0].Len());
		TCluster cl;
		for (int j=0; j<m_Transformations.Len(); j++){
			TClusterItem ci;
			if (m_Transformations[j].m_AttrType==VikStd::atDiscr){
				ci.InitDiscr(m_Transformations[j].m_ValCount);
				if (m_Tab->Attrs[j].iVec[ex]>=0)	ci.m_Probs[m_Tab->Attrs[j].iVec[ex]]=1;
			}
			else{
				ci.InitCon();
				if (m_Tab->Attrs[j].DoubleIsMissing(ex)==false) ci.m_Val=m_Transformations[j].GetPair(m_Tab,ex).m_c2;
			};
			cl.m_Items.Add(ci);
		};
		m_Clusters.Add(cl);
	};


    m_ClusterMembership.ResizeX(m_Tab->Attrs[0].Len());
    for (int j=0; j<m_ClusterMembership.Len(); j++)
        m_ClusterMembership[j] = -1;         
};

/////////////////////////////////////////////////////////////////

bool TKMeans::ReassignToClusters(){
	bool res=false;

    int i, j;
    double all=0;
    VikStd::TRnd rnd;
    VikStd::TVec<VikStd::TPairDoubleInt> Distances;
    Distances.ResizeX(m_Clusters.Len());

	for (i=0; i<m_ClusterMembership.Len(); i++){ // for each example

        for (j=0; j<m_Clusters.Len(); j++){ // for each cluster
            Distances[j].m_c1=GetDistance(i,j);
            Distances[j].m_c2=j;
        };

        Distances.Sort();
        for (j=1; ((j<Distances.Len()) && (Distances[j].m_c1==Distances[0].m_c1)); j++); // find number of same distances
        
        int Cluster = Distances[rnd.GetInt(j)].m_c2;
        res=(res||(m_ClusterMembership[i]!=Cluster));
		m_ClusterMembership[i]=Cluster;
	};
	return res;
};

/////////////////////////////////////////////////////////////////

void TKMeans::UpdateCentroids(){
    int count;
	for (int i=0; i<m_Clusters.Len(); i++){ // for each cluster		
		for (int j=0; j<m_Clusters[i].m_Items.Len(); j++){ // for each attribute
			m_Clusters[i].m_Items[j].Reset();
			count=0;
			for (int k=0; k<m_ClusterMembership.Len(); k++){ // for each example
				if (m_ClusterMembership[k]==i){
					count+=m_Clusters[i].m_Items[j].AddValue(m_Transformations[j].GetPair(m_Tab,k));
				};
			};
			m_Clusters[i].m_Items[j].Finalise(count);
		};
	};
};

/////////////////////////////////////////////////////////////////

TKMeans::TKMeans(VikStd::TDataTable *Tab){
	m_MaxIterations=50;
	m_NumOfClusters=-1;
	m_DoCV=false;
	m_Tab=Tab;
	for (int i=0; i<Tab->Attrs.Len(); i++){			
		m_Transformations.Add();
		m_Transformations.Last().Init(Tab->Attrs[i],i,1.0);
	};
	m_ClusterMembership.Gen(m_Tab->Attrs[0].Len());
};

/////////////////////////////////////////////////////////////////

void TKMeans::DoClustering(){
	if (m_NumOfClusters<1){
		if (m_DoCV==false){
			throw VikStd::TExc("TKMeans.DoClustering: m_NumOfClusters<1.");
		};
	};
	if (m_NumOfClusters>m_Tab->Attrs[0].Len())
		throw VikStd::TExc("TKMeans.DoClustering: m_NumOfClusters is greater than number of instances.");

    FILE *fp = fopen("output.txt","w");
    try{
        for (int i=0; i<m_Tab->Attrs[0].Len(); i++)
            fprintf(fp,"%s\n",m_Tab->Attrs[0].ValueAsStr(i).c_str());
        fclose(fp);
    }
    catch(...){ fclose(fp); };

	InitialiseClusters();		
	int Iter=0;
	bool Go=true;
	while (Go){
        printf("Performing %d-th iteration.\n",Iter);
		Go=ReassignToClusters();						
		if (Go){ 
			Iter++;
			UpdateCentroids();
			Go=(Iter<m_MaxIterations);
		};
	};
	int i;
	for (i=0; i<m_NumOfClusters; i++) m_ClusterCount.Add(0);
	for (i=0; i<m_ClusterMembership.Len(); i++) m_ClusterCount[m_ClusterMembership[i]]++;
};

/////////////////////////////////////////////////////////////////

VikStd::TStr TKMeans::GetDescAsXML(){
	VikStd::TStr res;
	res="<clustering>\n\t<ClusterAttr-list>\n";
	int i;
	for (i=0; i<m_Transformations.Len(); i++){
		res+="\t\t";
		res+=m_Transformations[i].GetDescription(m_Tab);
		res+="\n";
	};
	res+="\t</ClusterAttr-list>\n\n\t<Cluster-list>\n";
	for (i=0; i<m_Clusters.Len(); i++){
		
		res+=VikStd::TStr("\t\t<Cluster><values>");			
		for (int j=0; j<m_Clusters[i].m_Items.Len(); j++){			
			if (m_Clusters[i].m_Items[j].m_AttrType==VikStd::atDiscr){
				for (int k=0; k<m_Clusters[i].m_Items[j].m_Probs.Len(); k++){
					res+=m_Clusters[i].m_Items[j].m_Probs[k];
					if (k<m_Clusters[i].m_Items[j].m_Probs.Len()-1) res+=" ";
				};
			}
			else if (m_Clusters[i].m_Items[j].m_AttrType==VikStd::atCon){
                res+=m_Clusters[i].m_Items[j].m_Val; 
            };
			if (j<m_Clusters[i].m_Items.Len()-1) res+=" ";
		};
		res+="</values><members>";
		res+=m_ClusterCount[i];
		res+="</members></Cluster>\n";
	};
	res+="\t</Cluster-list>\n</clustering>";
	return res;
};

/////////////////////////////////////////////////////////////////

void TKMeans::ReadFromXMLFile(VikStd::TStr &ClustersFile){
	
	VikStd::THTMLParser HtmlParser;
	HtmlParser.LoadFromFile(ClustersFile);
	if (HtmlParser.FindTagFromBeginning("CLUSTERING")==false)
		throw VikStd::TExc("TKMeans::ReadFromXMLFile: cannot find tag 'clustering'.");

	// VJIDBG
	HtmlParser.SaveToFile("testout.txt");

	HtmlParser.NextTag(true);
	if (HtmlParser.GetTag().GetName()!="CLUSTERATTR-LIST")
		throw VikStd::TExc(VikStd::TStr("TKMeans::ReadFromXMLFile: 'CLUSTERATTR-LIST' tag expected, found '")+HtmlParser.GetTag().GetName()+"'");
	HtmlParser.NextTag(true);

	m_Transformations.Clear();
	while (HtmlParser.GetTag().GetName()!="/CLUSTERATTR-LIST"){
		ParseAtt(HtmlParser);		
	};
	HtmlParser.NextTag(true);

	// create transformation data - m_ValueIndex of TTransformAtt
	for (int i=0; i<m_Transformations.Len(); i++){
		TTransformAttr *tr=&m_Transformations[i];
		tr->m_Attr=m_Tab->FindAttrWithName(tr->m_AttrName);
		if (tr->m_Attr<0) throw VikStd::TExc(VikStd::TStr("TKMeans::ReadFromXMLFile: using non-existing attribute: ")+tr->m_AttrName);
		tr->m_ValueIndex.Clear();
		for (int j=0; j<tr->m_ValueNames.Len(); j++){
			tr->m_ValueIndex.Add(m_Tab->Attrs[tr->m_Attr].ValNamesFind(tr->m_ValueNames[j]));
		};
	};

	if (HtmlParser.GetTag().GetName()!="CLUSTER-LIST") 
		throw VikStd::TExc(VikStd::TStr("TKMeans::ReadFromXMLFile: 'cluster-list' tag expected, found '")+HtmlParser.GetTag().GetName()+"'");
	HtmlParser.NextTag(true);

	while (HtmlParser.GetTag().GetName()!="/CLUSTER-LIST"){
		ParseCluster(HtmlParser);		
	};
	HtmlParser.NextTag(true);

	if (HtmlParser.GetTag().GetName()!="/CLUSTERING") 
		throw VikStd::TExc(VikStd::TStr("TKMeans::ReadFromXMLFile: '/clustering' tag expected, found '")+HtmlParser.GetTag().GetName()+"'");
};

/////////////////////////////////////////////////////////////////

void TKMeans::ParseAtt(VikStd::THTMLParser &HtmlParser){
	m_Transformations.Add();

	if (HtmlParser.GetTag().GetName()!="CLUSTERATTR") throw VikStd::TExc("TKMeans::ParseAtt: 'ClusterAttr' tag expected.");
	HtmlParser.NextTag(true);
		
	/* shows if all 6 values were present
	0 - type 
	1 - name
	2 - dvalues
	3 - mean
	4 - dev
	5 - weight*/
	bool IsOK[6];
	int i;
	for (i=0; i<6; i++) IsOK[i]=false;

	while (HtmlParser.GetTag().GetName()!="/CLUSTERATTR"){

		// type tag
		if (HtmlParser.GetTag().GetName()=="ATTRTYPE"){
			HtmlParser.NextTag(true);
			if (HtmlParser.GetTag().GetName()!="/ATTRTYPE")
				throw VikStd::TExc("TKMeans::ParseAtt: '/AttrType' tag expected.");
			if (HtmlParser.GetTextBetween().GetUpperCase()!="D"){
				if (HtmlParser.GetTextBetween().GetUpperCase()!="C"){
					throw VikStd::TExc(VikStd::TStr("TKMeans::ParseAtt: invalid value of 'type' tag: '")+HtmlParser.GetTextBetween()+"'.");
				} else m_Transformations.Last().m_AttrType=VikStd::atCon;
			} else m_Transformations.Last().m_AttrType=VikStd::atDiscr;
			IsOK[0]=true;
		}
		
		// name tag
		else if (HtmlParser.GetTag().GetName()=="NAME"){
			HtmlParser.NextTag(true);
			if (HtmlParser.GetTag().GetName()!="/NAME")
				throw VikStd::TExc("TKMeans::ParseAtt: '/name' tag expected.");
			if (HtmlParser.GetTextBetween().Len()==0)
				throw VikStd::TExc("TKMeans::ParseAtt: attribute name cannot be zero length.");
			
			/*char bff[2048];
			sprintf(bff,"%s",HtmlParser.GetTextBetween().c_str());
			m_Transformations.Last().m_AttrName=bff;*/
			m_Transformations.Last().m_AttrName=HtmlParser.GetTextBetween().c_str();
			IsOK[1]=true;
		}

		// dvalues tag
		else if (HtmlParser.GetTag().GetName()=="DVALUES"){
			if (IsOK[0]==false) throw VikStd::TExc("TKMeans::ParseAtt: attribute type must be specified before discrete values are given.");			
			HtmlParser.NextTag(true);
			if (HtmlParser.GetTag().GetName()!="/DVALUES")
				throw VikStd::TExc("TKMeans::ParseAtt: '/dvalues' tag expected.");			
			if (m_Transformations.Last().m_AttrType==VikStd::atDiscr){
				VikStd::TVec<VikStd::TStr> vec;
                VikStd::TStr text_between;
                text_between = HtmlParser.GetTextBetween();
				SplitIntoWords(text_between, m_Transformations.Last().m_ValueNames);
				m_Transformations.Last().m_ValCount=m_Transformations.Last().m_ValueNames.Len();
			};
			IsOK[2]=true;
		}

		// mean tag
		else if (HtmlParser.GetTag().GetName()=="MEAN"){
			HtmlParser.NextTag(true);
			if (HtmlParser.GetTag().GetName()!="/MEAN") throw VikStd::TExc("TKMeans::ParseAtt: '/mean' tag expected.");
			m_Transformations.Last().m_Mean=VikStd::ToFloat(HtmlParser.GetTextBetween().c_str(), "TKMeans::ParseAtt: invalid value for mean - not float.")
            ;
			IsOK[3]=true;
		}

		// dev tag
		else if (HtmlParser.GetTag().GetName()=="DEV"){
			HtmlParser.NextTag(true);
			if (HtmlParser.GetTag().GetName()!="/DEV") throw VikStd::TExc("TKMeans::ParseAtt: '/dev' tag expected.");
			m_Transformations.Last().m_Dev=VikStd::ToFloat(HtmlParser.GetTextBetween().c_str(),"TKMeans::ParseAtt: invalid value for dev - not float.");
			IsOK[4]=true;
		}

		// weight tag
		else if (HtmlParser.GetTag().GetName()=="WEIGHT"){
			HtmlParser.NextTag(true);
			if (HtmlParser.GetTag().GetName()!="/WEIGHT") throw VikStd::TExc("TKMeans::ParseAtt: '/weight' tag expected.");
			m_Transformations.Last().m_Weight=VikStd::ToFloat(HtmlParser.GetTextBetween().c_str(),"TKMeans::ParseAtt: invalid value for weight - not float.");			
			IsOK[5]=true;
		}

		else{ // error - unrecognised tag
			throw VikStd::TExc(VikStd::TStr("TKMeans::ParseAtt: unexpected tag: ")+HtmlParser.GetTag().GetName());
		};

		// read next tag
		HtmlParser.NextTag(true);
	};
	
	for (i=0; i<6; i++){
		if (IsOK[i]==false){
			VikStd::TStr err="Missing field in attribute description: ";
			switch (i){
				case 0: err+="type"; break;
				case 1: err+="name"; break;
				case 2: err+="dvalues"; break;
				case 3: err+="mean"; break;
				case 4: err+="dev"; break;
				case 5: err+="weight"; break;
			};
			throw VikStd::TExc(err);
		};
	};

	HtmlParser.NextTag(true);
};

/////////////////////////////////////////////////////////////////

void TKMeans::ParseCluster(VikStd::THTMLParser &HtmlParser){
	m_Clusters.Add();

	if (HtmlParser.GetTag().GetName()!="CLUSTER") 
		throw VikStd::TExc(VikStd::TStr("TKMeans::ParseCluster: 'cluster' tag expected, found '")+HtmlParser.GetTag().GetName()+"'");
	HtmlParser.NextTag(true);

	while (HtmlParser.GetTag().GetName()!="/CLUSTER"){
		
		if (HtmlParser.GetTag().GetName()!="VALUES")
			throw VikStd::TExc(VikStd::TStr("TKMeans::ParseCluster: 'values' tag expected, found '")+HtmlParser.GetTag().GetName()+"'");
		HtmlParser.NextTag(true);
		if (HtmlParser.GetTag().GetName()!="/VALUES") 
			throw VikStd::TExc(VikStd::TStr("TKMeans::ParseCluster: '/values' tag expected, found '")+HtmlParser.GetTag().GetName()+"'");		

		VikStd::TVec<VikStd::TStr> vec;
        VikStd::TStr text_between;
        text_between = HtmlParser.GetTextBetween();
		SplitIntoWords(text_between, vec);
		
		int val=0;
		for (int i=0; i<m_Transformations.Len(); i++){			
			
			m_Clusters.Last().m_Items.Add();
			TClusterItem *item=&m_Clusters.Last().m_Items.Last();
			item->m_AttrType=m_Transformations[i].m_AttrType;

			if (m_Transformations[i].m_AttrType==VikStd::atDiscr){
				for (int j=0; j<m_Transformations[i].m_ValCount; j++){
					item->m_Probs.Add(VikStd::ToFloat(vec[val],"TKMeans::ParseCluster: Invalid probability string."));
					val++;
				};
			}
			else{
				item->m_Val=ToFloat(vec[val],"TKMeans::ParseCluster: Invalid probability string.");
				val++;
			};
		};

		HtmlParser.NextTag(true);
		if (HtmlParser.GetTag().GetName()!="MEMBERS")
			throw VikStd::TExc(VikStd::TStr("TKMeans::ParseCluster: 'members' tag expected, found '")+HtmlParser.GetTag().GetName()+"'");
		HtmlParser.NextTag(true);
		if (HtmlParser.GetTag().GetName()!="/MEMBERS")
			throw VikStd::TExc(VikStd::TStr("TKMeans::ParseCluster: '/members' tag expected, found '")+HtmlParser.GetTag().GetName()+"'");
		HtmlParser.NextTag(true);		
	};
	HtmlParser.NextTag(true);
};

/////////////////////////////////////////////////////////////////

void TKMeans::AssignToClusters(VikStd::TStr &AttrName){
	VikStd::TAttribute at(VikStd::atDiscr,AttrName);

    VikStd::TRnd rnd;
    VikStd::TVec<VikStd::TPairDoubleInt> Distances;
    Distances.ResizeX(m_Clusters.Len());
	
    int i, j;
	for (i=0; i<m_Clusters.Len(); i++) {
        VikStd::TStr s(i);
        at.DeclareValue(s, false);
    };

	for (i=0; i<m_Tab->Attrs[0].Len(); i++){ // for each example

        for (j=0; j<m_Clusters.Len(); j++){ // for each cluster
            Distances[j].m_c1=GetDistance(i,j);
            Distances[j].m_c2=j;
        };
        
        Distances.Sort();
        for (j=1; ((j<Distances.Len()) && (Distances[j].m_c1==Distances[0].m_c1)); j++); // find number of same distances
        at.AddDiscrInt(Distances[rnd.GetInt(j)].m_c2);
	};
	m_Tab->Attrs.Add(at);
};

/////////////////////////////////////////////////////////////////
}
