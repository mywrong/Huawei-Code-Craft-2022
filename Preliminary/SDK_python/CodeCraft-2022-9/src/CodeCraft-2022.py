import heapq
import math
from IO import IO
from copy import deepcopy
MAXINT=10000000

#0：提交，1:测试不画图，2：测试画图
TEST=1
if TEST in[1,2]:
    from analyze import Analyze
    analyze=Analyze('D:\Doc\Huawei2022\线下调试数据\data','D:\Doc\Huawei2022\线下调试数据\data/solution.txt')

class CodeCraft:
    def __init__(self,datapath,outpath):
        self.io=IO(datapath,outpath)
        self.custom_id, self.demandInfo=self.io.demandReader()
        self.demandInfoCopy=deepcopy(self.demandInfo)
        self.site_id,self.siteInfo,_=self.io.siteReader()
        self.qosInfo = self.io.qosReader()
        self.qos_constraint = self.io.configReader()

        self.T = len(self.demandInfo)
        self.N = len(self.site_id)
        self.M = len(self.custom_id)
        self.siteUsedFreq=[0 for _ in range(self.N)]
        self.siteSets,self.custSets=self.match()
        self.siteQ=self.siteQueue()

        self.N5=int(self.T*0.05)
        self.N95=math.ceil(self.N*0.95)-1

    def work(self):
        out=[[[] for _  in range(self.M)] for _ in range(self.T)]
        siteTimeInfo=[[self.siteInfo[i] for _ in range(self.T)] for i in range(self.N)] #每个边缘节点每个时刻剩余流量
        sitTimeUsed=[[0 for _ in range(self.T)] for _ in range(self.N)] #每个边缘节点每个时刻消费的流量
        demandInfo=deepcopy(self.demandInfo)
        for sidx in self.siteQ:
            """先取前N5大"""
            #对每时刻需求求和,取前N5大
            flows=[[] for _ in range(self.T)]
            for t in range(self.T):
                sum=0
                for cidx in self.custSets[sidx]:
                    sum+=demandInfo[t][cidx]
                flows[t]=[t,sum]
            N5Largest=heapq.nlargest(self.N5,flows,key=lambda x:(x[1],flows))
            for t,flows in N5Largest:
                for cidx in self.custSets[sidx]:
                    needflow=demandInfo[t][cidx]
                    if needflow!=0 and siteTimeInfo[sidx][t]!=0:
                        f=needflow if siteTimeInfo[sidx][t]>=needflow else siteTimeInfo[sidx][t]
                        if f!=0:
                            siteTimeInfo[sidx][t]-=f
                            sitTimeUsed[sidx][t]+=f
                            demandInfo[t][cidx]-=f
                            self.insertOut(out,sidx,f,t,cidx)
                            if siteTimeInfo[sidx][t]==0:
                                break

            """后取N95小"""
            N95Largest =0
            for t in range(self.T):
                s=0
                for cidx in self.custSets[sidx]:
                    if siteTimeInfo[sidx][t] > 0:
                        needflow=demandInfo[t][cidx]
                        n=len(self.siteSets[cidx])
                        f=needflow//n
                        f=f if f<=siteTimeInfo[sidx][t] else siteTimeInfo[sidx][t]
                        s+=f
                N95Largest=N95Largest if N95Largest>s else s
            for t in range(self.T):
                canFlow=N95Largest
                for cidx in self.custSets[sidx]:
                    if siteTimeInfo[sidx][t] > 0:
                        f=demandInfo[t][cidx]
                        f= f if f<=siteTimeInfo[sidx][t] and f<=canFlow else min(siteTimeInfo[sidx][t],canFlow)
                        if f!=0:
                            siteTimeInfo[sidx][t] -= f
                            sitTimeUsed[sidx][t] += f
                            demandInfo[t][cidx] -= f
                            self.insertOut(out,sidx,f,t,cidx)
                            canFlow-=f
        flowTarget=self.distribute2(out,siteTimeInfo,sitTimeUsed)
        """二次分配"""
        out = [[[] for _ in range(self.M)] for _ in range(self.T)]
        siteTimeInfo = [[self.siteInfo[i] for _ in range(self.T)] for i in range(self.N)]  # 每个边缘节点每个时刻剩余流量
        sitTimeUsed = [[0 for _ in range(self.T)] for _ in range(self.N)]  # 每个边缘节点每个时刻消费的流量
        demandInfo=deepcopy(self.demandInfo)
        for sidx in self.siteQ:
            """先取前N5大"""
            # 对每时刻需求求和,取前N5大
            flows = [[] for _ in range(self.T)]
            for t in range(self.T):
                sum = 0
                for cidx in self.custSets[sidx]:
                    sum += demandInfo[t][cidx]
                flows[t] = [t, sum]
            N5Largest = heapq.nlargest(self.N5, flows, key=lambda x: (x[1], flows))
            for t, flows in N5Largest:
                for cidx in self.custSets[sidx]:
                    needflow = demandInfo[t][cidx]
                    if needflow != 0 and siteTimeInfo[sidx][t] != 0:
                        f = needflow if siteTimeInfo[sidx][t] >= needflow else siteTimeInfo[sidx][t]
                        if f != 0:
                            siteTimeInfo[sidx][t] -= f
                            sitTimeUsed[sidx][t] += f
                            demandInfo[t][cidx] -= f
                            self.insertOut(out, sidx, f, t, cidx)
                            if siteTimeInfo[sidx][t] == 0:
                                break

            """后取N95小"""
            N95Largest = flowTarget
            for t in range(self.T):
                canFlow = N95Largest
                for cidx in self.custSets[sidx]:
                    if siteTimeInfo[sidx][t] > 0:
                        f = demandInfo[t][cidx]
                        f = f if f <= siteTimeInfo[sidx][t] and f <= canFlow else min(siteTimeInfo[sidx][t], canFlow)
                        if f != 0:
                            siteTimeInfo[sidx][t] -= f
                            sitTimeUsed[sidx][t] += f
                            demandInfo[t][cidx] -= f
                            self.insertOut(out, sidx, f, t, cidx)
                            canFlow -= f



        """最后鲁棒分配"""
        for t in range(self.T):
            for cidx in range(self.M):
                needflow=demandInfo[t][cidx]
                if needflow!=0:
                    q = self.siteSets[cidx]
                    n=len(q)
                    for sidx in q:
                        if siteTimeInfo[sidx][t]>0:
                            f=needflow
                            f = f if f <= siteTimeInfo[sidx][t] else siteTimeInfo[sidx][t]
                            if f!=0:
                                self.insertOut(out,sidx,f,t,cidx)
                                needflow-=f
                                siteTimeInfo[sidx][t] -= f
                                sitTimeUsed[sidx][t] += f
                                demandInfo[t][cidx]-=f
                                needflow=demandInfo[t][cidx]
                                if needflow==0:
                                    break
                        n-=1

        if TEST==2:
            analyze.drawSitTimeUsed(sitTimeUsed)
        self.io.output(out)

    def insertOut(self,out,sidx,flow,t,cidx):
        i = 0
        while i < len(out[t][cidx]) and out[t][cidx][i][0] != sidx:
            i += 1
        if i < len(out[t][cidx]):
            out[t][cidx][i][1] += flow
        else:
            out[t][cidx].append([sidx,flow])

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

    def distribute2(self,out,siteTimeInfo,siteTimeUsed):
        siteFlows=[[]for _ in range(self.N)]
        for sidx in range(self.N):
            for t in range(self.T):
                siteFlows[sidx].append([t,siteTimeUsed[sidx][t]])
        N95Flows=[]
        for sidx in range(self.N):
            tmp=sorted(siteFlows[sidx],key=lambda x:(x[1],siteFlows[sidx]))
            N95Flows.append([sidx]+tmp[self.N95])
        N95Flows.sort(key=lambda x:-x[2])
        Nlargest=int(0.09375*self.N)-1 #beat:0.09375
        flowTarget=N95Flows[Nlargest][2]
        return flowTarget












if TEST in [1,2]:
    codecraft=CodeCraft('D:\Doc\Huawei2022\线下调试数据\pressure_data','D:\Doc\Huawei2022\线下调试数据\pressure_data/solution.txt')
else:
    codecraft=CodeCraft('/data','/output/solution.txt')

codecraft.work()


