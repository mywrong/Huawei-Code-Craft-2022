import math
from collections import defaultdict
from IO import IO
from MyHeap import MyHeap
class CodeCraft:
    def __init__(self,datapath,outpath):
        self.io=IO(datapath,outpath)
        self.custom_id, self.demandInfo=self.io.demandReader()
        self.site_id,self.siteInfo,_=self.io.siteReader()
        self.qosInfo = self.io.qosReader()
        self.qos_constraint = self.io.configReader()

        self.T = len(self.demandInfo)
        self.N = len(self.site_id)
        self.M = len(self.custom_id)
        self.siteUsedFreq=[0 for _ in range(self.N)]
        self.siteSets,self.custSets=self.match()
        self.siteQ,self.siteRank,self.sitePos=self.siteQueue()
        self.freqThres=math.ceil(self.T*0.05)-1
        self.N95=self.num_95()
        self.timeRank=self.rank()
    def work(self):
        out=[]
        for row,demand in enumerate(self.demandInfo):
            flowlist=[]
            timeRank = self.timeRank[row]
            for cidx,needflow in enumerate(demand):
                outline = [self.custom_id[cidx]]
                idx=0
                while idx<self.M and self.siteRank[idx]<timeRank:
                    idx+=1

                for i in range(self.sitePos[idx],len(self.siteQ)):
                    sidx=self.siteQ[i]
                    flow=self.siteInfo[sidx]
                    qos = self.qosInfo[sidx][cidx]
                    if needflow > 0 and qos < self.qos_constraint and flow > 0:
                        duple = [(self.site_id[sidx], flow if needflow > flow else needflow)]
                        outline += duple
                        flowlist.append((sidx, duple[0][1]))
                        self.siteInfo[sidx] -= duple[0][1]
                        needflow -= duple[0][1]
                        if needflow == 0:
                            break
                if needflow!=0:
                    for i in range(self.sitePos[idx]):
                        sidx = self.siteQ[i]
                        flow = self.siteInfo[sidx]
                        qos = self.qosInfo[sidx][cidx]
                        if needflow > 0 and qos < self.qos_constraint and flow > 0:
                            duple = [(self.site_id[sidx], flow if needflow > flow else needflow)]
                            outline += duple
                            flowlist.append((sidx, duple[0][1]))
                            self.siteInfo[sidx] -= duple[0][1]
                            needflow -= duple[0][1]
                            if needflow == 0:
                                break
                out.append(outline)
            for duple in flowlist:
                self.siteInfo[duple[0]] += duple[1]

        self.io.output(out)

    def num_95(self):
        data=[]
        for i in range(self.T):
            for j in range(self.M):
                data.append(self.demandInfo[i][j])
        data.sort()
        return data[math.ceil(self.T*self.M*0.95)]

    def match(self):
        siteSets=[[] for _ in range(self.M)]   #???????????????????????????????????????????????????
        custSets = [[] for _ in range(self.N)] #?????????????????????????????????????????????????????????
        for i in range(self.N):
            for j in range(self.M):
                if self.qosInfo[i][j]<self.qos_constraint:
                    siteSets[j].append(i)
                    custSets[i].append(j)

        return siteSets,custSets

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
        while cnts[siteQ[-1]]==0:
            siteQ.pop(-1)
        rank=[0 for _ in range(self.M)]
        rank[0],rank[-1]=0,1
        for i in range(1,len(siteQ)):
            if cnts[siteQ[i]]!=cnts[siteQ[i-1]]:
                rank[self.M-cnts[siteQ[i-1]]]=i/len(siteQ)
        postions=[-1 for _ in range(self.M)]
        postions[self.M-cnts[siteQ[0]]]=0
        for i in range(1,len(siteQ)):
            if cnts[siteQ[i]]!=cnts[siteQ[i-1]]:
                postions[self.M-cnts[siteQ[i]]]=i
        return siteQ,rank,postions

    def rank(self):
        flows=[0 for _ in range(self.T)]
        for i in range(self.T):
            for j in range(self.M):
                flows[i]+=self.demandInfo[i][j]
        tmp=[i for i in range(self.T)]
        tmp=sorted(tmp,key=lambda x:(-flows[x],tmp))
        rank=[0 for _ in range(self.T)]
        for i in range(self.T):
            rank[tmp[i]]=(i+1)/self.T
        return rank






# codecraft=CodeCraft('D:\Doc\Huawei2022\??????????????????\data','D:\Doc\Huawei2022\??????????????????\data/solution.txt')
codecraft=CodeCraft('/data','/output/solution.txt')
codecraft.work()


