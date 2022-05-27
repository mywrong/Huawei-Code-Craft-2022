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
#include <set>

using namespace std;
int THREAD_WORKERS = 8;
int MAX_INT = 100000000;

Solution::Solution(string dataPath, string solutionPath) {
    this->dataPath=dataPath;
    this->solutionPath=solutionPath;
    IO io(dataPath, solutionPath);
    this->io = io;
    this->io.demandReader(this->demandDict);
    this->io.siteReader(this->siteDict);
    this->io.qosReader(this->qosDict);
    this->io.configReader(this->configDict);
    this->qos_constraint = this->configDict["qos_constraint"];
    this->base_cost = this->configDict["base_cost"];
    this->M = this->io.getM();
    this->T = this->io.getT();
    this->N = this->io.getN();
    this->custom_ids = this->io.getCustom_ids();
    this->site_ids = this->io.getSite_ids();
    this->times = this->io.getTimes();
    this->stream_ids = this->io.getStream_ids();

    this->getSiteSets(this->siteSets);
    this->getCustSets(this->custSets);
    this->N5 = int(floor(this->T * 0.05));
    this->N95 = int(ceil(this->T * 0.95)) - 1;


}

void Solution::work() {
    vector<vector<vector<int>>> X;//[t,cidx,pidx]
    X.reserve(this->T);
    for (int t = 0; t < this->T; t++) {
        string time = this->times[t];
        vector<vector<int>> custStreams;
        custStreams.reserve(this->M);
        for (int cidx = 0; cidx < this->M; cidx++) {
            int stream_nums = this->stream_ids[time].size();
            vector<int> streams(stream_nums, -1);
            custStreams.push_back(streams);
        }
        X.push_back(custStreams);
    }

    vector<vector<vector<int>>> demandInfo;
    this->getDemandInfo(demandInfo);
    vector<vector<int>> siteTimeInfo;
    this->initSiteTimeInfo(siteTimeInfo);
    vector<vector<int>> custTimeNeeds;
    this->getCustTimeNeeds(custTimeNeeds);

    vector<int> siteQ = this->getSiteQ();

    vector<int> siteMaxFlow(this->N,0);

//    int superSite=this->getFullSite()[0];
//    for(int t=0;t<this->T;t++){
//        string time=this->times[t];
//        int stream_nums=this->stream_ids[time].size();
//        vector<vector<int>> needs;
//        for(int cidx:this->custSets[superSite]){
//            for(int pidx=0;pidx< stream_nums;pidx++){
//                needs.push_back(vector<int>{cidx,pidx,demandInfo[t][cidx][pidx]});
//            }
//        }
//        sort(needs.begin(), needs.end(),[](vector<int> a,vector<int> b){return a[2]>b[2];});
//        for(vector<int> needpa:needs){
//            int cidx=needpa[0];
//            int pidx=needpa[1];
//            int need=needpa[2];
//            if (need > 0 && siteTimeInfo[superSite][t] >= need) {
//                siteTimeInfo[superSite][t] -= need;
//                siteMaxFlow[superSite] = max(siteMaxFlow[superSite], this->siteDict[this->site_ids[superSite]] - siteTimeInfo[superSite][t]);
//                X[t][cidx][pidx] = superSite;
//                custTimeNeeds[cidx][t] -= need;
//                demandInfo[t][cidx][pidx] = 0;
//            }
//        }
//    }


    /*先满足N5大的流量*/
    for (int sidx: siteQ) {
        string sid = this->site_ids[sidx];
        vector<vector<int>> timeFlows;
        timeFlows.reserve(this->T);
        for (int t = 0; t < this->T; t++) {
            string time = this->times[t];
            int sum = 0;
            for (int cidx: this->custSets[sidx]) {
                sum += custTimeNeeds[cidx][t];
            }
            timeFlows.push_back(vector<int>{t, sum});
        }
        vector<vector<int>> N5Largest = this->topK(timeFlows, this->N5);
        for (vector<int> v: N5Largest) {
            int t = v[0];
            string time = this->times[t];
            int stream_nums = this->stream_ids[time].size();
            for (int cidx: this->custSets[sidx]) {
                for (int pidx = 0; pidx < stream_nums; pidx++) {
                    int need = demandInfo[t][cidx][pidx];
                    if (need > 0 && siteTimeInfo[sidx][t] >= need) {
                        siteTimeInfo[sidx][t] -= need;
                        siteMaxFlow[sidx] = max(siteMaxFlow[sidx], this->siteDict[sid] - siteTimeInfo[sidx][t]);
                        X[t][cidx][pidx] = sidx;
                        custTimeNeeds[cidx][t] -= need;
                        demandInfo[t][cidx][pidx] = 0;
                    }
                }
            }
        }
    }

    for (int sidx: siteQ) {
        string sid = this->site_ids[sidx];
        int N95Largest = this->base_cost;
        for (int t = 0; t < this->T; t++) {
            int canFlow = N95Largest;
            string time = this->times[t];
            int stream_nums = this->stream_ids[time].size();
            for (int cidx: this->custSets[sidx]) {
                for (int pidx = 0; pidx < stream_nums; pidx++) {
                    int need = demandInfo[t][cidx][pidx];
//                    if (need > 0 && siteTimeInfo[sidx][t] >= need && canFlow >= need) {
                    if (need > 0 && siteTimeInfo[sidx][t] >= need) {
                        siteTimeInfo[sidx][t] -= need;
                        siteMaxFlow[sidx] = max(siteMaxFlow[sidx], this->siteDict[sid] - siteTimeInfo[sidx][t]);
                        X[t][cidx][pidx] = sidx;
                        custTimeNeeds[cidx][t] -= need;
                        demandInfo[t][cidx][pidx] = 0;
                        canFlow -= need;
                        if (canFlow <= 0)
                            break;
                    }
//                    if (canFlow <= 0)
//                        break;
                }
//                if (canFlow <= 0)
//                    break;
            }
        }
    }
    this->io.output(X, this->dataPath+"/first.txt");
//////新鲁棒分配
    vector<vector<int>> siteTimeUsed1(this->N, vector<int>(this->T, 0));
    this->getSiteTimeUsed(siteTimeUsed1, X);
    vector<vector<pair<int, int>>> used(this->N, vector<pair<int, int>>(this->T));
    for (int sidx = 0; sidx < this->N; sidx++) {
        for (int t = 0; t < this->T; t++) {
            used[sidx][t] = make_pair(t, siteTimeUsed1[sidx][t]);
        }
        sort(used[sidx].begin(), used[sidx].end(),
             [](pair<int, int> a, pair<int, int> b) { return a.second < b.second; });
    }
    auto cmp = [](const pair<int, int> &a, const pair<int, int> &b) {
        return a.second > b.second;
    };
    vector<priority_queue<pair<int, int>, vector<pair<int, int>>, decltype(cmp)>> N5Flows(this->N,priority_queue<pair<int, int>, vector<pair<int, int>>, decltype(cmp)>(cmp));
    vector<set<int>> N5Ts(this->N);
    for (int sidx = 0; sidx < this->N; sidx++) {
        for (int t = this->N95; t < this->T; t++) {
            N5Flows[sidx].emplace(used[sidx][t]);
            N5Ts[sidx].insert(used[sidx][t].first);
        }
    }

    for (int t = 0; t < this->T; t++) {
        string time = this->times[t];
        int stream_nums = this->stream_ids[time].size();
        for (int cidx = 0; cidx < this->M; cidx++) {
            for (int pidx = 0; pidx < stream_nums; pidx++) {
                int need = demandInfo[t][cidx][pidx];
                if (need == 0)
                    continue;
                int minAddCostSidx;
                double minAddCost = MAX_INT;
                for (int sidx: this->siteSets[cidx]) {
                    if (siteTimeInfo[sidx][t] >= need) {
                        int oldN95 = N5Flows[sidx].top().second;
                        int newN95 = oldN95;
                        int tmp = siteTimeUsed1[sidx][t] + need;
                        if (tmp > N5Flows[sidx].top().second ) {
                            if(N5Ts[sidx].count(t) == 0 || (N5Ts[sidx].count(t) != 0 && siteTimeUsed1[sidx][t]==oldN95)){
                                auto a = N5Flows[sidx].top();
                                N5Flows[sidx].pop();
                                auto b = N5Flows[sidx].top();
                                newN95=min(tmp,b.second);
                                N5Flows[sidx].emplace(a);
                            }
                        }
                        int oldcost=this->cost_cal(sidx,oldN95,siteMaxFlow);
                        int newcost=this->cost_cal(sidx,newN95,siteMaxFlow);
                        if(newcost-oldcost<minAddCost){
                            minAddCost=newcost-oldcost;
                            minAddCostSidx=sidx;
                        }
                    }
                }
                siteTimeInfo[minAddCostSidx][t]-=need;
                X[t][cidx][pidx]=minAddCostSidx;
                custTimeNeeds[cidx][t]-=need;
                demandInfo[t][cidx][pidx]=0;
                siteTimeUsed1[minAddCostSidx][t]+=need;
                siteMaxFlow[minAddCostSidx]=max(siteMaxFlow[minAddCostSidx],siteTimeUsed1[minAddCostSidx][t]);
                if(N5Ts[minAddCostSidx].count(t)==0){
                    N5Flows[minAddCostSidx].emplace(make_pair(t,siteTimeUsed1[minAddCostSidx][t]));
                    N5Ts[minAddCostSidx].insert(t);
                    auto a=N5Flows[minAddCostSidx].top();
                    N5Ts[minAddCostSidx].erase(a.first);
                    N5Flows[minAddCostSidx].pop();

                }else{
                    vector<pair<int,int>> pqtmp;
                    while(!N5Flows[minAddCostSidx].empty() && N5Flows[minAddCostSidx].top().first!=t){
                        pqtmp.push_back(N5Flows[minAddCostSidx].top());
                        N5Flows[minAddCostSidx].pop();
                    }
                    N5Flows[minAddCostSidx].pop();
                    N5Flows[minAddCostSidx].emplace(t,siteTimeUsed1[minAddCostSidx][t]);
                    for(auto pa:pqtmp){
                        N5Flows[minAddCostSidx].emplace(pa);
                    }
                }
            }
        }
    }
    this->io.output(X, this->dataPath+"/second.txt");
    ////迁移

    vector<vector<int>> siteTimeUsed(this->N, vector<int>(this->T, 0));
    this->getSiteTimeUsed(siteTimeUsed, X);
    unordered_map<int, int> N95Flows;
    this->get_N95Flows(N95Flows, siteTimeUsed);
    std::sort(siteQ.begin(), siteQ.end(),[&](int x,int y){return N95Flows[x]>N95Flows[y];});
    for (int i = 0; i < this->N; i++) {
        int sidx = siteQ[i];
        string sid=this->site_ids[sidx];
        cout<<"迁移"<<sid<<","<<"N95:"<<N95Flows[sidx]<<endl;
        int T = this->T;
        int n = THREAD_WORKERS;
        int startT = 0;
        vector<thread> Thread;
        for (int j = 0; j < THREAD_WORKERS; j++) {
            Thread.push_back(thread([&, startT, T, n]() {
                int sT = startT, eT = startT + int(T / n);
                for (int t = sT; t < eT; t++) {
                    string time = this->times[t];
                    int stream_nums = this->stream_ids[time].size();
                    if (siteTimeUsed[sidx][t] > N95Flows[sidx]) {
                        continue;
                    }
                    for (int cidx = 0; cidx < this->M; cidx++) {
                        string cid = this->custom_ids[cidx];
                        vector<int> brotherSidxs = this->siteSets[cidx];
                        vector<pair<int, int>> brotherQ;
                        for (int brotherSidx: brotherSidxs) {
                            int canFlow = 0;
                            string bsid = this->site_ids[brotherSidx];
                            if (brotherSidx != sidx) {
                                int tmp = N95Flows[brotherSidx];
//                                tmp = min(this->siteDict[bsid], max(this->base_cost, tmp));
                                if (siteTimeUsed[brotherSidx][t] <= tmp) {
                                    canFlow = tmp - siteTimeUsed[brotherSidx][t];
                                } else {
                                    canFlow = this->siteDict[bsid] - siteTimeUsed[brotherSidx][t];
                                }
                                brotherQ.push_back(make_pair(brotherSidx, canFlow));
                            }
                        }
                        sort(brotherQ.begin(), brotherQ.end(),
                             [](pair<int, int> x, pair<int, int> y) { return x.second > y.second; });
                        int n = brotherQ.size();
                        for (int pidx = 0; pidx < stream_nums; pidx++) {
                            string pid = this->stream_ids[time][pidx];
                            if (X[t][cidx][pidx] != sidx)
                                continue;
                            int need = this->demandDict[time][cid][pid];
                            for (int bri = 0; bri < n; bri++) {
                                int brotherSidx = brotherQ[bri].first;
                                int canFlow = brotherQ[bri].second;
                                if (canFlow >= need) {
                                    siteTimeInfo[brotherSidx][t] -= need;
                                    siteTimeUsed[brotherSidx][t] += need;
                                    brotherQ[bri].second -= need;
                                    siteTimeInfo[sidx][t] += need;
                                    siteTimeUsed[sidx][t] -= need;
                                    X[t][cidx][pidx] = brotherSidx;
                                    break;
                                }
                            }
                        }
                    }
                }
            }));
            startT += int(T / n);
            T -= int(T / n);
            n -= 1;
        }
        for (int j = 0; j < THREAD_WORKERS; j++) {
            Thread[j].join();
        }
        vector<int> tmp(siteTimeUsed[sidx]);
        sort(tmp.begin(), tmp.end());
        N95Flows[sidx] = tmp[this->N95];
    }
//    vector<int> sidxs;
//    sidxs.reserve(this->N);
//    for(int sidx=0;sidx<this->N;sidx++){
//        sidxs.push_back(sidx);
//    }
//    sort(sidxs.begin(), sidxs.end(),[&](int a,int b){return N95Flows[a]>N95Flows[b];});
//    vector<int> N90Sidxs;
//    for(int i=0;i<10;i++){
//        N90Sidxs.push_back(sidxs[i]);
//    }
//    this->io.output(X,N90Sidxs, this->solutionPath);

    this->io.output(X, this->solutionPath);
}

double Solution::cost_cal(int sidx, int N95f, vector<int> &siteMaxFlow) {
    double cost;
    if (siteMaxFlow[sidx]==0 && N95f == 0) {
        cost = 0;
    } else if (N95f <= this->base_cost) {
        cost = this->base_cost;
    } else {
        cost = pow(N95f - this->base_cost, 2) / this->siteDict[this->site_ids[sidx]] + N95f;
    }
    return cost;
}

void Solution::get_N95Flows(unordered_map<int, int> &N95Flows, vector<vector<int>> &siteTimeUsed) {

    vector<vector<int>> siteFlow(siteTimeUsed);
    for (int sidx = 0; sidx < this->N; sidx++) {
        sort(siteFlow[sidx].begin(), siteFlow[sidx].end());
        int flow = siteFlow[sidx][this->N95];
        N95Flows.insert(make_pair(sidx, flow));
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
        sort(siteSet.begin(), siteSet.end(),
             [&](int x, int y) { return this->siteDict[this->site_ids[x]] > this->siteDict[this->site_ids[y]]; });
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

//vector<int> Solution::getSiteQ() {
//    vector<int> cnts;
//    for (int sidx = 0; sidx < this->N; sidx++) {
//        string sid = this->site_ids[sidx];
//        int cnt = 0;
//        for (int cidx = 0; cidx < this->M; cidx++) {
//            string cid = this->custom_ids[cidx];
//            if (this->qosDict[sid][cid] < this->qos_constraint) {
//                cnt++;
//            }
//        }
//        cnts.push_back(cnt);
//    }
//    vector<int> siteQ(this->N);
//    for (int sidx = 0; sidx < this->N; sidx++) {
//        siteQ.push_back(sidx);
//    }
//    sort(siteQ.begin(), siteQ.end(), [cnts](int x, int y) { return cnts[x] > cnts[y]; });
//    return siteQ;
//
//}

vector<int> Solution::getSiteQ() {
    vector<int> siteQ;
    siteQ.reserve(this->N);
    for (int sidx = 0; sidx < this->N; sidx++) {
        siteQ.push_back(sidx);
    }
    sort(siteQ.begin(), siteQ.end(), [&](int x, int y) {
        if (this->custSets[x].size() == this->custSets[y].size()) {
            return this->siteDict[this->site_ids[x]] < this->siteDict[this->site_ids[y]];
        }
        return this->custSets[x].size() < this->custSets[y].size();
    });
    return siteQ;

}
vector<int> Solution::getFullSite(){
    vector<int> siteQ;
    siteQ.reserve(this->N);
    for (int sidx = 0; sidx < this->N; sidx++) {
        siteQ.push_back(sidx);
    }
    sort(siteQ.begin(), siteQ.end(), [&](int x, int y) {
        if (this->custSets[x].size() == this->custSets[y].size()) {
            return this->siteDict[this->site_ids[x]] > this->siteDict[this->site_ids[y]];
        }
        return this->custSets[x].size() > this->custSets[y].size();
    });
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

void Solution::getSiteTimeUsed(vector<vector<int>> &siteTimeUsed, vector<vector<vector<int>>> &X) {

    for (int t = 0; t < this->T; t++) {
        string time = this->times[t];
        int stream_nums = this->stream_ids[time].size();
        for (int cidx = 0; cidx < this->M; cidx++) {
            string cid = this->custom_ids[cidx];
            for (int pidx = 0; pidx < stream_nums; pidx++) {
                string pid = this->stream_ids[time][pidx];
                int sidx = X[t][cidx][pidx];
                if (sidx > -1)
                    siteTimeUsed[sidx][t] += this->demandDict[time][cid][pid];
            }
        }
    }

}


