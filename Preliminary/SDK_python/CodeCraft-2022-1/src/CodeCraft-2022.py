from IO import IO
class CodeCraft:
    def __init__(self,datapath,outpath):
        self.io=IO(datapath,outpath)
        self.custom_id, self.demandInfo=self.io.demandReader()
        self.site_id,self.siteInfo,_=self.io.siteReader()
        self.qosInfo=self.io.qosReader()
        self.qos_constraint=self.io.configReader()

    def work(self):
        out=[]
        for demand in self.demandInfo:
            flowlist=[]
            for cidx,needflow in enumerate(demand):
                outline = [self.custom_id[cidx]]
                for sidx,flow in enumerate(self.siteInfo):
                    qos=self.qosInfo[sidx][cidx]
                    if needflow>0 and qos<self.qos_constraint and flow>0:
                        duple=[(self.site_id[sidx],flow if needflow>flow else needflow)]
                        outline+=duple
                        flowlist.append((sidx,duple[0][1]))
                        self.siteInfo[sidx]-=duple[0][1]
                        needflow-=duple[0][1]
                        if needflow==0:
                            break
                out.append(outline)
            for duple in flowlist:
                self.siteInfo[duple[0]]+=duple[1]
        self.io.output(out)



codecraft=CodeCraft('D:\Doc\Huawei2022\线下调试数据\data','D:\Doc\Huawei2022\线下调试数据\data/solution.txt')
# codecraft=CodeCraft('/data','/output/solution.txt')
codecraft.work()


