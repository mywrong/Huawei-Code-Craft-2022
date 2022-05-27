//
// Created by Pot on 2022/4/1.
//
#include <queue>
#include <cmath>
#include <algorithm>

#include "Judge.h"
using namespace std;

Judge::Judge(string dataPath, string solutionPath){

    IO io(dataPath, solutionPath);
    this->io = io;
    this->io.demandReader(this->demandDict);
    this->io.siteReader(this->siteDict);
    this->io.qosReader(this->qosDict);
    this->io.configReader(this->configDict );
    this->qos_constraint = this->configDict["qos_constraint"];
    this->base_cost = this->configDict["base_cost"];
    this->M = this->io.getM();
    this->T = this->io.getT();
    this->N = this->io.getN();
    this->custom_ids=this->io.getCustom_ids();
    this->site_ids=this->io.getSite_ids();
    this->times = this->io.getTimes();
    this->stream_ids=this->io.getStream_ids();

    this->N5 = int(floor(this->T * 0.05));
    this->N95 = int(ceil(this->T * 0.95)) - 1;
}
int Judge::getScore(){
    vector<vector<vector<string>>> X= this->io.SolutionReader();
    unordered_map<string,vector<int>> siteTimeInfo=this->initSiteTimeInfo();
    for(int t=0;t<this->T;t++){
        string time=this->times[t];
        for(int cidx=0;cidx< this->M;cidx++){
            string cid= this->custom_ids[cidx];
            int stream_nums=this->stream_ids[time].size();
            for(int pidx=0;pidx<stream_nums;pidx++){
                string pid=this->stream_ids[time][pidx];
                int need=this->demandDict[time][cid][pid];
                string sid=X[t][cidx][pidx];
                if(sid.compare("")==0 && need>0){
                    cout<<"There are streams but not allocated:time:"<<time<<","<<"Custom_id:"<<cid<<","<<"stream_id:"<<pid<<endl;
                    return -1;
                }
                if(need>0 && siteTimeInfo[sid][t]<need){
                    cout<<"Edge node traffic allocation exceeds the limit:time:"<<time<<","<<"Custom_id:"<<cid<<","<<"stream_id:"<<pid<<","<<"site_id:"<<sid<<endl;
                    return -1;
                }
                if(need>0)
                    siteTimeInfo[sid][t]-=need;
            }
        }
    }
    unordered_map<string,vector<int>> siteTimeUsed;
    vector<double> cost(this->N,-1);
    for(int sidx=0;sidx<this->N;sidx++){
        string sid= this->site_ids[sidx];
        siteTimeUsed.emplace(sid,vector<int>());
        for(int t=0;t<this->T;t++){
            siteTimeUsed[sid].push_back(this->siteDict[sid]-siteTimeInfo[sid][t]);
        }
        sort(siteTimeUsed[sid].begin(), siteTimeUsed[sid].end());
        if(siteTimeUsed[sid][this->T-1]==0){
            cost[sidx]=0;
        }else if(siteTimeUsed[sid][this->N95]<=this->base_cost){
            cost[sidx]=this->base_cost;
        }else{
            cost[sidx]= pow(siteTimeUsed[sid][this->N95]-this->base_cost,2)/ this->siteDict[sid]+siteTimeUsed[sid][this->N95];
        }
//        cost[sidx]=siteTimeUsed[sid][this->N95];
    }
    double score=0;
    for(double c: cost){
        score+=c;
    }
    return int(floor(score));


}

unordered_map<string,vector<int>> Judge::initSiteTimeInfo() {
    unordered_map<string,vector<int>> siteTimeInfo;
    for (int sidx = 0; sidx < this->N; sidx++) {
        string sid = this->site_ids[sidx];
        vector<int> flows;
        for (int t = 0; t < this->T; t++) {
            flows.push_back(this->siteDict[sid]);
        }
        siteTimeInfo.insert(make_pair(sid,flows));
    }
    return siteTimeInfo;
}
