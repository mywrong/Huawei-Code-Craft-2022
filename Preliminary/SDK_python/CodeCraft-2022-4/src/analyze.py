import math
import matplotlib.pyplot as plt
from IO import IO
import numpy as np
class Analyze:
    def __init__(self,datapath,outpath):
        self.io=IO(datapath,outpath)
        self.custom_id, self.demandInfo=self.io.demandReader()
        self.site_id,self.siteInfo,_=self.io.siteReader()
        self.qosInfo=self.io.qosReader()
        self.qos_constraint=self.io.configReader()
        self.T = len(self.demandInfo)
        self.N = len(self.site_id)
        self.M = len(self.custom_id)
    def num_95(self):
        data=[]
        for i in range(self.T):
            for j in range(self.M):
                data.append(self.demandInfo[i][j])
        data.sort()
        print(data[math.ceil(self.T*self.M*0.95)])
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


        x = 0.5 + np.arange(self.N)
        y=sorted(cnts,reverse=True)
        # plot
        fig, ax = plt.subplots()

        ax.bar(x, y, width=1, edgecolor="white", linewidth=0.7)

        plt.show()
if __name__=='__main__':
    analyze=Analyze('D:\Doc\Huawei2022\线下调试数据\data','D:\Doc\Huawei2022\线下调试数据\data/solution.txt')
    analyze.siteQueue()