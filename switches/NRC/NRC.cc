#include "NRC.h"
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>

using flitbuf = std::deque<Flit *>;

NRC::NRC() {}

NRC::~NRC() {
  cancelAndDelete(self_timer_);
  for (auto i = 0; i < P; i++) {
    while (!free_msg_queue_[i].empty()) {
      delete free_msg_queue_[i].front();
      free_msg_queue_[i].pop_front();
    }
  }

  for (auto i = 0; i < P; i++) {
    for (auto j = 0; j < PipelineDepth - 1; j++) {
      if (forward_queue_[i][j] != nullptr) {
        delete forward_queue_[i][j];
      }
    }
  }

  for (auto i = 0; i < P; i++) {
    for (auto j = 0; j < V; j++) {
      while (!input_buffer_[i][j].empty()) {
        delete input_buffer_[i][j].front();
        input_buffer_[i][j].pop_front();
      }
    }
  }

  for (auto i = 0; i < H; i++)
    for (auto j = 0; j < W; j++)
      for (auto k = 0; k < W; k++)
        for (auto b = 0; b < H; b++)
          for (auto v = 0; v < V; v++)
            while (!switch_buffer_[i][j][k][b][v].empty()) {
              delete switch_buffer_[i][j][k][b][v].front();
              switch_buffer_[i][j][k][b][v].pop_front();
            }

  for (auto i = 0; i < H; i++)
    for (auto j = 0; j < W; j++)
      for (auto k = 0; k < H; k++)
        for (auto v = 0; v < V; v++)
          while (!column_buffer_[i][j][k][v].empty()) {
            delete column_buffer_[i][j][k][v].front();
            column_buffer_[i][j][k][v].pop_front();
          }
}

void NRC::initialize() {
  self_timer_ = new omnetpp::cMessage("self_timer");
  scheduleAt(SimStartTime, self_timer_);

  std::fill(routing_result_[0],
            routing_result_[0] + sizeof(routing_result_) / sizeof(int32_t), -1);
  std::fill(credit_[0], credit_[0] + sizeof(credit_) / sizeof(int32_t),
            InputBufferDepth);
  memset(vca1_state_[0][0][0][0], 0, sizeof(vca1_state_));
  std::fill(vca1_result_[0][0][0],
            vca1_result_[0][0][0] + sizeof(vca1_result_) / sizeof(int32_t), -1);
  std::fill(vca1_result_prev_[0][0][0],
            vca1_result_prev_[0][0][0] +
                sizeof(vca1_result_prev_) / sizeof(int32_t),
            -1);
  std::fill(sa1_result_[0][0],
            sa1_result_[0][0] + sizeof(sa1_result_) / sizeof(int32_t), -1);
  memset(vca2_state_[0][0][0], 0, sizeof(vca2_state_));
  std::fill(vca2_port_[0][0],
            vca2_port_[0][0] + sizeof(vca2_port_) / sizeof(int32_t), -1);
  std::fill(vca2_port_prev_[0][0],
            vca2_port_prev_[0][0] + sizeof(vca2_port_prev_) / sizeof(int32_t),
            -1);
  std::fill(sa2_vcid_, sa2_vcid_ + sizeof(sa2_vcid_) / sizeof(int32_t), -1);
  std::fill(forward_queue_[0],
            forward_queue_[0] + sizeof(forward_queue_) / sizeof(Flit *),
            nullptr);
}

void NRC::handleMessage(omnetpp::cMessage *msg) {
  if (msg->isSelfMessage()) {
    handle_timer();
  } else if (strcmp("freeMsg", msg->getName()) == 0) {
    handle_free_msg(msg);
  } else {
    handle_flit(msg);
  }
}

void NRC::handle_timer() {
  scheduleAt(omnetpp::simTime() + ClockCycle, self_timer_);
  route_compute();
  row_move();
  first_virtual_channel_allocation();
  switch_allocation();
  switch_traversal();
  second_virtual_channel_allocation();
  port_allocation();
  send_free_msg();
  pipe();
  forward_flit();
}

void NRC::route_compute() {
  for (auto pi = 0; pi < P; pi++) {
    for (auto vi = 0; vi < V; vi++) {
      if (input_buffer_[pi][vi].empty())
        continue;
      Flit *flit = input_buffer_[pi][vi].front();
      if (flit->getIsHead() && routing_result_[pi][vi] == -1) {
        auto po = get_routing_port(flit), vo = get_best_vcid(po);
        std::cerr << get_log(log_levels::info,
                             std::string("flit: ") + flit->getName() +
                                 " routed to port " + std::to_string(po));
        flit->setPort(po);
        flit->setVcid(vo);
        routing_result_[pi][vi] = po * V + vo;
      } else {
        assert(routing_result_[pi][vi] >= 0);
        auto po = routing_result_[pi][vi] / V;
        auto vo = routing_result_[pi][vi] % V;
        flit->setPort(po);
        flit->setVcid(vo);
      }
    }
  }
}

void NRC::row_move() {
  for (auto pi = 0; pi < P; pi++) {
    for (auto vi = 0; vi < V; vi++) {
      if (input_buffer_[pi][vi].empty()) {
        continue;
      }
      flitbuf &source = input_buffer_[pi][vi];
      Flit *flit = source.front();
      auto po = flit->getPort();
      auto ti = pi / W, tj = po % W, si = pi % W, sj = po % H;
      flitbuf &target = switch_buffer_[ti][tj][si][sj][vi];
      if (target.size() == SwitchBufferDepth) {
        continue;
      }
      if (flit->getIsTail()) {
        routing_result_[pi][vi] = -1;
      }
      target.push_back(flit);
      source.pop_front();
      auto port = get_next_port(pi);
      FreeMsg *freeMsg = new FreeMsg("freeMsg");
      freeMsg->setPort(port);
      freeMsg->setVcid(vi);
      free_msg_queue_[pi].push_back(freeMsg);
    }
  }
}

void NRC::first_virtual_channel_allocation() {
  for (auto ti = 0; ti < H; ti++) {
    for (auto tj = 0; tj < W; tj++) {
      for (auto po = 0; po < H; po++) {
        for (auto vo = 0; vo < V; vo++) {
          if (vca1_result_[ti][tj][po][vo] != -1) {
            continue;
          }
          auto r = vca1_result_prev_[ti][tj][po][vo] + 1, count = 0;
          for (; count < W * V; r++, count++) {
            auto result = r % (W * V);
            auto pi = result / V, vi = result % V;
            flitbuf &buffer = switch_buffer_[ti][tj][pi][po][vi];
            if (buffer.empty()) {
              continue;
            }
            Flit *flit = buffer.front();
            auto flit_po = flit->getPort();
            auto sub_po = flit_po % H;
            assert(sub_po == po);
            auto sub_vo = flit->getVcid();
            if (sub_vo == vo) {
              assert(flit->getIsHead());
              vca1_result_[ti][tj][po][vo] = result;
              vca1_result_prev_[ti][tj][po][vo] = result;
              vca1_state_[ti][tj][pi][po][vi] = true;
              break;
            }
          }
          assert(count != W * V || vca1_result_[ti][tj][po][vo] == -1);
        }
      }
    }
  }
}

void NRC::switch_allocation() {
  for (auto ti = 0; ti < H; ti++) {
    for (auto tj = 0; tj < W; tj++) {
      for (auto po = 0; po < H; po++) {
        auto r = sa1_result_[ti][tj][po] + 1, count = 0;
        for (; count < W * V; r++, count++) {
          auto result = r % (W * V);
          auto pi = result / V, vi = result % V;
          if (!vca1_state_[ti][tj][pi][po][vi]) { // lost in VA
            continue;
          }
          flitbuf &source = switch_buffer_[ti][tj][pi][po][vi];
          if (source.empty()) {
            continue;
          }
          Flit *flit = source.front();
          auto flit_po = flit->getPort(), flit_vo = flit->getVcid();
          assert(flit_po % H == po);
          auto dest_ti = flit_po / W, dest_tj = flit_po % W;
          assert(dest_tj == tj);
          flitbuf &target = column_buffer_[dest_ti][dest_tj][ti][flit_vo];
          if (target.size() == ColumnBufferDepth) {
            continue;
          }
          sa1_result_[ti][tj][po] = result;
          break;
        }
        if (count == W * V) {
          sa1_result_[ti][tj][po] = -1;
        }
      }
    }
  }
}

void NRC::switch_traversal() {
  for (auto ti = 0; ti < H; ti++) {
    for (auto tj = 0; tj < W; tj++) {
      for (auto po = 0; po < H; po++) {
        auto result = sa1_result_[ti][tj][po];
        if (result == -1) {
          continue;
        }
        auto pi = result / V, vi = result % V;
        flitbuf &source = switch_buffer_[ti][tj][pi][po][vi];
        assert(!source.empty());
        assert(vca1_state_[ti][tj][pi][po][vi]);
        Flit *flit = source.front();
        auto flit_po = flit->getPort();
        assert(flit_po % H == po);
        auto flit_vo = flit->getVcid();
        assert(sa1_result_[ti][tj][po] != -1);
        auto dest_ti = flit_po / W, dest_tj = flit_po % W;
        assert(dest_tj == tj);
        flitbuf &target = column_buffer_[dest_ti][dest_tj][ti][flit_vo];
        assert(target.size() < ColumnBufferDepth);
        target.push_back(flit);
        source.pop_front();
        if (flit->getIsTail()) {
          vca1_state_[ti][tj][pi][po][vi] = false;
          vca1_result_[ti][tj][po][flit_vo] = -1;
        }
      }
    }
  }
}

void NRC::second_virtual_channel_allocation() {
  for (auto ti = 0; ti < H; ti++) {
    for (auto tj = 0; tj < W; tj++) {
      for (auto vo = 0; vo < V; vo++) {
        if (vca2_port_[ti][tj][vo] != -1) {
          continue;
        }
        auto p = vca2_port_prev_[ti][tj][vo] + 1, count = 0;
        for (; count < H; p++, count++) {
          auto port = p % H;
          flitbuf &buffer = column_buffer_[ti][tj][port][vo];
          if (buffer.empty()) {
            continue;
          }
          if (credit_[ti * W + tj][vo] < PacketLength) {
            continue;
          }
          vca2_port_[ti][tj][vo] = port;
          vca2_port_prev_[ti][tj][vo] = port;
          break;
        }
        assert(count != H || vca2_port_[ti][tj][vo] == -1);
      }
    }
  }
}

void NRC::port_allocation() {
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

void NRC::send_free_msg() {
  for (auto p = 0; p < P; p++) {
    if (free_msg_queue_[p].empty()) {
      continue;
    }
    if (get_channel_available_time(p) > omnetpp::simTime()) {
      continue;
    }
    FreeMsg *freeMsg = free_msg_queue_[p].front();
    char poCstr[20];
    sprintf(poCstr, "port_%d$o", p);
    send(freeMsg, poCstr);
    free_msg_queue_[p].pop_front();
  }
}

void NRC::pipe() {
  auto N = PipelineDepth - 1;
  for (auto po = 0; po < P; po++) {
    if (forward_queue_[po][N - 1] != nullptr) {
      Flit *flit_sent = forward_queue_[po][N - 1];
      auto vi = flit_sent->getVcid();
      if (credit_[po][vi] == 0) {
        continue;
      }
      char poCstr[22];
      sprintf(poCstr, "port_%d$o", po);
      assert(get_channel_available_time(po) <= omnetpp::simTime());
      send(flit_sent, poCstr);
      std::cerr << get_log(log_levels::info, std::string("forwarded flit: ") +
                                                 flit_sent->getName() +
                                                 " at port " +
                                                 std::to_string(po));
      if (!connect_to_node(po)) {
        --credit_[po][vi];
      }
    }
    for (auto i = N - 1; i > 0; i--) {
      forward_queue_[po][i] = forward_queue_[po][i - 1];
    }
    forward_queue_[po][0] = nullptr;
  }
}

void NRC::forward_flit() {
  for (auto po = 0; po < P; po++) {
    auto vcid = sa2_vcid_[po];
    if (vcid == -1) {
      continue;
    }
    auto ti = po / W, tj = po % W, pi = vca2_port_[ti][tj][vcid];
    flitbuf &source = column_buffer_[ti][tj][pi][vcid];
    if (source.empty()) {
      continue;
    }
    if (forward_queue_[po][0] != nullptr) {
      continue;
    }
    Flit *flit = source.front();
    assert(po == flit->getPort());
    assert(vcid == flit->getVcid());
    flit->setHopCount(flit->getHopCount() + 1);
    auto next_pi = get_next_port(po);
    flit->setPort(next_pi);
    forward_queue_[po][0] = flit;
    source.pop_front();
    if (flit->getIsTail()) // reset VA2 Vcid
      vca2_port_[ti][tj][vcid] = -1;
  }
}

void NRC::handle_free_msg(omnetpp::cMessage *msg) {
  FreeMsg *freeMsg = omnetpp::check_and_cast<FreeMsg *>(msg);
  auto port = freeMsg->getPort();
  auto vcid = freeMsg->getVcid();
  ++credit_[port][vcid];
  delete freeMsg;
}

void NRC::handle_flit(omnetpp::cMessage *msg) {
  Flit *flit = omnetpp::check_and_cast<Flit *>(msg);
  auto p = flit->getPort();
  auto v = flit->getVcid();
  assert(input_buffer_[p][v].size() <= InputBufferDepth);
  input_buffer_[p][v].push_back(flit);
  std::cerr << get_log(log_levels::info,
                       std::string("received flit: ") + flit->getName());
}

int32_t NRC::get_best_vcid(int32_t port) {
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

omnetpp::simtime_t NRC::get_channel_available_time(int32_t port) {
  char poCstr[20];
  sprintf(poCstr, "port_%d$o", port);
  omnetpp::cChannel *channel = gate(poCstr)->getTransmissionChannel();
  omnetpp::simtime_t finishTime = channel->getTransmissionFinishTime();
  return finishTime;
}

inline std::string NRC::get_id() {
  char name[30];
  sprintf(name, "(NRC_%d,level_%d)", getIndex(), get_curr_level());
  return std::string(name);
}

std::string NRC::get_log(log_levels level, const std::string &msg) {
  if (log_lvl < level)
    return "";
  std::string lvl_type[4] = {"ERROR", "WARNING", "INFO", "DEBUG"};
  std::string log_msg = lvl_type[log_lvl] + "|at " +
                        std::to_string(omnetpp::simTime().dbl() * 1e9) +
                        "ns in " + get_id() + ", ";
  log_msg += msg + '\n';
  return log_msg;
}

void NRC::finish() {}
