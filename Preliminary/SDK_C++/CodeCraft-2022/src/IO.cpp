#include "iostream"

using namespace std;

class IO {
public:
    string datapath;
    string outpath;
    string demandpath = datapath + "'/demand.csv'";
    string qospath = datapath + "/qos.csv";
    string sitepath = datapath + "/site_bandwidth.csv";
    string configpath = datapath + "/config.ini";


};