#include <algorithm>
#include <cassert>
#include <cstring>

#include "BaseNRC.h"

using buf = std::deque<flit *>;

BaseNRC::BaseNRC() {}

BaseNRC::~BaseNRC() {
  cancelAndDelete(self_timer_);

  // clear inbuf
  for (auto i = 0; i < P; i++) {
    for (auto j = 0; j < V; j++) {
      while (!inbuf_[i][j].empty()) {
        delete inbuf_[i][j].front();
        inbuf_[i][j].pop_front();
      }
    }
  }

  // clear swtbuf
  for (auto i = 0; i < H; i++)
    for (auto j = 0; j < W; j++)
      for (auto k = 0; k < W; k++)
        for (auto b = 0; b < H; b++)
          for (auto v = 0; v < V; v++)
            while (!swtbuf_[i][j][k][b][v].empty()) {
              delete swtbuf_[i][j][k][b][v].front();
              swtbuf_[i][j][k][b][v].pop_front();
            }

  // clearing free_msg_queue and forward_queue is unnecessary.
  // pipeline is removed. free_msg is integrated into flit.

  // clear colbuf
  for (auto i = 0; i < H; i++)
    for (auto j = 0; j < W; j++)
      for (auto k = 0; k < H; k++)
        for (auto v = 0; v < V; v++)
          while (!colbuf_[i][j][k][v].empty()) {
            delete colbuf_[i][j][k][v].front();
            colbuf_[i][j][k][v].pop_front();
          }

  // clear credit_queue
  for (auto p = 0; p < P; p++) {
    while (!credit_queue_[p].empty()) {
      delete credit_queue_[p].front();
      credit_queue_[p].pop_front();
    }
  }
}

void BaseNRC::initialize() {
  self_timer_ = new omnetpp::cMessage("self_timer");
  scheduleAt(sim_start_time, self_timer_);

  std::fill(routing_result_[0],
            routing_result_[0] + sizeof(routing_result_) / sizeof(int32_t), -1);

  std::fill(credit_[0], credit_[0] + sizeof(credit_) / sizeof(int32_t),
            inbuf_capacity);

  memset(vca1_state_[0][0][0][0], 0, sizeof(vca1_state_));

  std::fill(vca1_result_[0][0][0],
            vca1_result_[0][0][0] + sizeof(vca1_result_) / sizeof(int32_t), -1);

  std::fill(vca1_result_prev_[0][0][0],
            vca1_result_prev_[0][0][0] +
                sizeof(vca1_result_prev_) / sizeof(int32_t),
            -1);

  std::fill(sa1_result_[0][0],
            sa1_result_[0][0] + sizeof(sa1_result_) / sizeof(int32_t), -1);

  std::fill(vca2_port_[0][0],
            vca2_port_[0][0] + sizeof(vca2_port_) / sizeof(int32_t), -1);

  std::fill(vca2_port_prev_[0][0],
            vca2_port_prev_[0][0] + sizeof(vca2_port_prev_) / sizeof(int32_t),
            -1);

  std::fill(sa2_vcid_, sa2_vcid_ + sizeof(sa2_vcid_) / sizeof(int32_t), -1);
}

void BaseNRC::handleMessage(omnetpp::cMessage *msg) {
  if (msg->isSelfMessage()) {
    timer_cb();
  } else {
    flit_cb(msg);
  }
}

void BaseNRC::timer_cb() {
  scheduleAt(omnetpp::simTime() + clk_cycle, self_timer_);
  route_compute();
  row_move();
  first_virtual_channel_allocation();
  switch_allocation();
  switch_traversal();
  second_virtual_channel_allocation();
  port_allocation();
  forward_flit();
}

#define ROUTE_COMPUTE_FOR_FLIT(pi, vi)                                         \
  {                                                                            \
    if (inbuf_[pi][vi].empty())                                                \
      continue;                                                                \
    flit *f = inbuf_[pi][vi].front();                                          \
                                                                               \
    if (f->getIs_head() && routing_result_[pi][vi] == -1) {                    \
      auto po = get_routing_port(f);                                           \
      auto vo = get_best_vcid(po);                                             \
      f->setPort(po);                                                          \
      f->setVcid(vo);                                                          \
      routing_result_[pi][vi] = po * V + vo;                                   \
    } else {                                                                   \
      auto po = routing_result_[pi][vi] / V;                                   \
      auto vo = routing_result_[pi][vi] % V;                                   \
      f->setPort(po);                                                          \
      f->setVcid(vo);                                                          \
    }                                                                          \
  }

void BaseNRC::route_compute() {
  for (auto pi = 0; pi < P; pi++) {
    for (auto vi = 0; vi < V; vi++) {
      ROUTE_COMPUTE_FOR_FLIT(pi, vi);
    }
  }
}

#define ROW_MOVE_FOR_BUF(pi, vi)                                               \
  {                                                                            \
    if (inbuf_[pi][vi].empty())                                                \
      continue;                                                                \
                                                                               \
    buf &source = inbuf_[pi][vi];                                              \
    flit *f = source.front();                                                  \
    auto po = f->getPort();                                                    \
    auto ti = pi / W;                                                          \
    auto tj = po % W;                                                          \
    auto si = pi % W;                                                          \
    auto sj = po % H;                                                          \
    buf &target = swtbuf_[ti][tj][si][sj][vi];                                 \
                                                                               \
    if (target.size() == swtbuf_capacity)                                      \
      continue;                                                                \
                                                                               \
    if (f->getIs_tail()) {                                                     \
      routing_result_[pi][vi] = -1;                                            \
    }                                                                          \
    target.push_back(f);                                                       \
    source.pop_front();                                                        \
                                                                               \
    auto cdt = new credit();                                                   \
    cdt->setOs(-1);                                                            \
    cdt->setPort(get_next_port(pi));                                           \
    cdt->setVc(vi);                                                            \
    credit_queue_[pi].push_back(cdt);                                          \
  }

void BaseNRC::row_move() {
  for (auto pi = 0; pi < P; pi++) {
    for (auto vi = 0; vi < V; vi++) {
      ROW_MOVE_FOR_BUF(pi, vi);
    }
  }
}

#define FIRST_VCA_FOR_FLIT(ti, tj, po, vo)                                     \
  {                                                                            \
    if (vca1_result_[ti][tj][po][vo] != -1)                                    \
      continue;                                                                \
                                                                               \
    auto r = vca1_result_prev_[ti][tj][po][vo] + 1;                            \
    auto count = 0;                                                            \
                                                                               \
    for (; count < W * V; r++, count++) {                                      \
      auto result = r % (W * V);                                               \
      auto pi = result / V, vi = result % V;                                   \
      buf &buffer = swtbuf_[ti][tj][pi][po][vi];                               \
      if (buffer.empty())                                                      \
        continue;                                                              \
                                                                               \
      flit *f = buffer.front();                                                \
      auto sub_vo = f->getVcid();                                              \
      if (sub_vo == vo) {                                                      \
        vca1_result_[ti][tj][po][vo] = result;                                 \
        vca1_result_prev_[ti][tj][po][vo] = result;                            \
        vca1_state_[ti][tj][pi][po][vi] = true;                                \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
  }

void BaseNRC::first_virtual_channel_allocation() {
  for (auto ti = 0; ti < H; ti++) {
    for (auto tj = 0; tj < W; tj++) {
      for (auto po = 0; po < H; po++) {
        for (auto vo = 0; vo < V; vo++) {
          FIRST_VCA_FOR_FLIT(ti, tj, po, vo);
        }
      }
    }
  }
}

#define SWITCH_ALLOC_FOR_PORT(ti, tj, po)                                      \
  {                                                                            \
    auto r = sa1_result_[ti][tj][po] + 1, count = 0;                           \
    for (; count < W * V; r++, count++) {                                      \
      auto result = r % (W * V);                                               \
      auto pi = result / V, vi = result % V;                                   \
      if (!vca1_state_[ti][tj][pi][po][vi])                                    \
        continue;                                                              \
                                                                               \
      buf &source = swtbuf_[ti][tj][pi][po][vi];                               \
      if (source.empty())                                                      \
        continue;                                                              \
                                                                               \
      flit *f = source.front();                                                \
      auto flit_po = f->getPort();                                             \
      auto flit_vo = f->getVcid();                                             \
      auto dest_ti = flit_po / W, dest_tj = flit_po % W;                       \
      buf &target = colbuf_[dest_ti][dest_tj][ti][flit_vo];                    \
      if (target.size() == colbuf_capacity)                                    \
        continue;                                                              \
                                                                               \
      sa1_result_[ti][tj][po] = result;                                        \
      break;                                                                   \
    }                                                                          \
    if (count == W * V)                                                        \
      sa1_result_[ti][tj][po] = -1;                                            \
  }

void BaseNRC::switch_allocation() {
  for (auto ti = 0; ti < H; ti++) {
    for (auto tj = 0; tj < W; tj++) {
      for (auto po = 0; po < H; po++) {
        SWITCH_ALLOC_FOR_PORT(ti, tj, po);
      }
    }
  }
}

#define SWITCH_TRAV_FOR_PORT(ti, tj, po)                                       \
  {                                                                            \
    auto result = sa1_result_[ti][tj][po];                                     \
    if (result == -1)                                                          \
      continue;                                                                \
                                                                               \
    auto pi = result / V, vi = result % V;                                     \
    buf &source = swtbuf_[ti][tj][pi][po][vi];                                 \
    flit *f = source.front();                                                  \
    auto flit_po = f->getPort();                                               \
    auto flit_vo = f->getVcid();                                               \
    auto dest_ti = flit_po / W, dest_tj = flit_po % W;                         \
    buf &target = colbuf_[dest_ti][dest_tj][ti][flit_vo];                      \
    target.push_back(f);                                                       \
    source.pop_front();                                                        \
    if (f->getIs_tail()) {                                                     \
      vca1_state_[ti][tj][pi][po][vi] = false;                                 \
      vca1_result_[ti][tj][po][flit_vo] = -1;                                  \
    }                                                                          \
  }

void BaseNRC::switch_traversal() {
  for (auto ti = 0; ti < H; ti++) {
    for (auto tj = 0; tj < W; tj++) {
      for (auto po = 0; po < H; po++) {
        SWITCH_TRAV_FOR_PORT(ti, tj, po);
      }
    }
  }
}

#define SECOND_VCA_FOR_FLIT(ti, tj, vo)                                        \
  {                                                                            \
    if (vca2_port_[ti][tj][vo] != -1)                                          \
      continue;                                                                \
                                                                               \
    auto p = vca2_port_prev_[ti][tj][vo] + 1, count = 0;                       \
    for (; count < H; p++, count++) {                                          \
      auto port = p % H;                                                       \
      buf &buffer = colbuf_[ti][tj][port][vo];                                 \
      if (buffer.empty())                                                      \
        continue;                                                              \
                                                                               \
      if (credit_[ti * W + tj][vo] < packet_length)                            \
        continue;                                                              \
                                                                               \
      vca2_port_[ti][tj][vo] = port;                                           \
      vca2_port_prev_[ti][tj][vo] = port;                                      \
      break;                                                                   \
    }                                                                          \
  }

void BaseNRC::second_virtual_channel_allocation() {
  for (auto ti = 0; ti < H; ti++) {
    for (auto tj = 0; tj < W; tj++) {
      for (auto vo = 0; vo < V; vo++) {
        SECOND_VCA_FOR_FLIT(ti, tj, vo);
      }
    }
  }
}

void BaseNRC::port_allocation() {
  for (auto po = 0; po < P; po++) {
    auto v = sa2_vcid_[po] + 1, count = 0;
    for (; count < V; v++, count++) {
      auto vcid = v % V, ti = po / W, tj = po % W;
      if (vca2_port_[ti][tj][vcid] == -1) {
        continue;
      }
      if (credit_[po][vcid] == 0) {
        continue;
      }
      sa2_vcid_[po] = vcid;
      break;
    }
    if (count == V) {
      sa2_vcid_[po] = -1;
    }
  }
}

void BaseNRC::forward_flit() {
  for (auto po = 0; po < P; po++) {
    auto vcid = sa2_vcid_[po];
    if (vcid == -1)
      continue;

    auto ti = po / W;
    auto tj = po % W;
    auto pi = vca2_port_[ti][tj][vcid];
    buf &source = colbuf_[ti][tj][pi][vcid];
    if (source.empty())
      continue;

    flit *f = source.front();
    auto vi = f->getVcid();
    if (credit_[po][vi] == 0)
      continue;

    --credit_[po][vi];

    auto next_pi = get_next_port(po);
    f->setPort(next_pi);

    assert(f->getCredit_vc() == -1);

    if (!credit_queue_[po].empty()) {
      auto cdt = credit_queue_[po].front();
      f->setCredit_os(cdt->getOs());
      f->setCredit_port(cdt->getPort());
      f->setCredit_vc(cdt->getVc());
      delete cdt;
      credit_queue_[po].pop_front();
    } else {
      f->setCredit_vc(-1);
    }

    f->setHop_count(f->getHop_count() + 1);

    char po_str[20];
    sprintf(po_str, "port_%d$o", po);
    assert(get_channel_available_time(po) <= omnetpp::simTime());
    send(f, po_str);
    std::cerr << get_log(log_levels::info, std::string("forwarded flit: ") +
                                               f->getName() + " at port " +
                                               std::to_string(po));
    source.pop_front();
    if (f->getIs_tail()) // reset VA2 Vcid
      vca2_port_[ti][tj][vcid] = -1;
  }
}

void BaseNRC::flit_cb(omnetpp::cMessage *msg) {
  flit *f = omnetpp::check_and_cast<flit *>(msg);
  auto p = f->getPort();
  auto v = f->getVcid();
  if (inbuf_[p][v].size() > inbuf_capacity)
    std::cerr << get_log(log_levels::critical,
                         "router " + get_id() + " input buffer overflow");
  assert(inbuf_[p][v].size() <= inbuf_capacity);

  inbuf_[p][v].push_back(f);
  std::cerr << get_log(log_levels::info, std::string("received flit ") +
                                             f->getName() + " from port " +
                                             std::to_string(p));

  if (f->getCredit_vc() != -1) {
    auto port = f->getCredit_port();
    auto vc = f->getCredit_vc();
    ++credit_[port][vc];
    f->setCredit_vc(-1);
  }
}

int32_t BaseNRC::get_best_vcid(int32_t port) {
  if (get_node_num() == P) {
    return rand() % V;
  } else {
    int max_credit = 0, max_vcid = 0;
    for (int v = 0; v < V; v++) {
      if (credit_[port][v] > max_credit) {
        max_credit = credit_[port][v];
        max_vcid = v;
      }
    }
    return max_vcid;
  }
}

omnetpp::simtime_t BaseNRC::get_channel_available_time(int32_t port) {
  char poCstr[20];
  sprintf(poCstr, "port_%d$o", port);
  omnetpp::cChannel *channel = gate(poCstr)->getTransmissionChannel();
  omnetpp::simtime_t finishTime = channel->getTransmissionFinishTime();
  return finishTime;
}

inline std::string BaseNRC::get_id() {
  char name[30];
  sprintf(name, "(BaseNRC_%d,level_%d)", getIndex(), get_curr_level());
  return std::string(name);
}

std::string BaseNRC::get_log(log_levels level, const std::string &msg) {
  if (log_lvl < level)
    return "";
  std::string lvl_type[4] = {"CRI", "WARN", "INFO", "DBG"};
  std::string log_msg = lvl_type[uint8_t(level)] + "|at " +
                        std::to_string(omnetpp::simTime().dbl() * 1e9) +
                        "ns in " + get_id() + ", ";
  log_msg += msg + '\n';
  return log_msg;
}

void BaseNRC::finish() {}
