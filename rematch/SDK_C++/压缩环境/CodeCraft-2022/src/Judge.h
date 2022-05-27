//
// Created by Pot on 2022/4/1.
//

#ifndef CODECRAFT_2022_JUDGE_H
#define CODECRAFT_2022_JUDGE_H
#include <string>
#include "IO.h"

using namespace std;
class Judge {
private:
    IO io;
    unordered_map<string, unordered_map<string, unordered_map<string, int>>> demandDict;
    unordered_map<string, int> siteDict;
    unordered_map<string, unordered_map<string, int>> qosDict;
    unordered_map<string, int> configDict;
    vector<string> custom_ids;
    vector<string> site_ids;
    vector<string> times;
    unordered_map<string, vector<string>> stream_ids;
    unordered_map<int, vector<int>> siteSets;
    unordered_map<int, vector<int>> custSets;
    int qos_constraint;
    int base_cost;
    int M;
    int T;
    int N;
    int N5;
    int N95;
public:
    Judge(string dataPath, string solutionPath);
    int getScore();

    unordered_map<string,vector<int>> initSiteTimeInfo();
};


#endif //CODECRAFT_2022_JUDGE_H
