/*
 * Node.h
 *
 *  Created on: Nov 26, 2017
 *      Author: Konigsberg
 */

#ifndef NODE_H_
#define NODE_H_

#include <omnetpp.h>
#include <cstring>
#include <cassert>
#include <cstdio>
#include <cmath>
#include <deque>
#include <string>
#include <algorithm>
#include <cstdlib>

// network head file
#include "Flit_m.h"
#include "BufferFreeMsg_m.h"
#include "globalParam.h"

using namespace omnetpp;
using std::cout;
using std::endl;

class Node : public cSimpleModule
{
    public:
        Node();
        ~Node();

    protected:
        void initialize() override;
        void handleMessage(cMessage* msg) override;
        void finish() override;

        void handleGenTimer();
        void handleSendTimer();
        void handleBufferFreeMsg(cMessage* msg);
        void handleFlit(cMessage* msg);
        int bestVcid();
        Flit* genFlit(bool isHead, bool isTail, int vcid, int flitId, int destId);
        int genDestId();

        simtime_t channelAvailTime();

        virtual int nextPort() = 0;

    private:
        cMessage* genTimer;
        cMessage* sendTimer;

        std::deque<Flit*> sendQueue;

        long numFlitGen;
        long numFlitSent;
        long numPacketSent;

        long numPacketDropped;

        long numFlitGot;
        long numPacketGot;

        double totalLatency;
        double avgLatency;

        int credit[V];

        int radixBValue(std::string num, int base);
        std::string getBRadix(int value, int base, int digitNum);
};



Node::Node() {
    genTimer = nullptr;
    sendTimer = nullptr;
}


Node::~Node() {
    cancelAndDelete(genTimer);
    cancelAndDelete(sendTimer);
    while (!sendQueue.empty()) {
        delete sendQueue.front();
        sendQueue.pop_front();
    }
}


void Node::initialize() {

    numFlitGen = 0;
    numFlitSent = 0;
    numPacketSent = 0;

    numPacketDropped = 0;

    numFlitGot = 0;
    numPacketGot = 0;

    totalLatency = 0.0;
    avgLatency = 0.0;

    for (int v = 0; v < V; v++)
        credit[v] = InputBufferDepth;

    genTimer = new cMessage("genTimer");
    sendTimer = new cMessage("sendTimer");
    scheduleAt(SimStartTime, genTimer);
    scheduleAt(SimStartTime, sendTimer);
}


void Node::handleMessage(cMessage* msg) {
    if (msg == genTimer)
        handleGenTimer();
    else if (msg == sendTimer)
        handleSendTimer();
    else if (strcmp("bufferFreeMsg", msg->getName()) == 0)
        handleBufferFreeMsg(msg);
    else
        handleFlit(msg);
}


void Node::handleGenTimer() {
    //TODO Under routing debug mode, add this line before the following code:
    // if (getIndex() != 0 || numFlitGen >= 500) return;
    if (sendQueue.size() + FlitSize <= NodeBufferDepth) {
        int v = bestVcid();

        int destId = genDestId();
        for (int i = 0; i < PacketLength; i++) {
            Flit* flit = genFlit(i == 0, i == PacketLength-1, v, numFlitGen, destId);

            if (Verbose >= DETAIL)
                cout << "DETAIL: At time " << simTime() << " in node_"
                << getIndex() << ", flit " << flit->getName() << " was generated"
                << endl;

            sendQueue.push_back(flit);

            if (simTime() >= RecordStartTime)
                ++numFlitGen;
        }
    } else {
        if (Verbose >= DETAIL)
            cout << "DETAIL: At time " << simTime() << " in node_"
            << getIndex() << ", a flit generation failed because buffer is full"
            << endl;

        if (simTime() >= RecordStartTime)
            ++numPacketDropped;
    }

    double interval = (1.0 / InjectionRate) * ClockCycle * PacketLength;
    interval = std::max(interval, ClockCycle);
    scheduleAt(simTime() + interval, genTimer);
}


void Node::handleSendTimer() {
    // TODO Under routing debug mode, add this line before the following code:
    // if (getIndex() != 0) return;
    scheduleAt(std::max(simTime() + ClockCycle, channelAvailTime()), sendTimer);
    if (sendQueue.empty()) return;
    if (channelAvailTime() > simTime()) return;

    Flit* flit = sendQueue.front();
    int v = flit->getVcid();
    if (credit[v] == 0) {
        if (Verbose >= DETAIL)
            cout << "DETAIL-WARNING: Next InputBuffer FULL!" << endl;
        return;
    }

    int pi = nextPort();
    flit->setPort(pi);
    flit->setSendTime(simTime().dbl());
    send(flit, "port$o");

    if (simTime() >= RecordStartTime) {
        numFlitSent++;
        if (flit->getIsTail())
            numPacketSent++;
    }

    sendQueue.pop_front();

    if (Verbose >= LOG)
        cout << "LOG: At time " << simTime() << ", node_" << getIndex()
        << " sent flit " << flit->getName() << endl;

    --credit[v];
}


void Node::handleBufferFreeMsg(cMessage* msg) {

    if (Verbose >= LOG)
        cout << "LOG: At time " << simTime() << ", node_" << getIndex()
        << " received a BufferFreeMsg" << endl;

    BufferFreeMsg* bufferFreeMsg = check_and_cast<BufferFreeMsg*>(msg);
    int v = bufferFreeMsg->getVcid();
    ++credit[v];
    delete bufferFreeMsg;
}


void Node::handleFlit(cMessage* msg) {

    Flit* flit = check_and_cast<Flit*>(msg);
    int destId = flit->getDestId();

	if (destId != getIndex()) {
		cout << "Wrong routing! The dest node id of flit is " << destId 
			 << ", but it arrives at node " << getIndex() << "!" << endl; 
	    assert(false); // Right routing assertion.
	}

    if (Verbose >= LOG)
        cout << "LOG: At time " << simTime() << ", node_" << getIndex()
        << " got flit " << flit->getName() << endl;
    
    if (simTime() >= RecordStartTime) {
        ++numFlitGot;

        totalLatency += (simTime().dbl() - flit->getSendTime());

        if (flit->getIsTail())
            ++numPacketGot;
    }

    delete flit;
}


int Node::bestVcid() {
    int maxCredit = 0, maxVcid = 0;
    for (int v = 0; v < V; v++)
        if (credit[v] > maxCredit) {
            maxCredit = credit[v];
            maxVcid = v;
        }
    return maxVcid;
}


Flit* Node::genFlit(bool isHead, bool isTail, int vcid, int flitId, int destId) {

    char flitName[30];
    int srcId = getIndex();
    if (isHead)
        sprintf(flitName, "HF%d-%d-to-%d", srcId, flitId, destId);
    else if (isTail)
        sprintf(flitName, "TF%d-%d-to-%d", srcId, flitId, destId);
    else
        sprintf(flitName, "BF%d-%d-to-%d", srcId, flitId, destId);

    Flit* flit = new Flit(flitName);
    flit->setByteLength(FlitSize);
    flit->setFlitCount(PacketLength);

    flit->setSrcId(srcId);
    flit->setDestId(destId);
    flit->setIsHead(isHead);
    flit->setIsTail(isTail);

    flit->setVcid(vcid);
    flit->setPort(nextPort());
    flit->setHopCount(0);
    // sendTime field left for handleSendTimer.

    return flit;
}


int Node::genDestId() {
    int destId;

    if (TrafficPattern == UniformTraffic)
        //destId = intuniform(0, NodeNum-1);
		destId = rand() % NodeNum;
		//destId = rand() % (32*12);

    else if (TrafficPattern == TransposeTraffic) { 
        std::string srcIdStr = getBRadix(getIndex(), 2, 5);
        std::string destIdStr = srcIdStr.substr(3,2) + srcIdStr[2] + srcIdStr.substr(0,2);
        destId = radixBValue(destIdStr, 2);

    } else if (TrafficPattern == BitReversalTraffic) {
        std::string srcIdStr = getBRadix(getIndex(), 2, 5);
        reverse(srcIdStr.begin(), srcIdStr.end());
        std::string destIdStr = srcIdStr;
        destId = radixBValue(destIdStr, 2);

    } else if (TrafficPattern == HotSpotTraffic) {
        int rand = intuniform(0, 9);
        if (rand < 1)
            destId = 0;
        else
            destId = intuniform(0, NodeNum-1);
    } else if (TrafficPattern == DebugPattern) {
        // add debug traffic pattern code here
        destId = 10;
    
    } else {
        printf("Invalid traffic pattern configuration! Simulation aborted.\n");
        assert(false);
        destId = 0;
    }

    if (destId < 0 || destId > NodeNum-1) {
        //printf("Invalid destination ID: %d! Set to 23.\n", destId);
		//assert(false);
        destId = intuniform(0, NodeNum-1);
    }
    return destId;
}


simtime_t Node::channelAvailTime() {
    cChannel* channel = gate("port$o")->getTransmissionChannel();
    simtime_t finishTime = channel->getTransmissionFinishTime();
    return finishTime;
}


int Node::radixBValue(std::string num, int base) {
    int value = 0;
    reverse(num.begin(), num.end());
    int index = 0;
    for (char ch : num) {
        value += (ch-'0') * pow(base, index);
        index++;
    }
    return value;
 }

std::string Node::getBRadix(int value, int base, int digitNum) {
    std::string num = std::string("");
    while (value > 0) {
        num.push_back('0' + value % base);
        value /= base;
    }
    while (int(num.size()) < digitNum)
        num.push_back('0');
    reverse(num.begin(), num.end());
    return num;
}


void Node::finish() {
    long numCycle = (simTime().dbl() - RecordStartTime) / ClockCycle;

    if (numFlitGot != 0)
        avgLatency = totalLatency * 1.0 / numFlitGot;

    if (getIndex() == 0)
        recordScalar("numCycle", numCycle);

    recordScalar("numFlitGen", numFlitGen);
    recordScalar("numFlitSent", numFlitSent);
    recordScalar("numPacketSent", numPacketSent);
    recordScalar("numPacketDropped", numPacketDropped);
    recordScalar("numFlitGot", numFlitGot);
    recordScalar("numPacketGot", numPacketGot);
    recordScalar("avgLatency", avgLatency);
}

#endif /* NODE_H_ */
