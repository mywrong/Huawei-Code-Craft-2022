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

}

