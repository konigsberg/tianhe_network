#include "../../switches/BaseNRC.h"

class THNRC : public BaseNRC {
protected:
  int32_t get_curr_level() override;
  int32_t get_routing_port(flit *f) override;
  int32_t get_next_port(int32_t port) override;
  bool connect_to_node(int32_t port) override;
  int32_t get_node_num() override;
};

Define_Module(THNRC);
