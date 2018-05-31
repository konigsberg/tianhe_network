/*
 * globalParam.h
 *
 * Create on 2017-12-28 18:02:30
 * This is an automatically generated file.
 * Author: Zhao WANG, SJTU
 */

#ifndef GLOBALPARAM_H_
#define GLOBALPARAM_H_


#define V 6

#define H 4
#define W 6
#define P 24

#define FlitSize 32
#define PacketLength 5
#define PacketSize 160

#define InputBufferDepth 12
#define SwitchBufferDepth 5
#define ColumnBufferDepth 20
#define PipelineDepth 3
#define NodeBufferDepth 256

#define NodeNum 18432

#define TrafficPattern 1
#define UniformTraffic 1
#define TransposeTraffic 2
#define BitReversalTraffic 3
#define HotSpotTraffic 4
#define DebugPattern 5

#define Frequency 437500000.0
#define ClockCycle 2.28571428571e-09

#define Verbose 0
#define LOG 1
#define DETAIL 2

#define SimStartTime 1
#define RecordStartTime 1.000001

#define InjectionRate 1.0

#endif /* GLOBALPARAM_H_ */