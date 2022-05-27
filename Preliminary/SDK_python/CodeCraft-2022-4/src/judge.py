import math
import re

from IO import IO
class Judge:
    def __init__(self,datapath,outpath):
        self.io=IO(datapath,outpath)
        self.outpath=outpath
        self.custom_id, self.demandInfo=self.io.demandReader()
        self.site_id,self.siteInfo,self.siteId2idx=self.io.siteReader()
        self.qosInfo=self.io.qosReader()
        self.qos_constraint=self.io.configReader()
        self.T=len(self.demandInfo)
        self.N=len(self.site_id)
        self.M=len(self.custom_id)
        self.siteDict={}
        for idx,id in enumerate(self.site_id):
            self.siteDict[id]=self.siteInfo[idx]

    def judge(self):
        m = len(self.demandInfo)
        n = len(self.demandInfo[0])
        flowlists=[]
        with open(self.outpath, "r") as f:
            for line in f.readlines():
                flowlist = []
                line=re.split('[:<,>\'\n\']',line)
                pos=0
                for i in range(len(line)):
                    if line[i]!='':
                        line[pos]=line[i]
                        pos+=1

                line=line[:pos]
                line=line[1:]
                for i in range(0,len(line),2):
                    flowlist.append([line[i],int(line[i+1])])
                flowlists.append(flowlist)

        k=0
        for i in range(m):
            tmpk=k
            for j in range(n):
                needflow=self.demandInfo[i][j]
                for duple in flowlists[k]:
                    site_id=duple[0]
                    flow=duple[1]
                    if flow >self.siteDict[site_id]:
                        print('error1')
                        return
                    needflow-=flow
                    self.siteDict[site_id]-=flow
                if needflow>0:
                    print('error2')
                    return
                k+=1
            for jj in range(tmpk,k):
                flowlist=flowlists[jj]
                for duple in flowlist:
                    site_id = duple[0]
                    flow = duple[1]
                    self.siteDict[site_id]+=flow

        print('OK')
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
            t95=math.ceil(self.T*0.95)
            for i in range(self.N):
                data[i].sort()
                score+=data[i][t95]
            print(score)


if __name__=='__main__':
    judge=Judge('D:\Doc\Huawei2022\线下调试数据\data','D:\Doc\Huawei2022\线下调试数据\data/solution.txt')
    judge.judge()
    judge.get_score()
