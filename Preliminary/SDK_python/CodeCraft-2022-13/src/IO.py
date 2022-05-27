import csv
import configparser
import os

class IO:
    def __init__(self,datapath,outpath):
        self.datapath=datapath
        self.outpath=outpath
        self.demandpath=self.datapath+'/demand.csv'
        self.qospath=self.datapath+'/qos.csv'
        self.sitepath=self.datapath+'/site_bandwidth.csv'
        self.configpath=self.datapath+'/config.ini'
        self.custom_id, self.demandInfo = self.demandReader()
        self.site_id, self.siteInfo, _ = self.siteReader()
        self.T = len(self.demandInfo)
        self.N = len(self.site_id)
        self.M = len(self.custom_id)
    def demandReader(self):
        demandInfo=[]
        f=csv.reader(open(self.demandpath,'r'))
        for idx,i in enumerate(f):
            if idx>0:
                i=list(map(lambda x:int(x),i[1:]))
                demandInfo.append(i)
            else:
                custom_id=i[1:]
        return custom_id,demandInfo

    def siteReader(self):
        site_id=[]
        siteInfo=[]
        siteId2idx={}
        f = csv.reader(open(self.sitepath, 'r'))
        for idx,i in enumerate(f):
            if idx>0:
                site_id.append(i[0])
                siteInfo.append(int(i[1]))
                siteId2idx[i[0]]=idx-1
        return site_id,siteInfo,siteId2idx

    def qosReader(self):
        qosInfo=[]
        f = csv.reader(open(self.qospath, 'r'))
        for idx,i in enumerate(f):
            if idx>0:
                qosInfo.append(list(map(lambda x:int(x),i[1:])))
        return qosInfo

    def configReader(self):
        config=configparser.ConfigParser()
        config.read(self.configpath)
        qos_constraint=int(config['config']['qos_constraint'])
        return qos_constraint

    def output(self,dp):
        if os.path.exists(self.outpath):
            os.remove(self.outpath)
        with open(self.outpath, "w") as f:
            for t in range(self.T):
                for cidx in range(self.M):
                    line=self.custom_id[cidx]+':'
                    for sidx in range(self.N):
                        if dp[t][cidx][sidx]!=0:
                            line+='<{},{}>,'.format(self.site_id[sidx],dp[t][cidx][sidx])
                    line=line[:-1]
                    f.write(line)
                    if t != self.T - 1 or cidx!=self.M-1:
                        f.write('\n')




