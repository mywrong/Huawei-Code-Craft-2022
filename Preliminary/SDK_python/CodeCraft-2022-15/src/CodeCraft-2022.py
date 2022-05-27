import heapq
import math
import time
from functools import reduce

from IO import IO
from copy import deepcopy

MAXINT = 10000000
PARAM = [1, 0.09375, 1]  # PARAM[2]=(37.5(err),43.75,50(ok))


# 0：提交，1:测试不画图，2：测试画图


class CodeCraft:
    def __init__(self, datapath, outpath):
        self.io = IO(datapath, outpath)
        self.custom_id, self.site_id, self.demandInfo, self.siteInfo, self.qosInfo, self.qos_constraint = self.io.fileReader()
        self.T = len(self.demandInfo)
        self.N = len(self.site_id)
        self.M = len(self.custom_id)

        self.siteSets, self.custSets = self.match()
        self.siteQ = self.siteQueue()

        self.N5 = int(self.T * 0.05)
        self.N95 = math.ceil(self.T * 0.95) - 1
        self.dp = [[[0 for _ in range(self.N)] for _ in range(self.M)] for _ in range(self.T)]

    def work(self):
        startTime=time.time()
        demandInfo = deepcopy(self.demandInfo)
        siteTimeInfo = [[self.siteInfo[i] for _ in range(self.T)] for i in range(self.N)]  # 每个边缘节点每个时刻剩余流量
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
                    f = min(demandInfo[t][cidx], siteTimeInfo[sidx][t])
                    self.updateInfo(sidx, f, t, cidx, siteTimeInfo, demandInfo)
                    if siteTimeInfo[sidx][t] == 0:
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
                        f = min(f, siteTimeInfo[sidx][t])
                        s += f
                N95Largest = int(max(N95Largest, s) * PARAM[0])  # 后N95%时刻可以尽量分配到N95Largest流量
            for t in range(self.T):
                canFlow = N95Largest
                for cidx in self.custSets[sidx]:
                    f = min(demandInfo[t][cidx], siteTimeInfo[sidx][t], canFlow)
                    self.updateInfo(sidx, f, t, cidx, siteTimeInfo, demandInfo)
                    canFlow -= f
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
                    if needFlow > 0:
                        for sidx in qq:
                            f = needFlow
                            f = min(f, siteTimeInfo[sidx][t])
                            self.updateInfo(sidx, f, t, cidx, siteTimeInfo, demandInfo)
                            needFlow = demandInfo[t][cidx]
                            if needFlow == 0:
                                break
        """
        N95Flows:每个边缘节点的95分位值，{sidx:xxx}
        siteTimeUsed：每个边缘节点每个时刻已分配的流量，[[for t in range(T)] for sidx in range(N)]
        siteTimeInfo:每个边缘节点每个时刻未分配
        self.dp：初解
        """
        N95Flows = self.get_N95Flows()
        siteTimeUsed,siteTimeInfo = self.get_siteTimeUsedAndInfo()
        siteQ=[i for i in range(self.N)]
        siteQ.sort(key=lambda x:(-N95Flows[x],siteQ))
        NN=min(50,self.N)
        for i in range(NN):
            sidx=siteQ[i]

            custQ = self.custSets[sidx]
            for t in range(self.T):
                if siteTimeUsed[sidx][t] > N95Flows[sidx]:
                    continue
                for cidx in custQ:
                    brotherSidxs = self.siteSets[cidx]

                    for brotherSidx in brotherSidxs:
                        if brotherSidx != sidx:

                            if siteTimeUsed[brotherSidx][t] <= N95Flows[brotherSidx]:
                                canFlow = N95Flows[brotherSidx] - siteTimeUsed[brotherSidx][t]
                            else:
                                canFlow = self.siteInfo[brotherSidx] - siteTimeUsed[brotherSidx][t]

                            canFlow=min(self.dp[t][cidx][sidx],canFlow)

                            self.dp[t][cidx][sidx]-=canFlow
                            siteTimeInfo[sidx][t]+=canFlow
                            siteTimeUsed[sidx][t]-=canFlow

                            self.dp[t][cidx][brotherSidx] += canFlow
                            siteTimeUsed[brotherSidx][t] += canFlow
                            siteTimeInfo[brotherSidx][t] -= canFlow


            tmp = sorted(siteTimeUsed[sidx])
            N95Flows[sidx] = tmp[self.N95]


        self.io.output(self.dp)

        if TEST == 2:
            data = [[self.siteInfo[sidx] - siteTimeInfo[sidx][t] for t in range(self.T)] for sidx in range(self.N)]
            for i in range(self.N):
                data[i].sort()
            # analyze.drawN95Flows(data)
            analyze.drawSitTimeUsed(self.get_siteTimeUsed(), 'D:\Doc\Huawei2022\数据分析\siteTimeInfo/12\最终')
            print('over3')

    def updateInfo(self, sidx, flow, t, cidx, siteTimeInfo, demandInfo):
        self.dp[t][cidx][sidx] += flow
        siteTimeInfo[sidx][t] -= flow
        demandInfo[t][cidx] -= flow

    def match(self):
        siteSets = [[] for _ in range(self.M)]  # 每个客户节点可以访问的边缘节点集合
        custSets = [[] for _ in range(self.N)]  # 每个边缘节点可以提供服务的客户节点集合
        for i in range(self.N):
            for j in range(self.M):
                if self.qosInfo[i][j] < self.qos_constraint:
                    siteSets[j].append(i)
                    custSets[i].append(j)
        for i in range(self.N):
            custSets[i].sort(key=lambda x: (len(siteSets[x]), custSets[i]))
        return siteSets, custSets

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
        return siteQ

    def get_siteTimeUsedAndInfo(self):
        siteTimeUsed = [[0 for _ in range(self.T)] for _ in range(self.N)]
        siteTimeInfo = [[self.siteInfo[i] for _ in range(self.T)] for i in range(self.N)]
        for sidx in range(self.N):
            for t in range(self.T):
                for cidx in range(self.M):
                    siteTimeUsed[sidx][t] += self.dp[t][cidx][sidx]
                    siteTimeInfo[sidx][t]-=self.dp[t][cidx][sidx]
        return siteTimeUsed,siteTimeInfo

    def get_N95Flows(self):
        """计算每个边缘节点N95处的流量大小"""
        siteTimeUsed,_ = self.get_siteTimeUsedAndInfo()
        siteFlows = [[] for _ in range(self.N)]
        for sidx in range(self.N):
            for t in range(self.T):
                siteFlows[sidx].append(siteTimeUsed[sidx][t])
        N95Flows = {}
        for sidx in range(self.N):
            tmp = sorted(siteFlows[sidx])
            N95Flows[sidx] = tmp[self.N95]
        return N95Flows

    def distribute2(self):
        N95Flows = self.get_N95Flows()
        N95Flows.sort(key=lambda x: -x[2])
        Nlargest = int(PARAM[1] * self.N) - 1  # best:0.09375
        flowTarget = N95Flows[Nlargest][2]
        return flowTarget


TEST = 1
if TEST in [1, 2]:
    from judge import Judge
    if TEST==2:
        from analyze import Analyze

    path = [
        {
            'dataPath': 'D:\Doc\Huawei2022\线下调试数据\CodeCraft2022-PressureGenerator-master\pressure_data',
            'solutionPath': 'D:\Doc\Huawei2022\线下调试数据\CodeCraft2022-PressureGenerator-master\pressure_data/solution.txt'
        },
        {
            'dataPath': 'D:\Doc\Huawei2022\线下调试数据\data1',
            'solutionPath': 'D:\Doc\Huawei2022\线下调试数据\data1/solution.txt'
        },
        {
            'dataPath': 'D:\Doc\Huawei2022\线下调试数据\data2',
            'solutionPath': 'D:\Doc\Huawei2022\线下调试数据\data2/solution.txt'
        },
        {
            'dataPath': 'D:\Doc\Huawei2022\线下调试数据\data3',
            'solutionPath': 'D:\Doc\Huawei2022\线下调试数据\data3/solution.txt'
        },
        {
            'dataPath': 'D:\Doc\Huawei2022\线下调试数据\data4',
            'solutionPath': 'D:\Doc\Huawei2022\线下调试数据\data4/solution.txt'
        },
        {
            'dataPath': 'D:\Doc\Huawei2022\线下调试数据\data5',
            'solutionPath': 'D:\Doc\Huawei2022\线下调试数据\data5/solution.txt'
        },
    ]

    for i in range(1, 6):
        dic = path[i]
        if TEST == 2:
            analyze = Analyze(dic['dataPath'], dic['solutionPath'])
        s = time.time()
        codecraft = CodeCraft(dic['dataPath'], dic['solutionPath'])
        codecraft.work()
        print('数据集：{},用时{}s'.format(dic['dataPath'], time.time() - s))
        judge = Judge(dic['dataPath'], dic['solutionPath'])
        judge.judge()
        judge.get_score()



else:
    codecraft = CodeCraft('/data', '/output/solution.txt')
    codecraft.work()
