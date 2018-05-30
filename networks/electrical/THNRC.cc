/*
 * THNRC.cc
 *
 *  Created on: Dec 24, 2017
 *      Author: Konigsberg
 */

#include "NRC.h"
using std::string;
using std::cout;
using std::endl;
using std::to_string;

class THNRC : public NRC
{
    protected:
        int routingPort(Flit* flit) override;
        int nextPort(int port) override;
        bool connectToNode(int port) override;
};

Define_Module(THNRC);

int THNRC::routingPort(Flit* flit) {
    // in router, calculate which output port of current router
    // flit should go to. the port is calculated according to
    // destination processor ID and network topology
	int inPort = flit->getPort(), outPort;

	int currLevel;
	string name = par("name").stdstringValue();

	if (name != "") {
		assert(name == "Leaf");
		currLevel = 3;
	}
	else {
		assert(getParentModule() != nullptr);
		name = getParentModule()->par("name").stdstringValue();
		if (name == "NRM") {
			if (getIndex() < 3)
				currLevel = 1;
			else 
				currLevel = 2;
		} else {
			assert(name == "Root");
			if (getIndex() < 4)
				currLevel = 4;
			else 
				currLevel = 5;
		}
	}
	
	int srcId = flit->getSrcId();
    int destId = flit->getDestId();

	int maxId = std::max(srcId, destId);
	int minId = std::min(srcId, destId);
	int ancestorLevel;

	if (maxId - (minId / 32) * 32 < 32) { 
		// src and dest nodes are in the same NRM
		maxId %= 32;
		minId %= 32;
		if (maxId - (minId / 12) * 12 < 12)
			ancestorLevel = 1;
		else
			ancestorLevel = 2;
	} else if (maxId - (minId / (32*12)) * 32*12 < 32*12) 
		// src and dest nodes are in the same group
		ancestorLevel = 3;
	else if (maxId - (minId / (32*12*12)) * 32*12*12 < 32*12*12)
		// src and dest nodes are in the same 4-group
		ancestorLevel = 4;
	else
		ancestorLevel = 5;


	if (currLevel < ancestorLevel && inPort < 12)
		// forward up
		outPort = rand() % 12 + 12;
	else {
		// forward down
		if (currLevel == 1)
			outPort = (destId - (destId / 32) * 32) % 12;
		else if (currLevel == 2) {
			// k is the dest index in one NRM
			int k = destId - (destId / 32) * 32;
			if (k < 12)
				outPort = rand() % 4;
			else if (k < 24)
				outPort = rand() % 4 + 4;
			else
				outPort = rand() % 4 + 8;
		} else if (currLevel == 3)
			outPort = (destId % (32*12)) / 32;
		else if (currLevel == 4)
			//outPort = (destId / (32*12)) % 12 - 12 * getIndex();
			outPort = (destId / (32*12)) % 12;
		else if (currLevel == 5)
			//outPort = rand() % 6 + ((destId / (32*12)) % 4) * 6;
			outPort = rand() % 6 + ((destId / (32*12)) / 12) * 6;
		else
			assert(false);
	}

	if (outPort < 0 || outPort > 23) {
		cout << "Error routing port " << outPort << endl;
		cout << "INFO:" << endl;
		cout << "srcId:" << srcId << ", destId:" << destId << ", currLevel:" << currLevel 
			 << ", ancestorLevel:" << ancestorLevel << ", inPort:" << inPort << endl;
		assert(false);
	}

	if (Verbose >= LOG) {
		cout << "LOG: At time " << simTime() << ", flit " << flit->getName() << " at " << name << " switch ";
		string id = name == "Leaf" ? name + to_string(getIndex()) : 
			name + to_string(getParentModule()->getIndex()) + "_nrc" + to_string(getIndex());
		cout << id << " is routed to outPort " << outPort << endl;
		cout << "INFO:" << endl;
		cout << "srcId:" << srcId << ", destId:" << destId << ", currLevel:" << currLevel 
			 << ", ancestorLevel:" << ancestorLevel << ", inPort:" << inPort << endl;
	}
    
	return outPort;
}

int THNRC::nextPort(int thisPort) {
    if (connectToNode(thisPort))
        return 0;

	int port;
	string name = par("name").stdstringValue();

	if (name != "") 
		assert(name == "Leaf");
	else {
		assert(getParentModule() != nullptr);
		name = getParentModule()->par("name").stdstringValue();
	}

	if (name == "NRM") { // NRM switch
		if (getIndex() < 3) { // Undermost NRC's upper ports
			assert(thisPort >= 12); // should return 0 at function beginning
			port = (thisPort - 12) % 4 + getIndex() * 4;
		} else {
			assert(getIndex() < 6);
			if (thisPort < 12) // second layer NRC's lower ports
				port = thisPort % 4 + (getIndex() - 3) * 4 + 12;
			else // second layer NRC's upper ports
				port = getParentModule()->getIndex() % 12;
		}

	} else if (name == "Leaf") { // Leaf switch
		if (thisPort < 12) // leaf switch NRC's lower ports
			port = (getIndex() % 36) % 12 + 12;
		else // leaft switch NRC's upper ports
			port = (getIndex() / 36) % 12;

	} else if (name == "Root") {
		if (getIndex() < 4) { // lower layer NRC's ports
			if (thisPort < 12)
				port = getParentModule()->getIndex() % 12 + 12;
			else
				port = (thisPort - 12) % 6 + getIndex() * 6;
		} else {
			port = thisPort % 6 + (getIndex() - 4) * 6 + 12;
		}
	} else {
		cout << "Invalid name: " << name << "!" << endl;
		assert(false);
	}

	if (port < 0 || port > 23) {
		cout << "Error calculated next port " << port << " in switch " << name << endl;
		assert(false);
	}
	
	return port;
}

bool THNRC::connectToNode(int port) {
	string name = par("name").stdstringValue();
	if (name != "") { // leaf switch
		assert(name == "Leaf");
		return false;
	}
    assert(getParentModule() != nullptr);
	name = getParentModule()->par("name").stdstringValue();
	return (name == "NRM" && getIndex() < 3 && port < 12);
}
