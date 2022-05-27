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
        self.siteQ=self.siteQueue()
        self.freqThres=math.ceil(self.T*0.04)
        self.N95=self.num_95()
    def work(self):
        out=[]
        for row,demand in enumerate(self.demandInfo):
            flowlist=[]
            for cidx,needflow in enumerate(demand):
                outline = [self.custom_id[cidx]]
                q=self.siteSets[cidx]
                n=len(q)
                for i in range(len(q)):
                    sidx = q[i]
                    flow=self.siteInfo[sidx]
                    duple = [(self.site_id[sidx], flow if flow<needflow//n else needflow//n)]
                    outline += duple
                    flowlist.append((sidx, duple[0][1]))
                    self.siteInfo[sidx] -= duple[0][1]
                    n-=1
                    needflow-=duple[0][1]

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
        siteSets=[[] for _ in range(self.M)]   #每个客户节点可以访问的边缘节点集合
        custSets = [[] for _ in range(self.N)] #每个边缘节点可以提供服务的客户节点集合
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

        return siteQ





codecraft=CodeCraft('D:\Doc\Huawei2022\线下调试数据\data1','D:\Doc\Huawei2022\线下调试数据\data1/solution.txt')
# codecraft=CodeCraft('/data','/output/solution.txt')
codecraft.work()


