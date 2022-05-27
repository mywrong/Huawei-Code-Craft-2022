import heapq
import math
import time
from functools import reduce
from IO import IO
from copy import deepcopy

MAXINT=10000000
PARAM=[1,0.09375,1]#PARAM[2]=(37.5(err),43.75,50(ok))
#0：提交，1:测试不画图，2：测试画图
TEST=1

if TEST in[1,2]:
    # dataPath='D:\Doc\Huawei2022\线下调试数据\CodeCraft2022-PressureGenerator-master\pressure_data'
    # solutionPath='D:\Doc\Huawei2022\线下调试数据\CodeCraft2022-PressureGenerator-master\pressure_data/solution.txt'
    dataPath = 'D:\Doc\Huawei2022\线下调试数据\CodeCraft2022-PressureGenerator-master\simulated_data'
    solutionPath='D:\Doc\Huawei2022\线下调试数据\CodeCraft2022-PressureGenerator-master\simulated_data/solution.txt'
    from analyze import Analyze
    import matplotlib.pyplot as plt
    analyze=Analyze(dataPath,solutionPath)
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
        self.brotherSites=self.get_brotherSites()
        self.N5=int(self.T*0.05)
        self.N95=math.ceil(self.N*0.95)-1
        self.dp=[[[0 for _ in range(self.N)] for _ in range(self.M)]for _ in range(self.T)]
    def work(self):

        demandInfo = deepcopy(self.demandInfo)
        siteTimeInfo = [[self.siteInfo[i] for _ in range(self.T)] for i in range(self.N)]  # 每个边缘节点每个时刻剩余流量

        LargestFlows=[set() for _ in range(self.T)]
        dataFlows=list(reduce(lambda x,y:x+y,self.demandInfo))
        dataFlows.sort(reverse=True)
        for sidx in self.siteQ:
            """先取满足N5大的流量"""
            # 对每时刻需求求和,取前N5大
            timeFlows = [[] for _ in range(self.T)] #边缘节点每时刻需要满足的流量总和
            for t in range(self.T):
                sum = 0
                for cidx in self.custSets[sidx]:
                    sum += demandInfo[t][cidx]
                timeFlows[t] = [t, sum]
            N5Largest = heapq.nlargest(self.N5, timeFlows, key=lambda x: (x[1], timeFlows)) #取前5%大需求流量
            for t,_ in N5Largest:
                q=[cidx for cidx in self.custSets[sidx]]
                q.sort(key=lambda cidx:(-demandInfo[t][cidx],q))
                for cidx in q:
                    f=min(demandInfo[t][cidx],siteTimeInfo[sidx][t])
                    self.updateInfo(sidx,f,t,cidx,siteTimeInfo,demandInfo)
                    if siteTimeInfo[sidx][t]==0:
                        break
            """后取N95小"""
            N95Largest = 0
            for t in range(self.T):
                s = 0
                for cidx in self.custSets[sidx]:
                    if siteTimeInfo[sidx][t] > 0:
                        needflow = demandInfo[t][cidx]
                        n = len(self.siteSets[cidx])
                        f = needflow // n
                        f = min(f,siteTimeInfo[sidx][t])
                        s += f
                N95Largest = int(max(N95Largest,s)*PARAM[0]) #后N95%时刻可以尽量分配到N95Largest流量
            for t in range(self.T):
                canFlow = N95Largest
                for cidx in self.custSets[sidx]:
                    f = min(demandInfo[t][cidx],siteTimeInfo[sidx][t],canFlow)
                    self.updateInfo(sidx,f,t,cidx,siteTimeInfo,demandInfo)
                    canFlow -= f

        if TEST == 2:
            data = [[self.siteInfo[sidx] - siteTimeInfo[sidx][t] for t in range(self.T)] for sidx in range(self.N)]
            for i in range(self.N):
                data[i].sort()
            # analyze.drawN95Flows(data)
            analyze.drawSitTimeUsed(self.get_siteTimeUsed(),'D:\Doc\Huawei2022\数据分析\siteTimeInfo/12\一次分配')
            print('over1')
        """二次分配"""
        flowTarget = self.distribute2()
        # flowTarget = 3875
        self.dp=[[[0 for _ in range(self.N)] for _ in range(self.M)]for _ in range(self.T)]
        siteTimeInfo = [[self.siteInfo[i] for _ in range(self.T)] for i in range(self.N)]  # 每个边缘节点每个时刻剩余流量
        demandInfo = deepcopy(self.demandInfo)

        for sidx in self.siteQ:
            """先取满足N5大的流量"""
            # 对每时刻需求求和,取前N5大
            timeFlows = [[] for _ in range(self.T)] #边缘节点每时刻需要满足的流量总和
            for t in range(self.T):
                sum = 0
                for cidx in self.custSets[sidx]:
                    sum += demandInfo[t][cidx]
                timeFlows[t] = [t, sum]
            N5Largest = heapq.nlargest(self.N5, timeFlows, key=lambda x: (x[1], timeFlows)) #取前5%大需求流量
            for t,_ in N5Largest:
                q = [cidx for cidx in self.custSets[sidx]]
                q.sort(key=lambda cidx: (-demandInfo[t][cidx], q))
                for cidx in q:
                    needFlow=demandInfo[t][cidx]
                    f=min(needFlow,siteTimeInfo[sidx][t])
                    self.updateInfo(sidx,f,t,cidx,siteTimeInfo,demandInfo)
                    if siteTimeInfo[sidx][t]==0:
                        break

            """后取N95小"""
            N95Largest = flowTarget
            for t in range(self.T):
                canFlow = N95Largest
                for cidx in self.custSets[sidx]:
                    f = min(demandInfo[t][cidx],siteTimeInfo[sidx][t],canFlow)
                    self.updateInfo(sidx,f,t,cidx,siteTimeInfo,demandInfo)
                    canFlow -= f
        if TEST==2:
            data=[[self.siteInfo[sidx]-siteTimeInfo[sidx][t] for t in range(self.T) ] for sidx in range(self.N)]
            for i in range(self.N):
                data[i].sort()
            # analyze.drawN95Flows(data)
            analyze.drawSitTimeUsed(self.get_siteTimeUsed(), 'D:\Doc\Huawei2022\数据分析\siteTimeInfo/12\二次分配')
            print('over2')

        self.robustDistribute(siteTimeInfo,demandInfo)
        siteTimeInfo,demandInfo=self.distribute(100,siteTimeInfo)
        self.robustDistribute(siteTimeInfo, demandInfo)
        self.io.output(self.dp)

        if TEST == 2:
            data = [[self.siteInfo[sidx] - siteTimeInfo[sidx][t] for t in range(self.T)] for sidx in range(self.N)]
            for i in range(self.N):
                data[i].sort()
            # analyze.drawN95Flows(data)
            analyze.drawSitTimeUsed(self.get_siteTimeUsed(), 'D:\Doc\Huawei2022\数据分析\siteTimeInfo/12\最终')
            print('over3')

    def distribute(self,cutFlow,siteTimeInfo):
        siteTimeUsed = self.get_siteTimeUsed()

        brotherSites = self.brotherSites
        N95Flows = self.get_N95Flows()


        for sidx in self.siteQ:
            for t in range(self.T):
                flow = siteTimeUsed[sidx][t]
                if flow != N95Flows[sidx]:
                    continue

                brotherCanFlows = {}
                for brotherSidx in brotherSites[sidx]:
                    if siteTimeUsed[brotherSidx][t] > N95Flows[brotherSidx]:
                        canFlow = self.siteInfo[brotherSidx] - siteTimeUsed[brotherSidx][t]
                    else:
                        canFlow = N95Flows[brotherSidx] - siteTimeUsed[brotherSidx][t]
                    brotherCanFlows[brotherSidx] = min(canFlow,siteTimeInfo[sidx][t])
                brothers=[sidx for sidx in brotherSites[sidx]]
                brothers.sort(key=lambda x: (-brotherCanFlows[x], brothers))

                workTmp = []
                disFlow = cutFlow
                n = len(brotherSites[sidx])
                for brotherSidx in brothers:
                    f = min(disFlow//n, brotherCanFlows[brotherSidx])
                    disFlow -= f
                    workTmp.append([brotherSidx, siteTimeUsed[brotherSidx][t]])
                    siteTimeUsed[brotherSidx][t] +=f
                    n -= 1
                    if disFlow==0:
                        break
                if disFlow > 0:
                    for brotherSidx, f in workTmp:
                        siteTimeUsed[brotherSidx][t] = f
                    continue
                siteTimeUsed[sidx][t]-=cutFlow
            tmp=sorted(siteTimeUsed[sidx])
            N95Flows[sidx] = tmp[self.N95]

        flowTarget = self.distribute2()
        self.dp = [[[0 for _ in range(self.N)] for _ in range(self.M)] for _ in range(self.T)]
        siteTimeInfo = [[self.siteInfo[i] for _ in range(self.T)] for i in range(self.N)]  # 每个边缘节点每个时刻剩余流量
        demandInfo = deepcopy(self.demandInfo)

        for sidx in self.siteQ:
            """先取满足N5大的流量"""
            # 对每时刻需求求和,取前N5大
            timeFlows = [[] for _ in range(self.T)]  # 边缘节点每时刻需要满足的流量总和
            for t in range(self.T):
                sum = 0
                for cidx in self.custSets[sidx]:
                    sum += demandInfo[t][cidx]
                timeFlows[t] = [t, sum]
            N5Largest = heapq.nlargest(self.N5, timeFlows, key=lambda x: (x[1], timeFlows))  # 取前5%大需求流量
            for t, _ in N5Largest:
                q = [cidx for cidx in self.custSets[sidx]]
                q.sort(key=lambda cidx: (-demandInfo[t][cidx], q))
                for cidx in q:
                    needFlow = demandInfo[t][cidx]
                    f = min(needFlow, siteTimeInfo[sidx][t])
                    self.updateInfo(sidx, f, t, cidx, siteTimeInfo, demandInfo)
                    if siteTimeInfo[sidx][t] == 0:
                        break

            """后取N95小"""
            N95Largest = N95Flows[sidx]
            for t in range(self.T):
                canFlow = N95Largest
                for cidx in self.custSets[sidx]:
                    f = min(demandInfo[t][cidx], siteTimeInfo[sidx][t], canFlow)
                    self.updateInfo(sidx, f, t, cidx, siteTimeInfo, demandInfo)
                    canFlow -= f
        return siteTimeInfo,demandInfo

    def robustDistribute(self,siteTimeInfo,demandInfo):
        """最后鲁棒分配"""
        for t in range(self.T):
            for cidx in range(self.M):
                needFlow = demandInfo[t][cidx]
                if needFlow != 0:
                    q = self.siteSets[cidx]
                    qq = []
                    n = 0
                    for sidx in q:
                        if siteTimeInfo[sidx][t] > 0:
                            qq.append(sidx)
                            n += 1
                    for sidx in qq:
                        f = needFlow // n
                        f = min(f, siteTimeInfo[sidx][t])
                        self.updateInfo(sidx, f, t, cidx, siteTimeInfo, demandInfo)
                        needFlow = demandInfo[t][cidx]
                        if needFlow == 0:
                            break
                        n -= 1


    def updateInfo(self,sidx,flow,t,cidx,siteTimeInfo,demandInfo):
        self.dp[t][cidx][sidx]+=flow
        siteTimeInfo[sidx][t]-=flow
        demandInfo[t][cidx]-=flow
    def get_brotherSites(self):
        brotherSites = [set() for _ in range(self.N)]
        for sidx in range(self.N):
            custQ = self.custSets[sidx]
            for cidx in custQ:
                for ssidx in self.siteSets[cidx]:
                    brotherSites[sidx].add(ssidx)
            if len(brotherSites[sidx])>0:
                brotherSites[sidx].remove(sidx)
        return brotherSites

    def match(self):
        siteSets=[[] for _ in range(self.M)]   #每个客户节点可以访问的边缘节点集合
        custSets = [[] for _ in range(self.N)] #每个边缘节点可以提供服务的客户节点集合
        for i in range(self.N):
            for j in range(self.M):
                if self.qosInfo[i][j]<self.qos_constraint:
                    siteSets[j].append(i)
                    custSets[i].append(j)
        for i in range(self.N):
            custSets[i].sort(key=lambda x:(len(siteSets[x]),custSets[i]))
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

    def get_siteTimeUsed(self):
        siteTimeUsed = [[0 for _ in range(self.T)] for _ in range(self.N)]
        for sidx in range(self.N):
            for t in range(self.T):
                for cidx in range(self.M):
                    siteTimeUsed[sidx][t] += self.dp[t][cidx][sidx]
        return siteTimeUsed

    def get_N95Flows(self):
        """计算每个边缘节点N95处的流量大小"""
        siteTimeUsed = self.get_siteTimeUsed()
        N95Flows = {}
        for sidx in range(self.N):
            tmp = sorted(siteTimeUsed[sidx])
            N95Flows[sidx]= tmp[self.N95]
        return N95Flows

    def distribute2(self):
        N95Flows=self.get_N95Flows()
        l=[]
        for sidx,f in N95Flows.items():
           l.append([sidx,f])
        l.sort(key=lambda x: -x[1])
        Nlargest=int(PARAM[1]*self.N)-1 #best:0.09375
        flowTarget=l[Nlargest][1]
        return flowTarget









if TEST in [1,2]:
    codecraft=CodeCraft(dataPath,solutionPath)
else:
    codecraft=CodeCraft('/data','/output/solution.txt')

codecraft.work()


