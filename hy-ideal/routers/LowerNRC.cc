#include <algorithm>
#include <cassert>
#include <cstring>

#include "LowerNRC.h"

using buf = std::deque<flit *>;

LowerNRC::LowerNRC() {}

LowerNRC::~LowerNRC() {
  cancelAndDelete(self_timer_);

  // clear inbuf
  auto inbuf_flits = 0;
  for (auto i = 0; i < P; i++) {
    for (auto j = 0; j < V; j++) {
      while (!inbuf_[i][j].empty()) {
        delete inbuf_[i][j].front();
        inbuf_[i][j].pop_front();
        inbuf_flits++;
      }
    }
  }
  recordScalar("inbuf_flits", inbuf_flits);

  // clear swtbuf
  auto swtbuf_flits = 0;
  for (auto i = 0; i < H; i++)
    for (auto j = 0; j < W; j++)
      for (auto k = 0; k < W; k++)
        for (auto b = 0; b < H; b++)
          for (auto v = 0; v < V; v++)
            while (!swtbuf_[i][j][k][b][v].empty()) {
              delete swtbuf_[i][j][k][b][v].front();
              swtbuf_[i][j][k][b][v].pop_front();
              swtbuf_flits++;
            }
  recordScalar("swtbuf_flits", swtbuf_flits);

  // clearing free_msg_queue and forward_queue is unnecessary.
  // pipeline is removed. free_msg is integrated into flit.

  // clear colbuf
  auto colbuf_flits = 0;
  for (auto i = 0; i < H; i++)
    for (auto j = 0; j < W; j++)
      for (auto k = 0; k < H; k++)
        for (auto v = 0; v < V; v++)
          while (!colbuf_[i][j][k][v].empty()) {
            delete colbuf_[i][j][k][v].front();
            colbuf_[i][j][k][v].pop_front();
            colbuf_flits++;
          }
  recordScalar("colbuf_flits", colbuf_flits);

  // clear credit_queue
  auto credits = 0;
  for (auto p = 0; p < P; p++) {
    while (!credit_queue_[p].empty()) {
      delete credit_queue_[p].front();
      credit_queue_[p].pop_front();
      ++credits;
    }
  }
  recordScalar("credits", credits);
}

void LowerNRC::initialize() {
  self_timer_ = new omnetpp::cMessage("self_timer");
  scheduleAt(sim_start_time, self_timer_);

  std::fill(routing_result_[0],
            routing_result_[0] + sizeof(routing_result_) / sizeof(int32_t), -1);

  std::fill(next_routing_result_[0],
            next_routing_result_[0] +
                sizeof(next_routing_result_) / sizeof(int32_t),
            -1);

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

  std::fill(cabinet_remote_credit_counter_[0][0],
            cabinet_remote_credit_counter_[0][0] +
                sizeof(cabinet_remote_credit_counter_) / sizeof(int32_t),
            inbuf_capacity);

  std::fill(cabinet_local_credit_counter_[0][0],
            cabinet_local_credit_counter_[0][0] +
                sizeof(cabinet_local_credit_counter_) / sizeof(int32_t),
            inbuf_capacity);
}

void LowerNRC::handleMessage(omnetpp::cMessage *msg) {
  if (msg->isSelfMessage()) {
    timer_cb();
  } else if (strcmp(msg->getName(), "credit") == 0 ||
             strcmp(msg->getName(), "credit_lower") == 0 ||
             strcmp(msg->getName(), "credit_upper") == 0) {
    credit_cb(msg);
  } else if (strcmp(msg->getName(), "exchange") == 0) {
    exchange_cb(msg);
  } else {
    flit_cb(msg);
  }
}

void LowerNRC::timer_cb() {
  scheduleAt(omnetpp::simTime() + clk_cycle, self_timer_);
  route_compute();
  row_move();
  first_virtual_channel_allocation();
  switch_allocation();
  switch_traversal();
  lower_port_second_virtual_channel_allocation();
  lower_port_allocation();
  lower_port_forward_flit();
  upper_port_select_packet();
}

#define ROUTE_COMPUTE_FOR_FLIT(pi, vi)                                         \
  {                                                                            \
    if (inbuf_[pi][vi].empty())                                                \
      continue;                                                                \
                                                                               \
    flit *f = inbuf_[pi][vi].front();                                          \
                                                                               \
    if (f->getIs_head() && routing_result_[pi][vi] == -1) {                    \
      auto po = get_routing_port(f);                                           \
      auto vo = rand() % V;                                                    \
                                                                               \
      if (po >= P / 2) {                                                       \
        auto next_po = get_next_routing_port(f);                               \
        f->setNext_port(next_po);                                              \
        next_routing_result_[pi][vi] = next_po;                                \
        if (next_po < P / 2)                                                   \
          vo = rand() % (V / 2) + V / 2;                                       \
        else                                                                   \
          vo = rand() % V;                                                     \
      } else                                                                   \
        next_routing_result_[pi][vi] = -1;                                     \
                                                                               \
      f->setPort(po);                                                          \
      f->setVcid(vo);                                                          \
      routing_result_[pi][vi] = po * V + vo;                                   \
    } else {                                                                   \
      auto po = routing_result_[pi][vi] / V;                                   \
      auto vo = routing_result_[pi][vi] % V;                                   \
      f->setPort(po);                                                          \
      f->setVcid(vo);                                                          \
      f->setNext_port(next_routing_result_[pi][vi]);                           \
    }                                                                          \
  }

void LowerNRC::route_compute() {
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
      next_routing_result_[pi][vi] = -1;                                       \
    }                                                                          \
    target.push_back(f);                                                       \
    source.pop_front();                                                        \
                                                                               \
    auto cdt = new credit("credit_lower");                                     \
                                                                               \
    if (pi < P / 2) {                                                          \
      cdt->setOs(-1);                                                          \
      cdt->setPort(get_next_port(pi));                                         \
      cdt->setVc(vi);                                                          \
      credit_queue_[pi].push_back(cdt);                                        \
    } else if (vi < V / 2) {                                                   \
      cdt->setOs(get_cabinet_id());                                            \
      cdt->setPort(get_nrm_id());                                              \
      cdt->setVc(vi);                                                          \
      credit_queue_[pi].push_back(cdt);                                        \
    } else {                                                                   \
      auto os = getIndex() * P / 2 + (po - P / 2);                             \
      auto port = get_nrm_id();                                                \
      update_credit(local, os, port, vi, 1);                                   \
    }                                                                          \
                                                                               \
    if (channel_is_available(pi) && !credit_queue_[pi].empty()) {              \
      cdt = credit_queue_[pi].front();                                         \
      char pi_cstr[20];                                                        \
      sprintf(pi_cstr, "port_%d$o", pi);                                       \
      send(cdt, pi_cstr);                                                      \
      credit_queue_[pi].pop_front();                                           \
      std::cerr << get_log(log_levels::debug,                                  \
                           "sending new credit " + credit_string(cdt));        \
    }                                                                          \
  }

void LowerNRC::row_move() {
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

void LowerNRC::first_virtual_channel_allocation() {
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

void LowerNRC::switch_allocation() {
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

void LowerNRC::switch_traversal() {
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

void LowerNRC::lower_port_second_virtual_channel_allocation() {
  for (auto ti = 0; ti < H / 2; ti++) {
    for (auto tj = 0; tj < W; tj++) {
      for (auto vo = 0; vo < V; vo++) {
        SECOND_VCA_FOR_FLIT(ti, tj, vo);
      }
    }
  }
}

void LowerNRC::lower_port_allocation() {
  for (auto po = 0; po < P / 2; po++) {
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

void LowerNRC::lower_port_forward_flit() {
  for (auto po = 0; po < P / 2; po++) {
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

    char po_str[20];
    sprintf(po_str, "port_%d$o", po);
    assert(channel_is_available(po));
    f->setHop_count(f->getHop_count() + 1);
    send(f, po_str);
    std::cerr << get_log(log_levels::info, std::string("forwarded flit: ") +
                                               f->getName() + " at port " +
                                               std::to_string(po));
    source.pop_front();
    if (f->getIs_tail()) // reset VA2 Vcid
      vca2_port_[ti][tj][vcid] = -1;
  }
}

void LowerNRC::upper_port_select_packet() {
  for (auto po = P / 2; po < P; po++) {
    auto ti = po / W;
    auto tj = po % W;
    for (auto v = 0; v < H * V; v++) {
      auto k = v / V;
      auto vc = v % V;
      buf &buffer = colbuf_[ti][tj][k][vc];
      if (buffer.size() < packet_length)
        continue;

      flit *f = buffer.front();
      auto next_pi = get_next_port(po);
      auto next_po = f->getNext_port();
      if (is_matched(next_po, get_switched_port(next_pi)) && is_right_time()) {
        auto cabinet_id = get_cabinet_id();
        auto os_id = getIndex() * P / 2 + (po - P / 2);
        auto vc = f->getVcid();

        int32_t *credit = nullptr;
        if (next_po < P / 2)
          credit = &(cabinet_local_credit_counter_[os_id][next_po][vc]);
        else
          credit =
              &(cabinet_remote_credit_counter_[os_id][next_po - P / 2][vc]);
        if (*credit < packet_length)
          continue;

        if (!channel_is_available(po))
          continue;

        upper_port_forward_packet(po, buffer);

        if (next_po < P / 2)
          update_credit(local, os_id, next_po, vc, -packet_length);
        else
          update_credit(remote, os_id, next_po - P / 2, vc, -packet_length);

        break;
      }
    }
  }
}

void LowerNRC::upper_port_forward_packet(int32_t po, buf &buffer) {
  char po_cstr[20];
  sprintf(po_cstr, "port_%d$o", po);
  for (auto count = 0; count < packet_length; count++) {
    auto f = buffer.front();
    f->setPort(get_next_port(po));

    std::cerr << get_log(log_levels::info, std::string("forwarded flit: ") +
                                               f->getName() + " at port " +
                                               std::to_string(po));
    sendDelayed(f, count * (clk_cycle + margin), po_cstr);
    buffer.pop_front();
  }
}

void LowerNRC::flit_cb(omnetpp::cMessage *msg) {
  flit *f = omnetpp::check_and_cast<flit *>(msg);
  auto p = f->getPort();
  auto v = f->getVcid();
  if (inbuf_[p][v].size() > inbuf_capacity)
    std::cerr << get_log(log_levels::critical,
                         "router " + get_id() + " input buffer overflow");
  // assert(inbuf_[p][v].size() <= inbuf_capacity);

  inbuf_[p][v].push_back(f);
  std::cerr << get_log(log_levels::info, std::string("received flit ") +
                                             f->getName() + " from port " +
                                             std::to_string(p));
}

void LowerNRC::credit_cb(omnetpp::cMessage *msg) {
  credit *cdt = omnetpp::check_and_cast<credit *>(msg);
  std::cerr << get_log(log_levels::debug,
                       "receiving credit " + credit_string(cdt));
  auto os = cdt->getOs();
  auto port = cdt->getPort();
  auto vc = cdt->getVc();
  delete cdt;

  if (os == -1)
    ++credit_[port][vc];
  else
    update_credit(remote, os, port, vc, 1);
}

void LowerNRC::exchange_cb(omnetpp::cMessage *msg) {
  exchange *exc = omnetpp::check_and_cast<exchange *>(msg);
  auto os = exc->getOs();
  auto port = exc->getPort();
  auto vc = exc->getVc();
  auto credit = exc->getCredit();
  if (exc->getType() == local) {
    cabinet_local_credit_counter_[os][port][vc] = credit;
  } else {
    cabinet_remote_credit_counter_[os][port][vc] = credit;
  }
  delete exc;
}

bool LowerNRC::channel_is_available(int32_t port) {
  char poCstr[20];
  sprintf(poCstr, "port_%d$o", port);
  omnetpp::cChannel *channel = gate(poCstr)->getTransmissionChannel();
  omnetpp::simtime_t finishTime = channel->getTransmissionFinishTime();
  return finishTime <= omnetpp::simTime();
}

inline std::string LowerNRC::get_id() {
  char name[30];
  sprintf(name, "(LowerNRC_%d,level_%d)", getIndex(), get_curr_level());
  return std::string(name);
}

std::string LowerNRC::get_log(log_levels level, const std::string &msg) {
  if (log_lvl < level)
    return "";
  std::string lvl_type[4] = {"CRI", "WARN", "INFO", "DBG"};
  std::string log_msg = lvl_type[uint8_t(level)] + "|at " +
                        std::to_string(omnetpp::simTime().dbl() * 1e9) +
                        "ns in " + get_id() + ", ";
  log_msg += msg + '\n';
  return log_msg;
}

inline int32_t LowerNRC::get_switched_port(int32_t in_port) {
  auto clk = uint64_t(omnetpp::simTime().dbl() / period);
  return (clk % 24 + in_port) % 24;
}

inline int32_t LowerNRC::get_cabinet_id() {
  return getParentModule()->getIndex() / 12;
}

inline int32_t LowerNRC::get_nrm_id() {
  return getParentModule()->getIndex() % 12;
}

inline bool LowerNRC::is_matched(int32_t request_po, int32_t switched_po) {
  if (request_po >= P / 2 && switched_po >= P / 2)
    return true;
  if (request_po == switched_po)
    return true;
  return false;
}

inline bool LowerNRC::is_right_time() {
  return uint64_t(omnetpp::simTime().dbl() / clk_cycle) % window == 0;
}

inline std::string LowerNRC::credit_string(credit *cdt) {
  std::string str("os=");
  str += std::to_string(cdt->getOs());
  str += ",port=";
  str += std::to_string(cdt->getPort());
  str += ",vc=";
  str += std::to_string(cdt->getVc());
  return str;
}

inline void LowerNRC::update_credit(int32_t type, int32_t os, int32_t port,
                                    int32_t vc, int32_t credit) {
  exchange *exc = new exchange();
  exc->setType(type);
  exc->setOs(os);
  exc->setPort(port);
  exc->setVc(vc);
  exc->setCredit(credit);
  send(exc, "port_24$o");
}

void LowerNRC::finish() {}
