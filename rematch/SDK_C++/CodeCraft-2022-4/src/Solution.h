//
// Created by Pot on 2022/3/31.
//

#ifndef CODECRAFT_2022_SOLUTION_H
#define CODECRAFT_2022_SOLUTION_H

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include<vector>
#include <unordered_map>
#include "IO.h"

using namespace std;

class Solution {
private:
    string dataPath;
    string solutionPath;
    IO io;
    unordered_map<string, unordered_map<string, unordered_map<string, int>>> demandDict; //demandDict:[time,cid,pid]
    unordered_map<string, int> siteDict;
    unordered_map<string, unordered_map<string, int>> qosDict; //qosDict:[sid,cid]
    unordered_map<string, int> configDict;
    int M, N, T,qos_constraint,base_cost;
    vector<string> custom_ids, site_ids, times;
    unordered_map<string,vector<string>> stream_ids;
    unordered_map<int, vector<int>> siteSetsIdx;
    unordered_map<int, vector<int>> custSetsIdx;
    unordered_map<string, vector<string>> siteSetsId;
    unordered_map<string, vector<string>> custSetsId;
    int N5,N95;


public:
    Solution(string dataPath,string solutionPath);
    void work();
    void getDemandInfo(vector<vector<vector<int>>> &demandInfo);

    void initSiteTimeInfo(vector<vector<int>> &siteTimeInfo);

    void getCustSets(unordered_map<int, vector<int>> &custSets);

    void getSiteSets(unordered_map<int, vector<int>> &siteSets);

    vector<vector<int>> topK(vector<vector<int>> &v, int k);


    vector<int> getSiteQ();

    void getCustTimeNeeds(vector<vector<int>> &custTimeNeeds);

    void getCustTimeStreamSort(vector<vector<vector<int>>> &custTimeStreamSort);

    void get_N95Flows(unordered_map<string, int> &N95Flows, unordered_map<string,vector<int>> &siteTimeUsed);

    void getSiteTimeUsed(unordered_map<string,vector<int>> &siteTimeUsed, unordered_map<string,unordered_map<string,unordered_map<string,string>>> &X);

    static bool cmp(pair<int, int>& m, pair<int, int>& n) {
        return m.second < n.second;
    }

    void reDistribute(int sidx, vector<vector<int>> &siteTimeUsed, vector<vector<int>> &siteTimeInfo,
                      unordered_map<int, int> &N95Flows, vector<vector<vector<int>>> &X);

    double cost_cal(int N95Used, int maxUsed,int limitUsed);

    vector<int> getFullSite();

    vector<int>
    beibao(int t, int sidx, int cidx, int canFlow, vector<vector<vector<int>>> &X,
           vector<vector<vector<int>>> &demandInfo);

    void distribute(int t, int sidx, int cidx, int canFlow, vector<vector<vector<int>>> &X,
                    vector<vector<vector<int>>> &demandInfo, vector<vector<int>> &custTimeNeeds,
                    vector<vector<int>> &siteTimeInfo);

    void getCustSets(unordered_map<string, vector<string>> &custSets);

    void getSiteSets(unordered_map<string, vector<string>> &siteSets);
};


#endif //CODECRAFT_2022_SOLUTION_H
