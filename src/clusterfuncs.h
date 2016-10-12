#include "stdvik.h"

namespace VikClustering {

    void DoClustering(VikStd::TStr DataFile, VikStd::TStr OutputFile, int iter, int clusters, VikStd::TOut &out);
    void AssignToClusters(VikStd::TStr DataFile, VikStd::TStr ClustersFile, VikStd::TStr OutputFile, VikStd::TStr AttrName, VikStd::TOut &out);

};
