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
#include <iostream>


using namespace std;
int THREAD_WORKERS = 8;
int MAX_INT = 100000000;

struct CMP{
    bool operator()(pair<string,int> &a,pair<string,int> &b){
        return a.second>b.second;
    }
};

struct Stream{
    string time;
    string cid;
    string pid;
    int need;
    Stream(){}
    Stream(string t,string c,string p,int n){
        this->time=t;
        this->cid=c;
        this->pid=p;
        this->need=n;
    }

};
struct Site{
    string sid;
    int limitUsed;
    int maxUsed;
    int N95Used;
    int N5;
    int T;

    unordered_map<string,int> timeUsed;
    unordered_map<string,int> timeInfo;
    priority_queue<pair<string,int>,vector<pair<string,int>>,CMP> N5Flows;
    set<string> N5Times;
    Site(){}
    Site(string id,int lU,vector<string> &times){
        this->sid=id;
        this->limitUsed=lU;
        this->maxUsed=0;
        this->N95Used=0;
        this->T=times.size();
        this->N5=int(floor(this->T * 0.05));

        for(string time:times){
            this->timeInfo.emplace(time,limitUsed);
        }
    }

    bool canConsume(Stream a){
        return this->timeInfo[a.time]>=a.need;
    }

    void consume(Stream a){


        this->timeInfo[a.time]-=a.need;
        this->timeUsed[a.time]+=a.need;
        this->maxUsed=max(maxUsed,timeUsed[a.time]);
        this->N95Used=this->flush_N5Flows(a.time);
    }
    void consume1(Stream a){

        this->timeInfo[a.time]-=a.need;
        this->timeUsed[a.time]+=a.need;
//        this->maxUsed=max(maxUsed,timeUsed[a.time]);
//        this->N95Used=this->flush_N5Flows(a.time);
    }
    int tryConsume(Stream a){
        ////返回新N95值
        int newN95=this->N95Used;
        int tmp= this->timeUsed[a.time]+a.need;
        string time=a.time;
        if(tmp>this->N95Used){
            if(this->N5Times.count(time)==0 || this->timeUsed[time]== this->N95Used){

              if(this->N5Flows.size()< this->N5){
                  newN95=0;
              }else {
                  auto a=this->N5Flows.top();
                  this->N5Flows.pop();
                  auto b=this->N5Flows.top();
                  newN95=min(tmp,b.second);
                  this->N5Flows.emplace(a);
              }
            }
        }
        return newN95;
    }
    int flush_N5Flows(string time){
        int timeUsed=this->timeUsed[time];
        if(this->N5Flows.size()<this->N5){
            this->N5Flows.emplace(make_pair(time,timeUsed));
            this->N5Times.insert(time);
        }else{
            if(timeUsed>this->N5Flows.top().second){
                if(this->N5Times.count(time)==0){
                    this->N5Flows.emplace(make_pair(time,timeUsed));
                    this->N5Times.insert(time);
                    string oldTime=this->N5Flows.top().first;
                    this->N5Flows.pop();
                    this->N5Times.erase(oldTime);
                }else{
                    vector<pair<string,int>> tmp;
                    tmp.push_back(make_pair(time,timeUsed));
                    while(this->N5Flows.top().first.compare(time)!=0){
                        auto p=this->N5Flows.top();
                        tmp.push_back(p);
                        this->N5Flows.pop();
                    }
                    this->N5Flows.pop();
                    for(auto p:tmp){
                        this->N5Flows.emplace(p);
                    }
                }
            }
        }
        return this->N5Flows.top().second;
    }
//    Site & operator=(const Site &site){
//        if(this !=&site){
//            this->sid=site.sid;
//            this->limitUsed=site.limitUsed;
//            this->maxUsed=site.maxUsed;
//            this->N95Used=site.N95Used;
//            this->N5=site.N5;
//            free(this->timeUsed);
//            unordered_map<string,int> timeUsed;
//            unordered_map<string,int> timeInfo;
//            priority_queue<pair<string,int>,vector<pair<string,int>>,CMP> N5Flows;
//            set<string> N5Times;
//        }
//    };
};

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

    this->getSiteSets(this->siteSetsIdx);
    this->getSiteSets(this->siteSetsId);
    this->getCustSets(this->custSetsIdx);
    this->getCustSets(this->custSetsId);

    this->N5 = int(floor(this->T * 0.05));
    this->N95 = int(ceil(this->T * 0.95)) - 1;


}

void Solution::work() {
    unordered_map<string,unordered_map<string,unordered_map<string,string>>> X;//[t,cidx,pidx]
    for (string time:this->times) {
        unordered_map<string,unordered_map<string,string>> cidmap;
        for (string cid:this->custom_ids) {
            unordered_map<string,string> pidmap;
            for(string pid:this->stream_ids[time]){
                pidmap.emplace(pid,"");
            }
            cidmap.emplace(cid,pidmap);
        }
        X.emplace(time,cidmap);
    }

    vector<vector<vector<int>>> demandInfo;
    this->getDemandInfo(demandInfo);
    vector<vector<int>> siteTimeInfo;
    this->initSiteTimeInfo(siteTimeInfo);
    vector<vector<int>> custTimeNeeds;
    this->getCustTimeNeeds(custTimeNeeds);

    unordered_map<string,Site> sites;
    for(string sid:this->site_ids){
        sites.emplace(sid,Site(sid,this->siteDict[sid],this->times));
    }

    vector<Stream> streams;
    streams.reserve(this->T*this->M*this->stream_ids[this->times[0]].size());

    for(string time:this->times){
        for(string cid:this->custom_ids){
            for(string pid:this->stream_ids[time]){
                int need=this->demandDict[time][cid][pid];
                streams.push_back(Stream(time,cid,pid,need));
            }
        }
    }
    sort(streams.begin(), streams.end(),[](Stream a,Stream b){return a.need>b.need;});
    vector<string> siteQ(this->site_ids);
    sort(siteQ.begin(), siteQ.end(),[&](string a,string b){
        if(this->custSetsId[a].size()==this->custSetsId[b].size()){
            return this->siteDict[a]>this->siteDict[b];
        }
        return this->custSetsId[a].size()<this->custSetsId[b].size();
    });

    unordered_map<string,vector<string>> siteSets(this->siteSetsId);
    for(string cid:this->custom_ids){
        sort(siteSets[cid].begin(), siteSets[cid].end(),[&](string a,string b){
            if(this->custSetsId[a].size()==this->custSetsId[b].size()){
                return this->siteDict[a]>this->siteDict[b];
            }
            return this->custSetsId[a].size()<this->custSetsId[b].size();
        });
    }

    vector<Stream> streams_remain;
    streams_remain.reserve(this->T*this->M*this->stream_ids[this->times[0]].size());

    unordered_map<string,int> usedTimes;
    int n=0;
    int sum=this->T*this->M*this->stream_ids[this->times[0]].size()*0.3;

    for(Stream &stream:streams){
        if(stream.need==0)
            continue;

        bool flag = false;

        for (string sid: siteSets[stream.cid]) {
                if (sites[sid].canConsume(stream) &&((usedTimes[sid] == this->N5 && sites[sid].timeUsed[stream.time] > 0) || usedTimes[sid] < this->N5)) {
//                if(sites[sid].canConsume(stream) && (sites[sid].N5Flows.size()<this->N5 || (sites[sid].N5Flows.size()==this->N5 && sites[sid].timeUsed[time]>0))) {
//                cout<<usedTimes[sid]<<endl;
                if (sites[sid].timeUsed[stream.time] == 0)
                    usedTimes[sid]++;
                sites[sid].consume1(stream);
                X[stream.time][stream.cid][stream.pid] = sid;

                flag = true;
                break;
            }
        }
        if(!flag) {
            streams_remain.push_back(stream);

        }
    }


    for(string sid:this->site_ids){

        for(auto it=sites[sid].timeUsed.begin();it!=sites[sid].timeUsed.end();it++){
            string time=it->first;
            int used=it->second;
            if(sites[sid].N5Flows.size()<this->N5){
                sites[sid].N5Flows.emplace(make_pair(time,used));
                sites[sid].N5Times.emplace(time);
                sites[sid].maxUsed=max(sites[sid].maxUsed,used);
                sites[sid].N95Used=sites[sid].N5Flows.top().second;
            }else{
                sites[sid].N5Flows.emplace(make_pair(time,used));
                sites[sid].N5Times.emplace(time);
                auto p=sites[sid].N5Flows.top();
                sites[sid].N5Flows.pop();
                sites[sid].N5Times.erase(p.first);
                sites[sid].maxUsed=max(sites[sid].maxUsed,used);
                sites[sid].N95Used=sites[sid].N5Flows.top().second;
            }
        }
    }

    unordered_map<string,vector<string>> siteSets1(this->siteSetsId);
    for(string cid:this->custom_ids){
        sort(siteSets1[cid].begin(), siteSets1[cid].end(),[&](string a,string b){
            if(this->custSetsId[a].size()==this->custSetsId[b].size()){
                return this->siteDict[a]>this->siteDict[b];
            }
            return this->custSetsId[a].size()>this->custSetsId[b].size();
        });
    }
    for(Stream &stream:streams_remain){
        if(stream.need==0)
            continue;
        string time=stream.time;
        string cid=stream.cid;
        string pid=stream.pid;
        int minCost=MAX_INT;
        string minSid="";
        for(string sid:siteSets1[cid]){
            if(sites[sid].canConsume(stream)){
                int oldN95=sites[sid].N95Used;
                int newN95=sites[sid].tryConsume(stream);
                int oldCost=this->cost_cal(oldN95,sites[sid].maxUsed,sites[sid].limitUsed);
                int newCost=this->cost_cal(newN95,sites[sid].maxUsed,sites[sid].limitUsed);
                if(newCost-oldCost<minCost){
                    minCost=newCost-oldCost;
                    minSid=sid;
                }
            }
        }
        sites[minSid].consume(stream);
        X[time][cid][pid]=sites[minSid].sid;
    }
//
//    this->io.output(X, this->dataPath+"/first.txt");
//
//    ////迁移
//
//    vector<vector<int>> siteTimeUsed(this->N, vector<int>(this->T, 0));
//    this->getSiteTimeUsed(siteTimeUsed, X);
//    unordered_map<int, int> N95Flows;
//    this->get_N95Flows(N95Flows, siteTimeUsed);
//    for (int i = 0; i < this->N; i++) {
//        int sidx = siteQ[i];
//
//        int T = this->T;
//        int n = THREAD_WORKERS;
//        int startT = 0;
//        vector<thread> Thread;
//        for (int j = 0; j < THREAD_WORKERS; j++) {
//            Thread.push_back(thread([&, startT, T, n]() {
//                int sT = startT, eT = startT + int(T / n);
//                for (int t = sT; t < eT; t++) {
//                    string time = this->times[t];
//                    int stream_nums = this->stream_ids[time].size();
//                    if (siteTimeUsed[sidx][t] > N95Flows[sidx]) {
//                        continue;
//                    }
//                    for (int cidx = 0; cidx < this->M; cidx++) {
//                        string cid = this->custom_ids[cidx];
//                        vector<int> brotherSidxs = this->siteSets[cidx];
//                        vector<pair<int, int>> brotherQ;
//                        for (int brotherSidx: brotherSidxs) {
//                            int canFlow = 0;
//                            string bsid = this->site_ids[brotherSidx];
//                            if (brotherSidx != sidx) {
//                                int tmp = N95Flows[brotherSidx];
////                                tmp = min(this->siteDict[bsid], max(this->base_cost, tmp));
//                                if (siteTimeUsed[brotherSidx][t] <= tmp) {
//                                    canFlow = tmp - siteTimeUsed[brotherSidx][t];
//                                } else {
//                                    canFlow = this->siteDict[bsid] - siteTimeUsed[brotherSidx][t];
//                                }
//                                brotherQ.push_back(make_pair(brotherSidx, canFlow));
//                            }
//                        }
//                        sort(brotherQ.begin(), brotherQ.end(),
//                             [](pair<int, int> x, pair<int, int> y) { return x.second > y.second; });
//                        int n = brotherQ.size();
//                        for (int pidx = 0; pidx < stream_nums; pidx++) {
//                            string pid = this->stream_ids[time][pidx];
//                            if (X[t][cidx][pidx] != sidx)
//                                continue;
//                            int need = this->demandDict[time][cid][pid];
//                            for (int bri = 0; bri < n; bri++) {
//                                int brotherSidx = brotherQ[bri].first;
//                                int canFlow = brotherQ[bri].second;
//                                if (canFlow >= need) {
//                                    siteTimeInfo[brotherSidx][t] -= need;
//                                    siteTimeUsed[brotherSidx][t] += need;
//                                    brotherQ[bri].second -= need;
//                                    siteTimeInfo[sidx][t] += need;
//                                    siteTimeUsed[sidx][t] -= need;
//                                    X[t][cidx][pidx] = brotherSidx;
//                                    break;
//                                }
//                            }
//                        }
//                    }
//                }
//            }));
//            startT += int(T / n);
//            T -= int(T / n);
//            n -= 1;
//        }
//        for (int j = 0; j < THREAD_WORKERS; j++) {
//            Thread[j].join();
//        }
//        vector<int> tmp(siteTimeUsed[sidx]);
//        sort(tmp.begin(), tmp.end());
//        N95Flows[sidx] = tmp[this->N95];
//    }

    unordered_map<string,vector<int>> siteTimeUsed;
    for(string sid:this->site_ids){
        siteTimeUsed.emplace(sid,vector<int>(this->T,0));
    }
    this->getSiteTimeUsed(siteTimeUsed, X);
    unordered_map<string, int> N95Flows;
    this->get_N95Flows(N95Flows, siteTimeUsed);

    vector<string> sidxs(this->site_ids);

    sort(sidxs.begin(), sidxs.end(),[&](string a,string b){return N95Flows[a]>N95Flows[b];});
    vector<string> N90Sidxs;
    for(int i=0;i<10;i++){
        N90Sidxs.push_back(sidxs[i]);
    }
    this->io.output(X,N90Sidxs, this->solutionPath);
}

double Solution::cost_cal(int N95Used, int maxUsed,int limitUsed) {
    double cost=0;
    if (maxUsed==0) {
        cost = 0;
    } else if (N95Used <= this->base_cost) {
        cost = this->base_cost;
    } else {
        cost = pow(N95Used - this->base_cost, 2) / limitUsed + N95Used;
    }
    return cost;
}

void Solution::get_N95Flows(unordered_map<string, int> &N95Flows, unordered_map<string,vector<int>> &siteTimeUsed) {

    unordered_map<string,vector<int>> siteFlow(siteTimeUsed);
    for (int sidx = 0; sidx < this->N; sidx++) {
        string sid=this->site_ids[sidx];
        sort(siteFlow[sid].begin(), siteFlow[sid].end());
        int flow = siteFlow[sid][this->N95];
        N95Flows.insert(make_pair(sid, flow));
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

void Solution::getCustSets(unordered_map<string, vector<string>> &custSets) {
    //{sidx:cidx}
    for (int sidx = 0; sidx < this->N; sidx++) {
        string sid = this->site_ids[sidx];
        vector<string> custSet;
        for (int cidx = 0; cidx < this->M; cidx++) {
            string cid = this->custom_ids[cidx];
            if (this->qosDict[sid][cid] < this->qos_constraint) {
                custSet.push_back(cid);
            }
        }
        custSets.insert(make_pair(sid, custSet));
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

void Solution::getSiteSets(unordered_map<string, vector<string>> &siteSets) {

    for (int cidx = 0; cidx < this->M; cidx++) {
        string cid = this->custom_ids[cidx];
        vector<string> siteSet;
        for (int sidx = 0; sidx < this->N; sidx++) {
            string sid = this->site_ids[sidx];
            if (this->qosDict[sid][cid] < this->qos_constraint) {
                siteSet.push_back(sid);
            }
        }
        siteSets.insert(make_pair(cid, siteSet));
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

//vector<int> Solution::getSiteQ() {
//    vector<int> siteQ;
//    siteQ.reserve(this->N);
//    for (int sidx = 0; sidx < this->N; sidx++) {
//        siteQ.push_back(sidx);
//    }
//    sort(siteQ.begin(), siteQ.end(), [&](int x, int y) {
//        if (this->siteDict[this->site_ids[x]] == this->siteDict[this->site_ids[y]]) {
//            return this->custSets[x].size() < this->custSets[y].size();
//        }
//        return this->siteDict[this->site_ids[x]] > this->siteDict[this->site_ids[y]];
//    });
//    return siteQ;
//
//}
//vector<int> Solution::getFullSite(){
//    vector<int> siteQ;
//    siteQ.reserve(this->N);
//    for (int sidx = 0; sidx < this->N; sidx++) {
//        siteQ.push_back(sidx);
//    }
//    sort(siteQ.begin(), siteQ.end(), [&](int x, int y) {
//        if (this->custSets[x].size() == this->custSets[y].size()) {
//            return this->siteDict[this->site_ids[x]] > this->siteDict[this->site_ids[y]];
//        }
//        return this->custSets[x].size() > this->custSets[y].size();
//    });
//    return siteQ;

//}
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

void Solution::getSiteTimeUsed(unordered_map<string,vector<int>> &siteTimeUsed, unordered_map<string,unordered_map<string,unordered_map<string,string>>> &X) {

    for (int t = 0; t < this->T; t++) {
        string time = this->times[t];
        int stream_nums = this->stream_ids[time].size();
        for (int cidx = 0; cidx < this->M; cidx++) {
            string cid = this->custom_ids[cidx];
            for (int pidx = 0; pidx < stream_nums; pidx++) {
                string pid = this->stream_ids[time][pidx];
                string sid = X[time][cid][pid];
                if (sid.compare("")!=0)
                    siteTimeUsed[sid][t] += this->demandDict[time][cid][pid];
            }
        }
    }

}

vector<int> Solution::beibao(int t,int sidx,int cidx,int canFlow,vector<vector<vector<int>>> &X,vector<vector<vector<int>>> &demandInfo){
    string time=this->times[t];
    int N= this->stream_ids[time].size();
    vector<vector<int>> dp(N+1,vector<int>(canFlow+1,0));
    for(int i=1;i<=N;i++){
        int need=demandInfo[t][cidx][i-1];
        for(int j=1;j<=canFlow;j++){
            if(j>=need){
                dp[i][j]=max(dp[i-1][j],dp[i-1][j-need]+need);
            }else{
                dp[i][j]=dp[i-1][j];
            }
        }
    }
    vector<int> x;
    for(int i=N;i>0;i--){
        int need=demandInfo[t][cidx][i-1];
        if(dp[i][canFlow]!=dp[i-1][canFlow]){
            x.push_back(i-1);
            canFlow-=need;
        }
    }
    return x;

}
void Solution::distribute(int t,int sidx,int cidx,int canFlow,vector<vector<vector<int>>> &X,vector<vector<vector<int>>> &demandInfo,vector<vector<int>>&custTimeNeeds,vector<vector<int>>&siteTimeInfo){
    string time=this->times[t];
    int stream_nums=this->stream_ids[time].size();

    if(canFlow>=custTimeNeeds[cidx][t] || canFlow>1000) {
        for (int pidx = 0; pidx < stream_nums; pidx++) {
            int need = demandInfo[t][cidx][pidx];
            if (need > 0 && siteTimeInfo[sidx][t]>=need) {
                siteTimeInfo[sidx][t] -= need;
                X[t][cidx][pidx] = sidx;
                custTimeNeeds[cidx][t] -= need;
                demandInfo[t][cidx][pidx] = 0;
            }
        }
    }else{
        vector<int> plist=this->beibao(t,sidx,cidx,canFlow,X,demandInfo);
        for(int pidx:plist){
            int need = demandInfo[t][cidx][pidx];
            if (need > 0) {
                siteTimeInfo[sidx][t] -= need;
                X[t][cidx][pidx] = sidx;
                custTimeNeeds[cidx][t] -= need;
                demandInfo[t][cidx][pidx] = 0;
            }
        }
    }
}