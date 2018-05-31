#include <cstdlib>

#include "../../routers/BaseNRC.h"

class THBaseNRC : public BaseNRC {
protected:
  int32_t get_node_num() override { return 18432; }

  int32_t get_curr_level() override {
    std::string parent_name = getParentModule()->par("name").stdstringValue();
    if (parent_name == "NRM")
      return 1;
    return 5;
  }

  int32_t get_routing_port(flit *f) override {
    auto in_port = f->getPort(), out_port = 0;
    auto curr_level = get_curr_level();
    auto src_id = f->getSrc_id();
    auto dest_id = f->getDest_id();
    auto max_id = std::max(src_id, dest_id);
    auto min_id = std::min(src_id, dest_id);
    auto ancestor_level = 0;

    if (max_id - (min_id / 32) * 32 < 32) {
      max_id %= 32;
      min_id %= 32;
      if (max_id - (min_id / 12) * 12 < 12)
        ancestor_level = 1;
      else
        ancestor_level = 2;
    } else if (max_id - (min_id / (32 * 12)) * 32 * 12 < 32 * 12)
      ancestor_level = 3;
    else if (max_id - (min_id / (32 * 12 * 12)) * 32 * 12 * 12 < 32 * 12 * 12)
      ancestor_level = 4;
    else
      ancestor_level = 5;

    if (curr_level < ancestor_level && in_port < 12)
      out_port = rand() % 12 + 12;
    else {
      if (curr_level == 1)
        out_port = (dest_id - (dest_id / 32) * 32) % 12;
      else
        out_port = rand() % 6 + ((dest_id / (32 * 12)) / 12) * 6;
    }
    assert(out_port >= 0 && out_port <= 23);
    return out_port;
  }

  int32_t get_next_port(int32_t this_port) override {
    if (connect_to_node(this_port))
      return 0;
    auto curr_level = get_curr_level();
    auto port = -1;

    if (curr_level == 1) {
      assert(this_port >= 12);
      port = (this_port - 12) % 4 + getIndex() * 4;
    } else {
      port = this_port % 6 + getIndex() * 6 + 12;
    }
    assert(port >= 0 && port <= 23);
    return port;
  }

  bool connect_to_node(int32_t port) override {
    return get_curr_level() == 1 && port < P / 2;
  }
};

Define_Module(THBaseNRC);
