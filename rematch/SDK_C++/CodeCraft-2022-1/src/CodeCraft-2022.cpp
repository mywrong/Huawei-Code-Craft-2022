#include <iostream>
#include "Solution.h"
#include "Judge.h"
#include <functional>
#include <algorithm>
#include <queue>
#include <thread>
#include <mutex>

using namespace std;

bool cmp(pair<int, int>& m, pair<int, int>& n) {
    return m.second < n.second;
}

int main() {
    int TEST=1;
    string dataPath,solutionPath;
    if(TEST==0){
        dataPath="/data";
        solutionPath="/output/solution.txt";
    }else if(TEST==1){
        dataPath="D:\\Doc\\Huawei2022\\rematch\\data";
        solutionPath="D:\\Doc\\Huawei2022\\rematch\\data\\solution.txt";
    } else{
        dataPath="/home/pot/Documents/data";
        solutionPath="/home/pot/Documents/data/solution.txt";
    }
    clock_t s= clock();
    Solution solution(dataPath,solutionPath);
    solution.work();
    cout<<(double)(clock()-s)/CLOCKS_PER_SEC<<endl;
    if(TEST==1 || TEST==2){
        Judge judge(dataPath,solutionPath);
        cout<<"score:"<<judge.getScore()<<endl;
    }

//    vector<int> a;
//    for(int i=99;i>-1;i--){
//        a.push_back(i);
//    }
//    for(int i:a){
//        cout<<i<<" ";
//    }
//    cout<<endl;
//    int n=8;
//    int T=100;
//    int startT=0;
//    vector<thread> Thread;
//    vector<pair<int,int>> ts(8);
//    mutex mt;
//    for(int i=0;i<8;i++){
//        ts[i]= make_pair(startT,startT+int(T/n));
//        startT+=int(T/n);
//        T-=int(T/n);
//        n-=1;
//    }
//    for(int i=0;i<8;i++){
//
//        Thread.push_back(thread([&,i](){
//
//            int sT=ts[i].first,eT=ts[i].second;
//            cout<<"线程:"<<i<<",st:"<<sT<<",eT:"<<eT<<endl;
//
//            for(int t=sT;t<eT;t++){
//                a[t]=t;
//            }
//        }));
//
//
//    }
//    for(int i=0;i<8;i++){
//        Thread[i].join();
//    }
//    for(int i:a){
//        cout<<i<<" ";
//    }



}

