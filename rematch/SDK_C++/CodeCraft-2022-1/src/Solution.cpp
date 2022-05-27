//
// Created by Pot on 2022/3/31.
//

#include <queue>
#include <cmath>
#include <algorithm>
#include <thread>
#include <mutex>
#include "Solution.h"
#include "IO.h"

using namespace std;
int THREAD_WORKERS=8;

Solution::Solution(string dataPath, string solutionPath) {
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

    this->getSiteSets(this->siteSets);
    this->getCustSets(this->custSets);
    this->N5 = int(floor(this->T * 0.05));
    this->N95 = int(ceil(this->T * 0.95)) - 1;


}

void Solution::work() {
    vector<vector<vector<int>>> X;//[t,cidx,pidx]
    for (int t = 0; t < this->T; t++) {
        string time = this->times[t];
        vector<vector<int>> custStreams;
        for (int cidx = 0; cidx < this->M; cidx++) {
            int stream_nums = this->stream_ids[time].size();
            vector<int> streams(stream_nums, -1);
            custStreams.push_back(streams);
        }
        X.push_back(custStreams);
    }

    vector<vector<vector<int>>> custTimeStreamSort;
    this->getCustTimeStreamSort(custTimeStreamSort);
    vector<vector<vector<int>>> demandInfo;
    this->getDemandInfo(demandInfo);
    vector<vector<int>> siteTimeInfo;
    this->initSiteTimeInfo(siteTimeInfo);
    vector<vector<int>> custTimeNeeds;
    this->getCustTimeNeeds(custTimeNeeds);

    vector<int> siteQ = this->getSiteQ();
//    /*先满足N5大的流量*/
//    for (int sidx: siteQ) {
//        string sid = this->site_ids[sidx];
//        vector<vector<int>> timeFlows;
//        for (int t = 0; t < this->T; t++) {
//            string time = this->times[t];
//            int sum = 0;
//            for (int cidx: this->custSets[sidx]) {
//                int stream_nums = this->stream_ids[time].size();
//                for (int pidx = 0; pidx < stream_nums; pidx++) {
//                    sum += demandInfo[t][cidx][pidx];
//                }
//            }
//            timeFlows.push_back(vector<int>{t, sum});
//        }
//        vector<vector<int>> N5Largest = this->topK(timeFlows, this->N5);
//        for (vector<int> v: N5Largest) {
//            int t = v[0];
//            vector<int> q(this->custSets[sidx]);
//            sort(q.begin(), q.end(),
//                 [custTimeNeeds, t](int x, int y) { return custTimeNeeds[x][t] > custTimeNeeds[y][t]; });
//            for (int cidx: q) {
//                for (int pidx: custTimeStreamSort[cidx][t]) {
//                    int need = demandInfo[t][cidx][pidx];
//                    if (need > 0 && siteTimeInfo[sidx][t] >= need) {
//                        siteTimeInfo[sidx][t] -= need;
//                        X[t][cidx][pidx] = sidx;
//                        custTimeNeeds[cidx][t] -= need;
//                        demandInfo[t][cidx][pidx] = 0;
//                    }
//                }
//            }
//        }
//        int N95Largest = 0;
//        for (int t = 0; t < this->T; t++) {
//            int s = 0;
//            for (int cidx: this->custSets[sidx]) {
//                if (siteTimeInfo[sidx][t] > 0) {
//                    int need = custTimeNeeds[cidx][t];
//                    int n = this->siteSets[cidx].size();
//                    int f = int(need / n);
//                    f = min(f, siteTimeInfo[sidx][t]);
//                    s += f;
//                }
//            }
//            N95Largest = max(N95Largest, s);
//        }
//        for (int t = 0; t < this->T; t++) {
//            int canFlow = N95Largest;
//            for (int cidx: this->custSets[sidx]) {
//                for (int pidx: custTimeStreamSort[cidx][t]) {
//                    int need = demandInfo[t][cidx][pidx];
//                    if (need>0 && siteTimeInfo[sidx][t] >= need){
//                        siteTimeInfo[sidx][t] -= need;
//                        X[t][cidx][pidx] = sidx;
//                        custTimeNeeds[cidx][t] -= need;
//                        demandInfo[t][cidx][pidx] = 0;
//                        canFlow -= need;
//                        if (canFlow <= 0)
//                            break;
//                    }
//                }
//            }
//        }
//    }

    ////鲁棒分配
    for (int t = 0; t < this->T; t++) {
        string time = this->times[t];
        int stream_nums = this->stream_ids[time].size();
        for (int cidx = 0; cidx < this->M; cidx++) {
            for (int pidx = 0; pidx < stream_nums; pidx++) {
                int need = demandInfo[t][cidx][pidx];
                if (need == 0)
                    continue;
                for (int sidx: this->siteSets[cidx]) {
                    if (siteTimeInfo[sidx][t] >= need) {
                        siteTimeInfo[sidx][t] -= need;
                        X[t][cidx][pidx] = sidx;
                        custTimeNeeds[cidx][t] -= need;
                        demandInfo[t][cidx][pidx] = 0;
                        break;
                    }
                }
            }
        }
    }
    vector<vector<int>> siteTimeUsed(this->N,vector<int>(this->T,0));
    this->getSiteTimeUsed(siteTimeUsed,X);
    unordered_map<int,int> N95Flows;
    this->get_N95Flows(N95Flows,siteTimeUsed);
//    int N=min(100,this->N);
    for(int i=0;i<this->N;i++){
        int sidx=siteQ[i];
        int T=this->T;
        int n=THREAD_WORKERS;
        int startT=0;
        vector<thread> Thread;
        for(int j=0;j<THREAD_WORKERS;j++){
            Thread.push_back(thread([&,startT,T,n](){
                int sT=startT,eT=startT+int(T/n);
                for(int t=sT;t<eT;t++){
                    string time=this->times[t];
                    int stream_nums=this->stream_ids[time].size();
                    if(siteTimeUsed[sidx][t]>N95Flows[sidx]){
                        continue;
                    }
                    for(int cidx=0;cidx<this->M;cidx++){
                        string cid=this->custom_ids[cidx];
                        vector<int> brotherSidxs=this->siteSets[cidx];
                        vector<pair<int,int>> brotherQ;
                        for(int brotherSidx:brotherSidxs){
                            int canFlow=0;
                            string bsid=this->site_ids[brotherSidx];
                            if(brotherSidx!=sidx){
                                if(siteTimeUsed[brotherSidx][t]<=N95Flows[brotherSidx]){
                                    canFlow=N95Flows[brotherSidx]-siteTimeUsed[brotherSidx][t];
                                }else{
                                    canFlow=this->siteDict[bsid]-siteTimeUsed[brotherSidx][t];
                                }
                                brotherQ.push_back(make_pair(brotherSidx,canFlow));
                            }
                        }
//                sort(brotherQ.begin(),brotherQ.end(),[](pair<int,int> x,pair<int,int> y){return x.second>y.second;});
//                int bri=0,n=brotherQ.size();
                        priority_queue<pair<int, int>, vector<pair<int, int>>, decltype(&this->cmp)> q(cmp,brotherQ);

                        for(int pidx=0;pidx<stream_nums;pidx++){
                            string pid=this->stream_ids[time][pidx];
                            if(X[t][cidx][pidx]!=sidx)
                                continue;
                            int need=this->demandDict[time][cid][pid];
                            int brotherSidx=q.top().first;
                            int canFlow=q.top().second;
                            q.pop();
                            if(canFlow>=need){
                                siteTimeInfo[brotherSidx][t] -= need;
                                siteTimeUsed[brotherSidx][t]+=need;
//                            cout<<"bri:"<<bri<<endl;
//                            cout<<"修改前："<<brotherQ[bri].second<<endl;
//                            cout<<"need:"<<need<<endl;
                                canFlow-=need;
//                            cout<<"修改后："<<brotherQ[bri].second<<endl;
                                siteTimeInfo[sidx][t]+=need;
                                siteTimeUsed[sidx][t]-=need;
                                X[t][cidx][pidx] = brotherSidx;
                                break;
                            }
                            q.emplace(brotherSidx,canFlow);
                        }
                    }
                }
            }));
            startT+=int(T/n);
            T-=int(T/n);
            n-=1;
        }
        for(int j=0;j<THREAD_WORKERS;j++){
            Thread[j].join();
        }
        vector<int> tmp(siteTimeUsed[sidx]);
        sort(tmp.begin(),tmp.end());
        N95Flows[sidx]=tmp[this->N95];
    }


    this->io.output(X);
}

void Solution::reDistribute(int sidx,vector<vector<int>> &siteTimeUsed,vector<vector<int>> &siteTimeInfo,unordered_map<int,int> &N95Flows,vector<vector<vector<int>>> &X){
    for(int t=0;t< this->T;t++){
        string time=this->times[t];
        int stream_nums=this->stream_ids[time].size();
        if(siteTimeUsed[sidx][t]>N95Flows[sidx]){
            continue;
        }
        for(int cidx=0;cidx<this->M;cidx++){
            string cid=this->custom_ids[cidx];
            vector<int> brotherSidxs=this->siteSets[cidx];
            vector<pair<int,int>> brotherQ;
            for(int brotherSidx:brotherSidxs){
                int canFlow=0;
                string bsid=this->site_ids[brotherSidx];
                if(brotherSidx!=sidx){
                    if(siteTimeUsed[brotherSidx][t]<=N95Flows[brotherSidx]){
                        canFlow=N95Flows[brotherSidx]-siteTimeUsed[brotherSidx][t];
                    }else{
                        canFlow=this->siteDict[bsid]-siteTimeUsed[brotherSidx][t];
                    }
                    brotherQ.push_back(make_pair(brotherSidx,canFlow));
                }
            }
//                sort(brotherQ.begin(),brotherQ.end(),[](pair<int,int> x,pair<int,int> y){return x.second>y.second;});
//                int bri=0,n=brotherQ.size();
            priority_queue<pair<int, int>, vector<pair<int, int>>, decltype(&this->cmp)> q(cmp,brotherQ);

            for(int pidx=0;pidx<stream_nums;pidx++){
                string pid=this->stream_ids[time][pidx];
                if(X[t][cidx][pidx]!=sidx)
                    continue;
                int need=this->demandDict[time][cid][pid];
                int brotherSidx=q.top().first;
                int canFlow=q.top().second;
                q.pop();
                if(canFlow>=need){
                    siteTimeInfo[brotherSidx][t] -= need;
                    siteTimeUsed[brotherSidx][t]+=need;
//                            cout<<"bri:"<<bri<<endl;
//                            cout<<"修改前："<<brotherQ[bri].second<<endl;
//                            cout<<"need:"<<need<<endl;
                    canFlow-=need;
//                            cout<<"修改后："<<brotherQ[bri].second<<endl;
                    siteTimeInfo[sidx][t]+=need;
                    siteTimeUsed[sidx][t]-=need;
                    X[t][cidx][pidx] = brotherSidx;
                    break;
                }
                q.emplace(brotherSidx,canFlow);
            }
        }
    }
}


void Solution::get_N95Flows(unordered_map<int,int> &N95Flows,vector<vector<int>> &siteTimeUsed){

    vector<vector<int>> siteFlow(siteTimeUsed);
    for(int sidx=0;sidx<this->N;sidx++){
        sort(siteFlow[sidx].begin(),siteFlow[sidx].end());
        N95Flows.insert(make_pair(sidx,siteFlow[sidx][this->N95]));
    }

}

void Solution::getDemandInfo(vector<vector<vector<int>>> &demandInfo) {
    //demandInfo:[t,cidx,pidx]
    for (int t = 0; t < this->T; t++) {
        string time = this->times[t];
        vector<vector<int>> custNeed;
        for (int cidx = 0; cidx < this->M; cidx++) {
            string cid = this->custom_ids[cidx];
            vector<int> needFlow;
            int stream_nums = this->stream_ids[time].size();
            for (int pidx = 0; pidx < stream_nums; pidx++) {
                string pid = this->stream_ids[time][pidx];
                needFlow.push_back(this->demandDict[time][cid][pid]);
            }
            custNeed.push_back(needFlow);
        }
        demandInfo.push_back(custNeed);
    }

}



void Solution::initSiteTimeInfo(vector<vector<int>> &siteTimeInfo) {
    for (int sidx = 0; sidx < this->N; sidx++) {
        string sid = this->site_ids[sidx];
        vector<int> flows;
        for (int t = 0; t < this->T; t++) {
            flows.push_back(this->siteDict[sid]);
        }
        siteTimeInfo.push_back(flows);
    }
}

void Solution::getCustSets(unordered_map<int, vector<int>> &custSets) {
    //{sidx:cidx}
    for (int sidx = 0; sidx < this->N; sidx++) {
        string sid = this->site_ids[sidx];
        vector<int> custSet;
        for (int cidx = 0; cidx < this->M; cidx++) {
            string cid = this->custom_ids[cidx];
            if (this->qosDict[sid][cid] < this->qos_constraint) {
                custSet.push_back(cidx);
            }
        }
        custSets.insert(make_pair(sidx, custSet));
    }

}

void Solution::getSiteSets(unordered_map<int, vector<int>> &siteSets) {

    for (int cidx = 0; cidx < this->M; cidx++) {
        string cid = this->custom_ids[cidx];
        vector<int> siteSet;
        for (int sidx = 0; sidx < this->N; sidx++) {
            string sid = this->site_ids[sidx];
            if (this->qosDict[sid][cid] < this->qos_constraint) {
                siteSet.push_back(sidx);
            }
        }
        siteSets.insert(make_pair(cidx, siteSet));
    }
}

vector<vector<int>> Solution::topK(vector<vector<int>> &v, int k) {
    auto cmp = [](const vector<int> &a, const vector<int> &b) {
        return a[1] > b[1];
    };
    priority_queue<vector<int>, vector<vector<int>>, decltype(cmp)> pq(cmp);
    for (auto &it: v) {
        pq.emplace(it);
        if (int(pq.size()) > k) {
            pq.pop();
        }
    }
    vector<vector<int>> ret(k);
    for (int i = k - 1; i > -1; i--) {
        ret[i] = pq.top();
        pq.pop();
    }
    return ret;

}

vector<int> Solution::getSiteQ() {
    vector<int> cnts;
    for (int sidx = 0; sidx < this->N; sidx++) {
        string sid = this->site_ids[sidx];
        int cnt = 0;
        for (int cidx = 0; cidx < this->M; cidx++) {
            string cid = this->custom_ids[cidx];
            if (this->qosDict[sid][cid] < this->qos_constraint) {
                cnt++;
            }
        }
        cnts.push_back(cnt);
    }
    vector<int> siteQ(this->N);
    for (int sidx = 0; sidx < this->N; sidx++) {
        siteQ.push_back(sidx);
    }
    sort(siteQ.begin(), siteQ.end(), [cnts](int x, int y) { return cnts[x] > cnts[y]; });
    return siteQ;

}

void Solution::getCustTimeNeeds(vector<vector<int>> &custTimeNeeds) {
    //return [cidx,time]
    for (int cidx = 0; cidx < this->M; cidx++) {
        string cid = this->custom_ids[cidx];
        vector<int> custNeeds(this->M);

        for (int t = 0; t < this->T; t++) {
            string time = this->times[t];
            int need = 0;
            int stream_nums = this->stream_ids[time].size();
            for (int pidx = 0; pidx < stream_nums; pidx++) {
                string pid = this->stream_ids[time][pidx];
                need += this->demandDict[time][cid][pid];
            }
            custNeeds.push_back(need);
        }
        custTimeNeeds.push_back(custNeeds);
    }

}

void Solution::getCustTimeStreamSort(vector<vector<vector<int>>> &custTimeStreamSort) {
    //return [cidx,t,pidx]
    for (int cidx = 0; cidx < this->M; cidx++) {
        string cid = this->custom_ids[cidx];
        vector<vector<int>> timeStreamSort;
        for (int t = 0; t < this->T; t++) {
            string time = this->times[t];
            int stream_nums = this->stream_ids[time].size();
            vector<int> streamNeeds(stream_nums);
            vector<int> streams(stream_nums);
            for (int pidx = 0; pidx < stream_nums; pidx++) {
                string pid = this->stream_ids[time][pidx];
                streamNeeds.push_back(this->demandDict[time][cid][pid]);
                streams.push_back(pidx);
            }
            sort(streams.begin(), streams.end(),
                 [streamNeeds](int x, int y) { return streamNeeds[x] > streamNeeds[y]; });
            timeStreamSort.push_back(streams);
        }
        custTimeStreamSort.push_back(timeStreamSort);
    }
}

void Solution::getSiteTimeUsed(vector<vector<int>> &siteTimeUsed,vector<vector<vector<int>>> &X){

    for(int t=0;t<this->T;t++){
        string time=this->times[t];
        int stream_nums=this->stream_ids[time].size();
        for(int cidx=0;cidx<this->M;cidx++){
            string cid=this->custom_ids[cidx];
            for(int pidx=0;pidx<stream_nums;pidx++){
                string pid=this->stream_ids[time][pidx];
                int sidx= X[t][cidx][pidx];
                if(sidx>-1)
                    siteTimeUsed[sidx][t]+=this->demandDict[time][cid][pid];
            }
        }
    }

}


