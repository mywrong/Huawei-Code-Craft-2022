import math
import re
import matplotlib.pyplot as plt
import numpy as np
import matplotlib.colors as mcolors
from IO import IO
from collections import defaultdict

class Judge:
    def __init__(self,datapath,outpath):
        self.io=IO(datapath,outpath)
        self.outpath=outpath
        self.demandInfo,self.stream_id,self.custom_id,self.times,self.T,self.M,self.siteInfo,self.site_id,self.N,self.qosInfoDict,self.qos_constraint,self.base_cost=self.io.fileReader()
        self.ans=self.SolutonReader()
        self.siteTimeUsed=self.get_siteTimeUsed()
        self.custSets=self.get_custSets()
    def SolutonReader(self):

        with open(self.outpath, "r") as f:
            ans={}

            for t,line in enumerate(f.readlines()):
                time=self.times[t//self.M]
                if time not in ans.keys():
                    ans[time]={}
                line=line.split(':')
                cid=line[0]
                ans_c={}
                if line[1]!='':
                    S=re.findall("<[\w,]*>",line[1])
                    for s in S:
                        ans_p=[]
                        st=re.split("[:<,>\'\n\']",s)
                        sid=st[1]
                        for i in range(2,len(st)-1):
                            pid=st[i]
                            ans_p.append(pid)

                        ans_c[sid]=ans_p
                ans[time][cid]=ans_c

        return ans

    def get_siteTimeUsed(self):
        ans=self.ans
        siteTimeUsed= {}
        for sid in self.site_id:
            siteTimeUsed[sid]=[0 for _ in range(self.T)]
        for sid in self.site_id:
            for t,time in enumerate(self.times):
                for cid in self.custom_id:
                    if sid in ans[time][cid].keys():
                        plist=ans[time][cid][sid]
                        for p in plist:
                            need=self.demandInfo[cid][time][p]
                            siteTimeUsed[sid][t]+=need
        return siteTimeUsed

    def get_custSets(self):
        custSets=defaultdict(list)
        for sid in self.site_id:
            for cid in self.custom_id:
                if self.qosInfoDict[cid][sid]<self.qos_constraint:
                    custSets[sid].append(cid)
        return custSets
    def draw_eachTime_Used(self,path):
        # plt.ion()
        T=np.arange(self.T)
        colors = list(mcolors.TABLEAU_COLORS.keys())
        for sidx,sid in enumerate(self.site_id):
            plt.clf()
            plt.plot(T,self.siteTimeUsed[sid],color=mcolors.TABLEAU_COLORS[colors[len(self.custSets[sid])]],label="{}".format(len(self.custSets[sid])))
            site_band=self.siteInfo[sid]
            plt.plot(T,[site_band for _ in range(self.T)])
            used=sorted(self.siteTimeUsed[sid])
            plt.plot(T, [used[math.ceil(self.T*0.95)-1] for _ in range(self.T)])
            # plt.ylim(0,self.siteInfo[sid])
            plt.legend()
            plt.savefig(path+"/{}_{}.jpg".format(len(self.custSets[sid]),sid))
            plt.pause(0.01)
        # plt.show()


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
    dataPath="D:\Doc\Huawei2022/rematch\data"
    solutionPath="D:\Doc\Huawei2022/rematch\data/solution.txt"

    # judge = Judge(dataPath, dataPath+"/first.txt")
    # judge.draw_eachTime_Used("D:\Doc\Huawei2022/rematch/analyze\eachTime_Used/3.5\初次分配")
    judge = Judge(dataPath, dataPath+"/second.txt")
    judge.draw_eachTime_Used("D:\Doc\Huawei2022/rematch/analyze\eachTime_Used/3.5/鲁棒分配")
    # judge = Judge(dataPath, solutionPath)
    # judge.draw_eachTime_Used("D:\Doc\Huawei2022/rematch/analyze\eachTime_Used/3.5\最终")