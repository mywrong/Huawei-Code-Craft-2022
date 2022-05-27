import math
import matplotlib.pyplot as plt
from IO import IO
import numpy as np
import re
class Analyze:
    def __init__(self,datapath,outpath):
        self.io=IO(datapath,outpath)
        self.custom_id, self.demandInfo=self.io.demandReader()
        self.site_id, self.siteInfo, self.siteId2idx = self.io.siteReader()
        self.qosInfo=self.io.qosReader()
        self.qos_constraint=self.io.configReader()
        self.T = len(self.demandInfo)
        self.N = len(self.site_id)
        self.M = len(self.custom_id)
        self.outpath=outpath
        self.siteQ=self.siteQueue()
    def num_95(self):
        data=[]
        for i in range(self.T):
            for j in range(self.M):
                data.append(self.demandInfo[i][j])
        data.sort()
        print(data[math.ceil(self.T*self.M*0.95)])
    def siteQueue(self):
        cnts=[]
        for i in range(self.N):
            cnt=0
            for j in range(self.M):
                if self.qosInfo[i][j] < self.qos_constraint:
                    cnt+=1
            cnts.append(cnt)
        siteQ=[i for i in range(self.N)]
        siteQ=sorted(siteQ,key=lambda x:(-cnts[x],siteQ))


        # x = 0.5 + np.arange(self.N)
        # y=sorted(cnts,reverse=True)
        # # plot
        # fig, ax = plt.subplots()
        #
        # ax.bar(x, y, width=1, edgecolor="white", linewidth=0.7)
        #
        # plt.show()
        return siteQ
    def get_score(self):
        m = len(self.demandInfo)
        n = len(self.demandInfo[0])
        flowlists = []
        with open(self.outpath, "r") as f:
            for line in f.readlines():
                flowlist = []
                line = re.split('[:<,>\'\n\']', line)
                pos = 0
                for i in range(len(line)):
                    if line[i] != '':
                        line[pos] = line[i]
                        pos += 1

                line = line[:pos]
                line = line[1:]
                for i in range(0, len(line), 2):
                    flowlist.append([line[i], int(line[i + 1])])
                flowlists.append(flowlist)

            data=[[0]*self.T for _ in range(self.N)]
            for i in range(0,self.T*self.M,self.M):
                t=i//self.M

                for j in range(i,i+self.M):
                    flowlist=flowlists[j]
                    for flow in flowlist:
                        siteId=flow[0]
                        siteflow=flow[1]
                        siteIdx=self.siteId2idx[siteId]
                        data[siteIdx][t]+=siteflow

            score=0
            t95=math.ceil(self.T*0.95)-1
            for i in range(self.N):
                data[i].sort()
                score+=data[i][t95]
            x = 0.5 + np.arange(self.N)
            y=[data[i][t95] for i in self.siteQ]
            # plot
            fig, ax = plt.subplots()

            ax.bar(x, y, width=1, edgecolor="white", linewidth=0.7)

            plt.show()

            print(score)
if __name__=='__main__':
    analyze=Analyze('D:\Doc\Huawei2022\线下调试数据\data','D:\Doc\Huawei2022\线下调试数据\data/solution.txt')
    analyze.get_score()