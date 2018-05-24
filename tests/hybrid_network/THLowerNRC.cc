#include <cstdlib>

#include "../../switches/LowerNRC.h"

class THLowerNRC : public LowerNRC {

protected:
  int32_t get_node_num() override { return 18432; }

  int32_t get_curr_level() override { return 2; }

  int32_t get_routing_port(flit *f) override {
    auto ancestor_level = get_ancestor_level(f);
    auto curr_level = get_curr_level();
    auto in_port = f->getPort();
    auto out_port = 0;
    auto dest_id = f->getDest_id();

    if (curr_level < ancestor_level && in_port < 12)
      out_port = rand() % 12 + 12;
    else {
      auto k = dest_id - (dest_id / 32) * 32;
      if (k < 12)
        out_port = rand() % 4;
      else if (k < 24)
        out_port = rand() % 4 + 4;
      else
        out_port = rand() % 4 + 8;
    }
    assert(out_port >= 0 && out_port <= 23);
    return out_port;
  }

  int32_t get_next_port(int32_t this_port) override {
    if (connect_to_node(this_port))
      return 0;

    auto port = 0;
    if (this_port < 12)
      port = this_port % 4 + (getIndex() - 3) * 4 + 12;
    else
      port = getParentModule()->getIndex() % 12;
    assert(port >= 0 && port <= 23);
    return port;
  }

  bool connect_to_node(int32_t port) override { return false; }

  int32_t get_next_routing_port(flit *f) override {
    auto ancestor_level = get_ancestor_level(f);
    auto dest_id = f->getDest_id();
    auto next_port = 0;
    if (ancestor_level == 3) {
      next_port = (dest_id % (32 * 12)) / 32;
    } else {
      next_port = rand() % 12 + 12;
    }
    return next_port;
  }

  int32_t get_ancestor_level(flit *f) {
    auto src_id = f->getSrc_id();
    auto dest_id = f->getDest_id();
    auto max_id = std::max(src_id, dest_id);
    auto min_id = std::min(src_id, dest_id);
    auto ancestor_level = -1;
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
    return ancestor_level;
  }
};

Define_Module(THLowerNRC);
