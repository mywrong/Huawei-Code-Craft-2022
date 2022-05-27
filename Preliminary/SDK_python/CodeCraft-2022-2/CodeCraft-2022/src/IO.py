import csv
import configparser
import os

import numpy as np
class IO:
    def __init__(self,datapath,outpath):
        self.datapath=datapath
        self.outpath=outpath
        self.demandpath=self.datapath+'/demand.csv'
        self.qospath=self.datapath+'/qos.csv'
        self.sitepath=self.datapath+'/site_bandwidth.csv'
        self.configpath=self.datapath+'/config.ini'
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

    def output(self,out):
        if os.path.exists(self.outpath):
            os.remove(self.outpath)
        with open(self.outpath, "w") as f:
            for idx,outline in enumerate(out):
                line = outline[0] + ':'
                for i in range(1,len(outline)):
                    line+='<{},{}>'.format(outline[i][0],outline[i][1])
                    if i!=len(outline)-1:
                        line+=','
                f.write(line)
                if idx!=len(out)-1:
                    f.write('\n')



