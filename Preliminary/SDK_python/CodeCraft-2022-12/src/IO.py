import csv
import configparser
import os
from collections import defaultdict
class IO:
    def __init__(self,datapath,outpath):
        self.datapath=datapath
        self.outpath=outpath
        self.demandpath=self.datapath+'/demand.csv'
        self.qospath=self.datapath+'/qos.csv'
        self.sitepath=self.datapath+'/site_bandwidth.csv'
        self.configpath=self.datapath+'/config.ini'
        self.custom_id, self.site_id=[],[]
        self.T,self.M,self.N=0,0,0
    def demandReader(self):
        """
        :return:
        demandInfo:{'A':[],'B':[],'C':[]}
        T:时间长度
        M：客户节点数
        """
        demandInfo=defaultdict(list)
        demandInfoTmp=[]
        custom_id=[]
        f=csv.reader(open(self.demandpath,'r'))
        for idx,line in enumerate(f):
            if idx>0:
                demands=list(map(lambda x:int(x),line[1:]))
                demandInfoTmp.append(demands)
            else:
                custom_id = line[1:]
        T=len(demandInfoTmp)
        M=len(demandInfoTmp[0])
        for t in range(T):
            for m in range(M):
                cid=custom_id[m]
                demandInfo[cid].append(demandInfoTmp[t][m])
        self.T,self.M=T,M
        return demandInfo,T,M

    def siteReader(self):
        """
        :return:
        siteInfo:{'An':XXX,'Bn':XXX,'Cn':XXX}
        N：边缘节点数
        """
        site_id=[]
        siteInfoTmp=[]
        siteInfo={}
        f = csv.reader(open(self.sitepath, 'r'))
        for idx,line in enumerate(f):
            if idx>0:
                site_id.append(line[0])
                siteInfoTmp.append(int(line[1]))
        for i,id in enumerate(site_id):
            siteInfo[id]=siteInfoTmp[i]
        N=len(siteInfo)
        self.N=N
        return siteInfo,N

    def qosReader(self):
        """
        :return:
        qosInfo:{'A':{'An':XXX','Bn':XXX}}
        """
        qosInfo=defaultdict(dict)
        custom_id=[]
        f = csv.reader(open(self.qospath, 'r'))
        for idx,line in enumerate(f):
            if idx>0:
                site_id=line[0]
                tmp=line[1:]
                for i,qos in enumerate(tmp):
                    cid=custom_id[i]
                    qos=int(qos)
                    qosInfo[cid][site_id]=qos
            else:
                custom_id = line[1:]
        return qosInfo

    def configReader(self):
        config=configparser.ConfigParser()
        config.read(self.configpath)
        qos_constraint=int(config['config']['qos_constraint'])
        return qos_constraint

    def fileReader(self):
        demandInfoDict,T,M=self.demandReader()
        siteInfoDict,N=self.siteReader()
        qosInfoDict=self.qosReader()
        qos_constranint=self.configReader()
        custom_id=[id for id in demandInfoDict.keys()]
        site_id=[id for id in siteInfoDict.keys()]
        demandInfo=[[demandInfoDict[cid][t] for cid in custom_id] for t in range(T)]
        siteInfo=[siteInfoDict[sid] for sid in site_id]
        qosInfo=[[qosInfoDict[cid][sid] for cid in custom_id] for sid in site_id]
        self.site_id,self.custom_id=site_id,custom_id
        return custom_id,site_id,demandInfo,siteInfo,qosInfo,qos_constranint

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




