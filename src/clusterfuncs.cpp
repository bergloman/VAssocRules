#include "ClusterFuncs.h"
#include "Clustering.h"

namespace VikClustering{
/////////////////////////////////////////////////////

void DoClustering(VikStd::TStr DataFile, VikStd::TStr OutputFile, int iter, int clusters, VikStd::TOut &out){

	out.Put("Reading data...\n");
	VikStd::TDataTable tab;
	tab.LoadFromFileX(DataFile);
	out.Put("Performing K-Means clustering...\n");
	TKMeans kmeans(&tab);

	kmeans.m_MaxIterations=iter;
	kmeans.m_NumOfClusters=clusters;
	kmeans.DoClustering();

	out.Put("writing results...\n");
	FILE *fp=fopen(OutputFile.c_str(),"w");
	if (fp==NULL)
		throw VikStd::TExc(VikStd::TStr("Cannot open output file ")+OutputFile+"for writing");
	fprintf(fp,"%s",kmeans.GetDescAsXML().c_str());
	fclose(fp);
	out.Put("done");
};

/////////////////////////////////////////////////////

void AssignToClusters(VikStd::TStr DataFile, VikStd::TStr ClustersFile, VikStd::TStr OutputFile, VikStd::TStr AttrName, VikStd::TOut &out){

	out.Put("Reading data...\n");	
	VikStd::TDataTable tab;
	tab.LoadFromFileX(DataFile);
	out.Put("Reading clustering file...\n");
	TKMeans kmeans(&tab);
			
	kmeans.ReadFromXMLFile(ClustersFile);
	out.Put("Assigning to clusters...\n");
	kmeans.AssignToClusters(AttrName);
	out.Put("Writing results...\n");
	kmeans.SaveTabToFile(OutputFile);	
};

/////////////////////////////////////////////////////
};
