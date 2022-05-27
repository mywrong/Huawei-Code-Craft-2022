//
// Created by Pot on 2022/3/31.
//

#include "IO.h"
#include <iostream>
#include <string>
#include <cstdio>


using namespace std;
IO::IO() {}
IO::IO(string dataPath, string solutionPath) {
    this->dataPath = dataPath;
    this->solutionPath = solutionPath;
    this->demandPath = dataPath + "/demand.csv";
    this->qosPath = dataPath + "/qos.csv";
    this->sitePath = dataPath + "/site_bandwidth.csv";
    this->configPath = dataPath + "/config.ini";

}

void IO::demandReader(unordered_map<string, unordered_map<string, unordered_map<string, int>>> &demandDict) {
    ifstream inFile(this->demandPath, ios::in);
    if (!inFile) {
        cout << "fail to open the demand file!" << endl;
        exit(1);
    }

    string line;
    int row = 0;
    while (getline(inFile, line)) {
        if (row == 0) {
            string field;
            istringstream sin(line);
            getline(sin, field, ',');
            getline(sin, field, ',');
            while (getline(sin, field, ',')) {
                field=this->stripr(field);
                this->custom_ids.push_back(field);
                this->M++;
            }

        } else {
            istringstream sin(line);
            string mtime;
            getline(sin, mtime, ',');
            if (demandDict.find(mtime) == demandDict.end()) {
                this->times.push_back(mtime);
            }

            string streamId;
            getline(sin, streamId, ',');
            this->stream_ids.emplace(mtime,vector<string>());
            this->stream_ids[mtime].push_back(streamId);

            for (int i = 0; i < this->M-1; i++) {
                string cid = this->custom_ids[i];
                string field;
                getline(sin, field, ',');
                int f = atoi(field.c_str());
                demandDict.emplace(mtime,unordered_map<string, unordered_map<string, int>>());
                demandDict[mtime].emplace(cid,unordered_map<string, int>());
                demandDict[mtime][cid].insert(make_pair(streamId,f));
            }
            string cid = this->custom_ids[this->M-1];
            string field;
            getline(sin, field, ',');
            field=this->stripr(field);
            int f = atoi(field.c_str());
            demandDict.emplace(mtime,unordered_map<string, unordered_map<string, int>>());
            demandDict[mtime].emplace(cid,unordered_map<string, int>());
            demandDict[mtime][cid].insert(make_pair(streamId,f));
        }
        row++;
    }
    this->T=demandDict.size();

}

void IO::siteReader(unordered_map<string,int> &siteDict) {
    ifstream inFile(this->sitePath, ios::in);
    if (!inFile) {
        cout << "fail to open the site file!"  << endl;
        exit(1);
    }

    string line;
    getline(inFile, line);
    while (getline(inFile, line)){
        istringstream sin(line);
        string siteId;
        getline(sin, siteId, ',');
        string field;
        getline(sin, field, ',');
        field=this->stripr(field);
        int f= atoi(field.c_str());
        siteDict.insert(make_pair(siteId,f));
        this->site_ids.push_back(siteId);
        this->N++;
    }
}

void IO::qosReader(unordered_map<string,unordered_map<string,int>> &qosDict) {
    ifstream inFile(this->qosPath, ios::in);
    if (!inFile) {
        cout << "fail to open the qos file!"  << endl;
        exit(1);
    }
    vector<string> customId;
    string line;
    getline(inFile, line);
    istringstream sin(line);
    string field;
    getline(sin, field, ',');
    for(int i=0;i< this->M;i++){
        getline(sin, field, ',');
        field=this->stripr(field);
        customId.push_back(field);
    }
    while (getline(inFile, line)){
        istringstream sin(line);
        string siteId;
        getline(sin, siteId, ',');
        qosDict.emplace(siteId,unordered_map<string,int>());
        for(int i=0;i<this->M-1;i++){
            getline(sin, field, ',');
            int f= atoi(field.c_str());
            qosDict[siteId].insert(make_pair(customId[i],f));
        }
        getline(sin, field, ',');
        field=this->stripr(field);
        int f= atoi(field.c_str());
        qosDict[siteId].insert(make_pair(customId[this->M-1],f));
    }

}

void IO::configReader(unordered_map<string,int>&configDict){
    ifstream inFile(this->configPath, ios::in);
    if (!inFile) {
        cout << "fail to open the config file!"  << endl;
        exit(1);
    }
    string line;
    getline(inFile, line);
    while(getline(inFile, line)){
        istringstream sin(line);
        string key,field;
        int value;
        getline(sin, key, '=');
        getline(sin, field, '=');
        field=this->stripr(field);
        value= atoi(field.c_str());
        configDict.insert(make_pair(key,value));
    }

}

void IO::output(unordered_map<string,unordered_map<string,unordered_map<string,string>>> &X,vector<string> &N90Sidxs,string outPath) {
    IO::exists_test(outPath);
    ofstream outFile;
    outFile.open(outPath);
    if(!outFile.is_open()){
        cout<<"fail to open the output file!" <<endl;
        return;
    }
    for(int i=0;i<10;i++){
        outFile<<N90Sidxs[i];
        if(i<9){
            outFile<<",";
        }else{
            outFile<<"\n";
        }
    }
    for(int t=0;t< this->T;t++){
        string time=this->times[t];
        int stream_nums=this->stream_ids[time].size();
        for(int cidx=0;cidx<this->M;cidx++){
            string cid=this->custom_ids[cidx];
            outFile<<this->custom_ids[cidx]<<":";
            unordered_map<string,vector<string>> line(this->N);
            for(int pidx=0;pidx<stream_nums;pidx++){
                string pid=this->stream_ids[time][pidx];
                string sid=X[time][cid][pid];
                if(sid.compare("")!=0){
                    line.emplace(sid,vector<string>());
                    line[sid].push_back(pid);
                }
            }
            for(auto iter=line.begin();iter!=line.end();){
                string sid=iter->first;
                vector<string> ll=iter->second;
                outFile<<"<"<<sid<<",";
                int lln=ll.size();
                for(int i=0;i<lln;i++){
                    outFile<<ll[i];
                    if(i<lln-1){
                        outFile<<",";
                    }else{
                        outFile<<">";
                    }
                }
                if(++iter!=line.end()){
                    outFile<<",";
                }
            }
            if(t< this->T-1 || cidx< this->M-1){
                outFile<<"\n";
            }
        }
    }
    outFile<<flush;
    outFile.close();
}
vector<vector<vector<string>>> IO::SolutionReader(){
    vector<unordered_map<string,int>> timeStreamId2Pidx=this->getTimeStreamId2Pidx();

    vector<vector<vector<string>>> X;
    for(int t=0;t<this->T;t++){
        string time=this->times[t];
        vector<vector<string>> custStream;
        for(int cidx=0;cidx< this->M;cidx++){
            int stream_nums=this->stream_ids[time].size();
            custStream.push_back(vector<string>(stream_nums));
        }
        X.push_back(custStream);
    }
    ifstream inFile(this->solutionPath, ios::in);
    if (!inFile) {
        cout << "fail to open the solution file!"  << endl;
        exit(1);
    }
    for(int t=0;t< this->T;t++){
        string time=this->times[t];
        for(int cidx=0;cidx< this->M;cidx++){
            string line;
            getline(inFile, line);
            istringstream sin(line);
            getline(sin,line,':');
            getline(sin,line,':');
            istringstream ssin(line);
            while(getline(ssin,line,'>')){
                istringstream sssin(line);
                getline(sssin,line,',');
                while(line.compare("")==0)
                    getline(sssin,line,',');
                string sid= line.substr(1);
                while(getline(sssin,line,',')){
                    string streamId=line;
                    int pidx=timeStreamId2Pidx[t][streamId];
                    X[t][cidx][pidx]=sid;
                }
            }
        }
    }
    return X;
}
vector<unordered_map<string,int>> IO::getTimeStreamId2Pidx(){
    vector<unordered_map<string,int>> timeStreamId2Pidx;
    for(int t=0;t< this->T;t++){
        unordered_map<string,int> streamId2Pidx;
        string time=this->times[t];
        int stream_nums=this->stream_ids[time].size();
        for(int pidx=0;pidx<stream_nums;pidx++){
            string pid=this->stream_ids[time][pidx];
            streamId2Pidx.insert(make_pair(pid,pidx));
        }
        timeStreamId2Pidx.push_back(streamId2Pidx);
    }
    return timeStreamId2Pidx;
}
int IO::getM() const {
    return this->M;
}

int IO::getN() const {
    return this->N;
}

int IO::getT() const {
    return this->T;
}

vector<string> IO::getCustom_ids() {
    return this->custom_ids;
}

vector<string> IO::getSite_ids() {
    return this->site_ids;
}

vector<string> IO::getTimes() {
    return this->times;
}

unordered_map<string,vector<string>> IO::getStream_ids(){
    return this->stream_ids;
}

inline void IO::exists_test (const std::string& name) {

    if(remove(name.c_str())==0){
        cout<<"remove success"<<endl;
    }else{
        cout<<"remove fail"<<endl;
    }

}

string IO::stripr(string line){
    istringstream sin(line);
    string field;
    getline(sin,field,'\r');
    return field;
}