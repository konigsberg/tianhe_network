/*
 * THNode.cc
 *
 *  Created on: Dec 24, 2017
 *      Author: Konigsberg
 *
 */

#include "Node.h"

class THNode : public Node
{
	protected:
		int nextPort() override;
};

Define_Module(THNode);

int THNode::nextPort() {
	return (getIndex() % 32) % 12;
}
