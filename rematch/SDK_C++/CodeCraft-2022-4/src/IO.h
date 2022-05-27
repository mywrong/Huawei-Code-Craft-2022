//
// Created by Pot on 2022/3/31.
//

#ifndef CODECRAFT_2022_IO_H
#define CODECRAFT_2022_IO_H

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include<vector>
#include <unordered_map>

using namespace std;

class IO {
private:

    string dataPath;
    string solutionPath;
    string demandPath;
    string qosPath;
    string sitePath;
    string configPath;
    int T = 0, M = 0, N = 0;
    vector<string> custom_ids, site_ids, times;
    unordered_map<string,vector<string>> stream_ids;
public:
    IO();
    IO(string dataPath, string solutionPath);

    void demandReader(unordered_map<string, unordered_map<string, unordered_map<string, int>>> &demandDict);

    void siteReader(unordered_map<string, int> &siteDict);

    void qosReader(unordered_map<string, unordered_map<string, int>> &qosDict);

    void configReader(unordered_map<string, int> &configDict);

    int getT() const;

    int getM() const;

    int getN() const;

    vector<string> getCustom_ids();

    vector<string>  getSite_ids();

    vector<string> getTimes();

    unordered_map<string,vector<string>> getStream_ids();

    void output(unordered_map<string,unordered_map<string,unordered_map<string,string>>> &X,vector<string> &N90Sidxs,string outPath);

    static void exists_test(const string &name);

    vector<vector<vector<string>>> SolutionReader();

    vector<unordered_map<string,int>> getTimeStreamId2Pidx();

    string stripr(string line);
};


#endif //CODECRAFT_2022_IO_H
