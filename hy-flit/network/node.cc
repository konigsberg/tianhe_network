#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "node.h"

int32_t delays[10] = {0, 100, 0, 300, 0, 500, 0, 800, 0, 1000};

Node::Node() {}

Node::~Node() {
  cancelAndDelete(gen_timer_);
  cancelAndDelete(send_timer_);
  while (!send_queue_.empty()) {
    delete send_queue_.front();
    send_queue_.pop_front();
  }
}

void Node::initialize() {
  gen_timer_ = new omnetpp::cMessage("gen_timer");
  send_timer_ = new omnetpp::cMessage("send_timer");
  scheduleAt(sim_start_time, gen_timer_);
  scheduleAt(sim_start_time, send_timer_);
  std::fill(credit_, credit_ + V, inbuf_capacity);
}

void Node::handleMessage(omnetpp::cMessage *msg) {
  if (msg == gen_timer_) {
    gen_timer_cb();
  } else if (msg == send_timer_) {
    send_timer_cb();
  } else {
    flit_cb(msg);
  }
}

void Node::gen_timer_cb() {
  if (debug_mode && (total_flit_sent_ == 20 || getIndex() >= 10))
    return;

  if (send_queue_.size() + sizeof_flit <= nodebuf_capacity) {
    auto v = get_best_vcid();
    auto dest_id = get_dest_id();
    for (auto i = 0; i < packet_length; i++) {
      flit *f = flit_factory(i == 0, i == packet_length - 1, v, total_flit_gen_,
                             dest_id);
      send_queue_.push_back(f);
      if (omnetpp::simTime() >= record_start_time)
        ++total_flit_gen_;
    }
  } else if (omnetpp::simTime() >= record_start_time) {
    ++total_packet_dropped_;
  }

  double interval =
      std::max((1.0 / injection_rate) * clk_cycle * packet_length, clk_cycle);
  scheduleAt(omnetpp::simTime() + interval, gen_timer_);
}

void Node::send_timer_cb() {
  scheduleAt(
      std::max(omnetpp::simTime() + clk_cycle, get_channel_available_time()),
      send_timer_);
  if (send_queue_.empty())
    return;

  if (get_channel_available_time() > omnetpp::simTime())
    return;

  flit *f = send_queue_.front();
  auto v = f->getVcid();
  if (credit_[v] == 0)
    return;

  auto pi = get_next_port();
  f->setPort(pi);
  f->setSend_time(omnetpp::simTime().dbl());
  send(f, "port$o");

  std::cerr << get_log(log_levels::info,
                       std::string("sent flit: ") + f->getName());

  if (omnetpp::simTime() >= record_start_time) {
    ++total_flit_sent_;
    if (f->getIs_tail())
      ++total_packet_sent_;
  }
  send_queue_.pop_front();
  --credit_[v];
}

void Node::flit_cb(omnetpp::cMessage *msg) {
  flit *f = omnetpp::check_and_cast<flit *>(msg);
  std::cerr << get_log(log_levels::info,
                       std::string("got flit: ") + f->getName());
  auto dest_id = f->getDest_id();

  if (dest_id != getIndex()) {
    std::string msg("received wrong flit ");
    std::cerr << get_log(log_levels::critical, msg + f->getName() +
                                                   " at node " +
                                                   std::to_string(getIndex()));
  }
  assert(dest_id == getIndex());

  if (omnetpp::simTime() >= record_start_time) {
    ++total_flit_received_;
    auto flit_latency = omnetpp::simTime().dbl() - f->getSend_time() +
                        delays[f->getHop_count()];
    total_latency_ += flit_latency;
    if (f->getIs_tail())
      ++total_packet_received_;
  }

  auto v = f->getCredit_vc();
  if (v != -1) {
    ++credit_[v];
  }
  delete f;
}

int32_t Node::get_best_vcid() {
  auto max_credit = 0, max_vcid = 0;
  for (auto v = 0; v < V; v++)
    if (credit_[v] > max_credit) {
      max_credit = credit_[v];
      max_vcid = v;
    }
  return max_vcid;
}

flit *Node::flit_factory(bool is_head, bool is_tail, int32_t vcid,
                         int32_t flit_id, int32_t dest_id) {

  char flit_name[30];
  auto src_id = getIndex();
  if (is_head) {
    sprintf(flit_name, "HF%d-%d-to-%d", src_id, flit_id, dest_id);
  } else if (is_tail) {
    sprintf(flit_name, "TF%d-%d-to-%d", src_id, flit_id, dest_id);
  } else {
    sprintf(flit_name, "BF%d-%d-to-%d", src_id, flit_id, dest_id);
  }
  flit *f = new flit(flit_name);
  f->setByteLength(sizeof_flit);
  // f->setByteLength(0);
  f->setSrc_id(src_id);
  f->setDest_id(dest_id);
  f->setIs_head(is_head);
  f->setIs_tail(is_tail);
  f->setVcid(vcid);
  f->setPort(get_next_port());
  f->setCredit_vc(-1);
  f->setHop_count(0);
  return f;
}

int32_t Node::get_dest_id() {
  int32_t dest_id = 0;
  if (traffic_pattern == traffic_patterns::uniform) {
    dest_id = rand() % get_node_num();
  } else {
    std::cerr << "invalid traffic pattern" << std::endl;
    assert(false);
  }
  return dest_id;
}

omnetpp::simtime_t Node::get_channel_available_time() {
  omnetpp::cChannel *channel = gate("port$o")->getTransmissionChannel();
  omnetpp::simtime_t finish_time = channel->getTransmissionFinishTime();
  return finish_time;
}

void Node::finish() {
  uint64_t total_cycles =
      (omnetpp::simTime().dbl() - record_start_time) / clk_cycle;
  if (total_flit_received_ != 0)
    avg_latency_ = total_latency_ * 1.0 / total_flit_received_;
  recordScalar("total_cycles", total_cycles);
  recordScalar("total_flit_gen", total_flit_gen_);
  recordScalar("total_flit_sent", total_flit_sent_);
  recordScalar("total_packet_sent", total_packet_sent_);
  recordScalar("total_packet_dropped", total_packet_dropped_);
  recordScalar("total_flit_received", total_flit_received_);
  recordScalar("total_packet_received", total_packet_received_);
  recordScalar("avg_latency", avg_latency_);
}

inline std::string Node::get_id() {
  char name[20];
  sprintf(name, "(node_%d)", getIndex());
  return std::string(name);
}

std::string Node::get_log(log_levels level, const std::string &msg) {
  if (log_lvl < level)
    return "";
  std::string lvl_type[4] = {"CRI", "WARN", "INFO", "DBG"};
  std::string log_msg = lvl_type[uint8_t(level)] + "|at " +
                        std::to_string(omnetpp::simTime().dbl() * 1e9) +
                        "ns in " + get_id() + ", ";
  log_msg += msg + '\n';
  return log_msg;
}
