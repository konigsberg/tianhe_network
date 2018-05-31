#pragma once

#include <deque>
#include <omnetpp.h>

#include "../credit_m.h"
#include "../flit_m.h"
#include "../macros.h"
#include "../sim.h"

class Node : public omnetpp::cSimpleModule {
public:
  Node();
  ~Node();

protected:
  void initialize() override;
  void handleMessage(omnetpp::cMessage *msg) override;
  void finish() override;

  void gen_timer_cb();
  void send_timer_cb();
  void flit_cb(omnetpp::cMessage *msg);
  void credit_cb(omnetpp::cMessage *msg);

  int32_t get_best_vcid();
  flit *flit_factory(bool is_head, bool is_tail, int32_t vcid, int32_t flit_id,
                     int32_t dest_id);
  int32_t get_dest_id();
  bool channel_is_available();
  omnetpp::simtime_t get_channel_available_time();
  std::string get_id();
  std::string get_log(log_levels level, const std::string &msg);

  virtual int32_t get_next_port() = 0;
  virtual int32_t get_node_num() = 0;

private:
  omnetpp::cMessage *gen_timer_{nullptr};
  omnetpp::cMessage *send_timer_{nullptr};
  std::deque<flit *> send_queue_;
  uint64_t total_flit_gen_{0};
  uint64_t total_flit_sent_{0};
  uint64_t total_packet_sent_{0};
  uint64_t total_packet_dropped_{0};
  uint64_t total_flit_received_{0};
  uint64_t total_packet_received_{0};
  double total_latency_{0};
  double avg_latency_{0};
  int32_t credit_[V];
};
