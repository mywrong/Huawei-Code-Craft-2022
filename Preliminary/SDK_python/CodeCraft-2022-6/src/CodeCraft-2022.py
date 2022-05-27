import heapq
import math
from collections import defaultdict
from IO import IO
from utils import MyHeap

MAXINT=10000000
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
        self.freqThres=math.ceil(self.T*0.05)-1
        self.N95=self.num_95()
        self.N5=int(self.T*0.05)
    def work(self):
        out=[[[] for _  in range(self.M)] for _ in range(self.T)]
        siteTimeInfo=[[self.siteInfo[i] for _ in range(self.T)] for i in range(self.N)]
        sitTimeUsed=[[0 for _ in range(self.T)] for _ in range(self.N)]
        for sidx in self.siteQ:
            """先取前N5大"""
            #对每时刻需求求和,取前N5大
            flows=[[] for _ in range(self.T)]
            for t in range(self.T):
                sum=0
                for cidx in self.custSets[sidx]:
                    sum+=self.demandInfo[t][cidx]
                flows[t]=[t,sum]
            N5Largest=heapq.nlargest(self.N5,flows,key=lambda x:(x[1],flows))
            for t,flows in N5Largest:
                for cidx in self.custSets[sidx]:
                    needflow=self.demandInfo[t][cidx]
                    if needflow!=0:
                        f=needflow if siteTimeInfo[sidx][t]>=needflow else siteTimeInfo[sidx][t]
                        siteTimeInfo[sidx][t]-=f
                        sitTimeUsed[sidx][t]+=f
                        self.demandInfo[t][cidx]-=f
                        out[t][cidx].append([sidx,f])
                        if siteTimeInfo[sidx][t]==0:
                            break
            """后取N95小"""
            for t in range(self.T):
                    for cidx in self.custSets[sidx]:
                        if siteTimeInfo[sidx][t] > 0:
                            needflow=self.demandInfo[t][cidx]
                            n=len(self.siteSets[cidx])
                            f=needflow//n
                            f=f if f<=siteTimeInfo[sidx][t] else siteTimeInfo[sidx][t]
                            siteTimeInfo[sidx][t] -= f
                            sitTimeUsed[sidx][t] += f
                            self.demandInfo[t][cidx] -= f
                            out[t][cidx].append([sidx, f])

        for t in range(self.T):
            for cidx in range(self.M):
                needflow=self.demandInfo[t][cidx]
                if needflow!=0:
                    q = self.siteSets[cidx]
                    n=len(q)
                    for sidx in q:
                        if siteTimeInfo[sidx][t]>0:
                            f=needflow
                            f = f if f <= siteTimeInfo[sidx][t] else siteTimeInfo[sidx][t]
                            i=0
                            while i<len(out[t][cidx]) and out[t][cidx][i][0]!=sidx:
                                i+=1
                            if i<len(out[t][cidx]):
                                out[t][cidx][i][1]+=f
                            else:
                                out[t][cidx].append([sidx,f])
                            needflow-=f
                            siteTimeInfo[sidx][t] -= f
                            sitTimeUsed[sidx][t] += f
                            self.demandInfo[t][cidx]-=f
                            needflow=self.demandInfo[t][cidx]
                            if needflow==0:
                                break
                        n-=1
        # for t in range(self.T):
        #     for cidx in range(self.M):
        #         needflow=self.demandInfo[t][cidx]
        #         if needflow!=0:
        #             q = self.siteSets[cidx]
        #             n=len(q)
        #             for sidx in q:
        #                 if siteTimeInfo[sidx][t]>0:
        #                     f=needflow//n
        #                     f = f if f <= siteTimeInfo[sidx][t] else siteTimeInfo[sidx][t]
        #                     i=0
        #                     while i<len(out[t][cidx]) and out[t][cidx][i][0]!=sidx:
        #                         i+=1
        #                     if i<len(out[t][cidx]):
        #                         out[t][cidx][i][1]+=f
        #                     else:
        #                         out[t][cidx].append([sidx,f])
        #                     needflow-=f
        #                     siteTimeInfo[sidx][t] -= f
        #                     sitTimeUsed[sidx][t] += f
        #                     self.demandInfo[t][cidx]-=f
        #                     needflow=self.demandInfo[t][cidx]
        #                 n-=1


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
        siteQ=sorted(siteQ,key=lambda x:(cnts[x],siteQ))
        return siteQ








# codecraft=CodeCraft('D:\Doc\Huawei2022\线下调试数据\data','D:\Doc\Huawei2022\线下调试数据\data/solution.txt')
codecraft=CodeCraft('/data','/output/solution.txt')
codecraft.work()


