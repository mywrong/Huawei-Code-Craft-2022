import math
from collections import OrderedDict

import matplotlib.pyplot as plt
from IO import IO
import numpy as np
import re
import matplotlib.colors as mcolors

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
        self.siteSets, self.custSets = self.match()
    def num_95(self):
        data=[]
        for i in range(self.T):
            for j in range(self.M):
                data.append(self.demandInfo[i][j])
        data.sort()
        print(data[math.ceil(self.T*self.M*0.95)])

    def siteQueue(self):
        cnts = []
        for i in range(self.N):
            cnt = 0
            for j in range(self.M):
                if self.qosInfo[i][j] < self.qos_constraint:
                    cnt += 1
            cnts.append(cnt)
        siteQ = [i for i in range(self.N)]
        siteQ = sorted(siteQ, key=lambda x: (cnts[x], siteQ))

        while cnts[siteQ[0]] == 0:
            siteQ.pop(0)

        return siteQ


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

            x = 0.5 + np.arange(len(self.siteQ))
            y=[data[i][t95] for i in self.siteQ]
            print(self.siteQ[10])
            # plot
            fig, ax = plt.subplots()

            ax.bar(x, y, width=1, edgecolor="white", linewidth=0.7)

            plt.show()

            print(score)


    def drawdemandInfo(self,demandInfo):
        plt.ion()
        x=[i for i in range(self.T)]
        demandInfo=np.array(demandInfo)
        for i in range(self.M):
            plt.plot(x,demandInfo[:,i])

    def drawSitTimeUsed(self,sitTimeUsed):
        plt.ion()
        T=[i for i in range(self.T)]
        colors = list(mcolors.TABLEAU_COLORS.keys())
        for sidx in range(self.N):
            plt.plot(T,sitTimeUsed[sidx],color=mcolors.TABLEAU_COLORS[colors[len(self.custSets[sidx])]],label="{}".format(len(self.custSets[sidx])))
            plt.ylim(0,2000)
            plt.savefig("D:\Doc\Huawei2022\siteTimeInfo/{}-{}.png".format(len(self.custSets[sidx]),self.site_id[sidx]))
            plt.pause(0.01)


    def match(self):
        siteSets=[[] for _ in range(self.M)]   #每个客户节点可以访问的边缘节点集合
        custSets = [[] for _ in range(self.N)] #每个边缘节点可以提供服务的客户节点集合
        for i in range(self.N):
            for j in range(self.M):
                if self.qosInfo[i][j]<self.qos_constraint:
                    siteSets[j].append(i)
                    custSets[i].append(j)

        return siteSets,custSets
if __name__=='__main__':
    analyze=Analyze('D:\Doc\Huawei2022\线下调试数据\data','D:\Doc\Huawei2022\线下调试数据\data/solution.txt')
    analyze.get_score()
    # analyze.num_95()