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
        self.custom_id, self.site_id,self.stream_id,self.times=[],[],[],[]
        self.T,self.M,self.N=0,0,0
    def demandReader(self):
        """
        :return:
        demandInfo:{'A':{'time:'{'An':xx}}]}
        T:时间长度
        M：客户节点数
        """
        demandInfo=defaultdict(defaultdict)
        custom_id=[]
        f=csv.reader(open(self.demandpath,'r'))
        times=[]
        stream_id=defaultdict(set)
        for idx,line in enumerate(f):
            if idx>0:
                time=line[0]
                if len(times)==0 or times[-1]!=time:
                    times.append(time)
                streamId=line[1]
                stream_id[time].add(streamId)
                demands=list(map(lambda x:int(x),line[2:]))
                for i,need in enumerate(demands):
                    cid=custom_id[i]
                    if time not in demandInfo[cid].keys():
                        demandInfo[cid][time]={}
                    demandInfo[cid][time][streamId]=need
            else:
                custom_id = line[2:]
        T=len(times)
        M=len(custom_id)
        self.T,self.M=T,M
        self.stream_id=stream_id
        self.times=times
        self.custom_id=custom_id
        return demandInfo,stream_id,custom_id,times,T,M

    def siteReader(self):
        """
        :return:
        siteInfo:{'An':XXX,'Bn':XXX,'Cn':XXX}
        N：边缘节点数
        """
        site_id=[]
        siteInfo={}
        f = csv.reader(open(self.sitepath, 'r'))
        for idx,line in enumerate(f):
            if idx>0:
                sid=line[0]
                site_id.append(sid)
                siteInfo[sid]=int(line[1])

        N=len(siteInfo)
        self.N=N
        return siteInfo,site_id,N

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
        base_cost=int(config['config']['base_cost'])
        return qos_constraint,base_cost

    def fileReader(self):
        demandInfo,stream_id,custom_id,times,T,M=self.demandReader()
        siteInfo,site_id,N=self.siteReader()
        qosInfoDict=self.qosReader()
        qos_constraint,base_cost=self.configReader()

        return demandInfo,stream_id,custom_id,times,T,M,siteInfo,site_id,N,qosInfoDict,qos_constraint,base_cost

    # def output(self,dp):
    #     if os.path.exists(self.outpath):
    #         os.remove(self.outpath)
    #     with open(self.outpath, "w") as f:
    #         for t in range(self.T):
    #             for cidx in range(self.M):
    #                 line=self.custom_id[cidx]+':'
    #                 for sidx in range(self.N):
    #                     if dp[t][cidx][sidx]!=0:
    #                         line+='<{},{}>,'.format(self.site_id[sidx],dp[t][cidx][sidx])
    #                 line=line[:-1]
    #                 f.write(line)
    #                 if t != self.T - 1 or cidx!=self.M-1:
    #                     f.write('\n')




