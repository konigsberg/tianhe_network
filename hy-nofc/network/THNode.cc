#include "node.h"

class THNode : public Node {
protected:
  int32_t get_next_port() override { return (getIndex() % 32) % 12; };
  int32_t get_node_num() override { return 18432; };
};

Define_Module(THNode);
