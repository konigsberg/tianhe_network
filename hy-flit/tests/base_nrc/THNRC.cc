#include "THNRC.h"
#include <cstdlib>

int32_t THNRC::get_node_num() { return 18432; }

int32_t THNRC::get_curr_level() {
  int32_t curr_level = 0;
  std::string name = par("name").stdstringValue();
  if (name != "") {
    assert(name == "Leaf");
    curr_level = 3;
  } else {
    assert(getParentModule() != nullptr);
    name = getParentModule()->par("name").stdstringValue();
    if (name == "NRM") {
      if (getIndex() < 3)
        curr_level = 1;
      else
        curr_level = 2;
    } else {
      assert(name == "Root");
      if (getIndex() < 4)
        curr_level = 4;
      else
        curr_level = 5;
    }
  }
  return curr_level;
}

int32_t THNRC::get_routing_port(flit *f) {
  auto in_port = f->getPort(), out_port = 0;
  auto curr_level = get_curr_level();
  auto src_id = f->getSrc_id(), dest_id = f->getDest_id();
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
    else if (curr_level == 2) {
      auto k = dest_id - (dest_id / 32) * 32;
      if (k < 12)
        out_port = rand() % 4;
      else if (k < 24)
        out_port = rand() % 4 + 4;
      else
        out_port = rand() % 4 + 8;
    } else if (curr_level == 3)
      out_port = (dest_id % (32 * 12)) / 32;
    else if (curr_level == 4)
      out_port = (dest_id / (32 * 12)) % 12;
    else if (curr_level == 5)
      out_port = rand() % 6 + ((dest_id / (32 * 12)) / 12) * 6;
    else
      assert(false);
  }
  assert(out_port >= 0 && out_port <= 23);
  return out_port;
}

int32_t THNRC::get_next_port(int32_t this_port) {
  if (connect_to_node(this_port))
    return 0;
  auto port = 0;
  std::string name = par("name").stdstringValue();
  if (name != "")
    assert(name == "Leaf");
  else {
    assert(getParentModule() != nullptr);
    name = getParentModule()->par("name").stdstringValue();
  }
  if (name == "NRM") {
    if (getIndex() < 3) {
      assert(this_port >= 12);
      port = (this_port - 12) % 4 + getIndex() * 4;
    } else {
      assert(getIndex() < 6);
      if (this_port < 12)
        port = this_port % 4 + (getIndex() - 3) * 4 + 12;
      else
        port = getParentModule()->getIndex() % 12;
    }
  } else if (name == "Leaf") {
    if (this_port < 12)
      port = (getIndex() % 36) % 12 + 12;
    else
      port = (getIndex() / 36) % 12;
  } else if (name == "Root") {
    if (getIndex() < 4) {
      if (this_port < 12)
        port = getParentModule()->getIndex() % 12 + 12;
      else
        port = (this_port - 12) % 6 + getIndex() * 6;
    } else {
      port = this_port % 6 + (getIndex() - 4) * 6 + 12;
    }
  } else {
    assert(false);
  }
  assert(port >= 0 && port <= 23);
  return port;
}

bool THNRC::connect_to_node(int32_t port) {
  std::string name = par("name").stdstringValue();
  if (name != "") { // leaf switch
    assert(name == "Leaf");
    return false;
  }
  assert(getParentModule() != nullptr);
  name = getParentModule()->par("name").stdstringValue();
  return (name == "NRM" && getIndex() < 3 && port < 12);
}
