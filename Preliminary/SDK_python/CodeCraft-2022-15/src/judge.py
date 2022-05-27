import math
import re
from IO import IO


class Judge:
    def __init__(self,datapath,outpath):
        self.io=IO(datapath,outpath)
        self.outpath=outpath
        self.custom_id, self.site_id, self.demandInfo, self.siteInfo, self.qosInfo, self.qos_constraint = self.io.fileReader()
        self.T=len(self.demandInfo)
        self.N=len(self.site_id)
        self.M=len(self.custom_id)
        self.siteDict={}
        for idx,id in enumerate(self.site_id):
            self.siteDict[id]=self.siteInfo[idx]

    def judge(self):
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
        for t in range(self.T):
            tmpk=k
            for cidx in range(self.M):
                needflow=self.demandInfo[t][cidx]
                for duple in flowlists[k]:
                    site_id=duple[0]
                    flow=duple[1]
                    if flow >self.siteDict[site_id]:
                        print('超过边缘节点流量限制：时刻{}，custmon:{}，{}超过流量：{}'.format(t+1,self.custom_id[cidx],site_id,flow))
                        return
                    needflow-=flow
                    self.siteDict[site_id]-=flow
                if needflow>0:
                    print('有客户流量未分配:时刻{}，custmon:{}，剩余流量：{}'.format(t+1,self.custom_id[cidx],needflow))
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
        siteId2Idx={}
        for i,id in enumerate(self.site_id):
            siteId2Idx[id]=i
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
                        siteIdx=siteId2Idx[siteId]
                        data[siteIdx][t]+=siteflow

            score=0
            t95=math.ceil(self.T*0.95)-1
            for i in range(self.N):
                data[i].sort()
                score+=data[i][t95]
            print(score)


if __name__=='__main__':
    judge=Judge(dataPath,solutionPath)
    judge.judge()
    judge.get_score()
