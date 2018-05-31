"""
    Script for generating NED file of TH-2 interconnect network, global parameters header
    for simulation configuration, and omnetpp.ini file for OMNeT++ simulation.
    The header file is generated under certain command line arguments.
    
    @author: ouakira
"""

import time
from optparse import OptionParser


def writeChannel(nedFile, channelName, delay, datarate):
    nedFile.write('\t\tchannel ' + channelName + ' extends ned.DatarateChannel\n\t\t{\n')
    nedFile.write('\t\t\tdelay = ' + str(delay) + 'ns;\n')
    nedFile.write('\t\t\tdatarate = ' + str(datarate) + 'Gbps;\n')
    nedFile.write('\t\t}\n\n')

def writeNode(nedFile, nodeName):
    '''
        Write customized compute node declaration.
    '''
    nedFile.write('simple ' + nodeName + \
                  '\n{\n\tgates:\n\t\tinout port;\n}\n\n')


def genThNed():
    '''
        Generate fat-tree NED file for interconnect network of Tianhe-2 System.
    '''
    
    nedFileName = 'THFatTree.ned' # e.g. fat_tree2p3t.ned
    nedFile = open(nedFileName, 'w')
    
    ##### Some author infomation #####
    nedFile.write('//\n// '+ nedFileName + '\n');
    nedFile.write('// @author: Zhao WANG, SJTU\n')
    timestamp = time.time()
    timestruct = time.localtime(timestamp)
    nedFile.write('// Create on ' + time.strftime('%Y-%m-%d %H:%M:%S', timestruct) + '\n')
    nedFile.write('// This is an automatically generated file.\n//\n\n');
    
    routerNed = open('routerNed.txt')
    lines = routerNed.readlines()
    for line in lines:
        nedFile.write(line)
    
    writeNode(nedFile, 'THNode')
    
    ##### writing connection declarations #####
    
    nedFile.write('network THFatTree\n{\n')
    channelC1Delay = 0 #5
    channelC2Delay = 0 #50
    channelC3Delay = 0 #100
    datarate = 112
    
    # writing channel declaration
    nedFile.write('\ttypes:\n')
    writeChannel(nedFile, 'C1', str(channelC1Delay), str(datarate))
    writeChannel(nedFile, 'C2', str(channelC2Delay), str(datarate))
    writeChannel(nedFile, 'C3', str(channelC3Delay), str(datarate))
    
    # writing submodules
    nedFile.write('\tsubmodules:\n')
    
    # nedFile.write('\t\tnrm[576]: NRM {\n\n\t\t}\n')
    nedFile.write('''\t\tnrm[576]: NRM {\n\t\t\tparameters:\n\t\t\t\tname = "NRM";\n\t\t}\n''')
    nedFile.write('''\t\tleaf[1728]: THNRC {\n\t\t\tparameters:\n\t\t\t\tname = "Leaf";\n\t\t}\n''')
    nedFile.write('''\t\troot[432]: Root {\n\t\t\tparameters:\n\t\t\t\tname = "Root";\n\t\t}\n''')
    
    nedFile.write('\t\tnode[18432]: THNode {\n\n\t\t}\n\n')
    
    # writing connections
    nedFile.write('\tconnections:\n')
    # lower ports of nrm connects to all nodes
    for i in range(0, 576): # i-th nrm
        for p in range(0, 32):
            nedFile.write('\t\tnode[' + str(32*i+p) + '].port <--> C1 <--> '\
                          'nrm[' + str(i) + '].port_' + str(p) + ';\n')

    # upper ports of nrm connects to lower ports of leaf
    for i in range(0, 576): #i-th nrm
        for p in range(32, 32+36):
            nedFile.write('\t\tnrm[' + str(i) + '].port_' + str(p) + \
                          ' <--> C2 <--> leaf[' + str(i/12*36+p-32) + \
                          '].port_' + str(i%12) + ';\n')
    
    # upper ports of leaf connects to lower ports of root
    # leaf[36k+i]'s port_p <-> root[12i+p-12]'s port_k
    for k in range(0, 48):
        for i in range(0, 36):
            for p in range(12, 24):
                nedFile.write('\t\tleaf[' + str(36*k+i) + '].port_' + str(p) + \
                    ' <--> C3 <--> ' + \
                    'root[' + str(12*i+p-12) + '].port_' + str(k) + ';\n')

    nedFile.write('}\n')
    nedFile.close()


def genHeader():
    VC = 6
    FlitSize = 32
    PacketLength = 5
    PacketSize = PacketLength * FlitSize

    Height = 4
    Width = 6
    PortNum = Height * Width

    InputBufferDepth = 12
    SwitchBufferDepth = 5
    ColumnBufferDepth = 20

    NodeBufferDepth = 256

    NodeNum = 18432
    TrafficPattern = 1

    Frequency = 437500000.0
    ClockCycle = 1.0 / Frequency
   
    routerLatency = 8e-9 #84e-9
    PipelineDepth = int(routerLatency * 1.0 / ClockCycle)

    Verbose = 0
    LOG = 1
    DETAIL = 2

    SimStartTime = 1

    RecordStartTime = 1.000001 # 2us. Make sure all flits are got at this time.
    InjectionRate = 1.0

    header = open('globalParam.h', 'w')

    header.write('/*\n * globalParam.h\n *\n')
    timestamp = time.time()
    timestruct = time.localtime(timestamp)
    header.write(' * Create on ' + time.strftime('%Y-%m-%d %H:%M:%S', timestruct) + '\n')
    header.write(' * This is an automatically generated file.\n');
    header.write(' * Author: Zhao WANG, SJTU\n')
    header.write(' */\n')
    
    header.write('\n#ifndef GLOBALPARAM_H_\n#define GLOBALPARAM_H_\n\n')
    header.write('\n#define V ' + str(VC))

    header.write('\n')

    header.write('\n#define H ' + str(Height))
    header.write('\n#define W ' + str(Width))
    header.write('\n#define P ' + str(PortNum))

    header.write('\n')

    header.write('\n#define FlitSize ' + str(FlitSize))
    header.write('\n#define PacketLength ' + str(PacketLength))
    header.write('\n#define PacketSize ' + str(PacketSize))

    header.write('\n')

    header.write('\n#define InputBufferDepth ' + str(InputBufferDepth))
    header.write('\n#define SwitchBufferDepth ' + str(SwitchBufferDepth))
    header.write('\n#define ColumnBufferDepth ' + str(ColumnBufferDepth))
    header.write('\n#define PipelineDepth ' + str(PipelineDepth))
    header.write('\n#define NodeBufferDepth ' + str(NodeBufferDepth))

    header.write('\n')

    header.write('\n#define NodeNum ' + str(NodeNum))

    header.write('\n')

    header.write('\n#define TrafficPattern ' + str(TrafficPattern))
    header.write('\n#define UniformTraffic 1')
    header.write('\n#define TransposeTraffic 2')
    header.write('\n#define BitReversalTraffic 3')
    header.write('\n#define HotSpotTraffic 4')
    header.write('\n#define DebugPattern 5')

    header.write('\n')

    header.write('\n#define Frequency ' + str(Frequency))
    header.write('\n#define ClockCycle ' + str(ClockCycle))

    header.write('\n')

    header.write('\n#define Verbose ' + str(Verbose))
    header.write('\n#define LOG ' + str(LOG))
    header.write('\n#define DETAIL ' + str(DETAIL))

    header.write('\n')

    header.write('\n#define SimStartTime ' + str(SimStartTime))
    header.write('\n#define RecordStartTime ' + str(RecordStartTime))

    header.write('\n')

    header.write('\n#define InjectionRate ' + str(InjectionRate))

    header.write('\n\n#endif /* GLOBALPARAM_H_ */')
    header.close()

def genIni():
    SimTimeLimit = 1.000004 # 4us
    ini = open('omnetpp.ini', 'w')
    ini.write('[General]\n\n')
    ini.write('[Config THFatTree]\n')
    ini.write('network = THFatTree\n')
    ini.write('sim-time-limit = ' + str(SimTimeLimit) + 's\n')
    ini.close()

def main():
    genThNed()
    genHeader()
    genIni()

if __name__ == '__main__':
    main()

