#pragma once

#include <deque>
#include <omnetpp.h>
#include <string>
#include <vector>

#include "../credit_m.h"
#include "../exchange_m.h"
#include "../flit_m.h"
#include "../macros.h"
#include "../sim.h"

class UpperNRC : public omnetpp::cSimpleModule {
public:
  UpperNRC();
  ~UpperNRC();

protected:
  void initialize() override;
  void handleMessage(omnetpp::cMessage *msg) override;
  void finish() override;
  void timer_cb();
  void route_compute();
  void row_move();
  void send_credit();
  void first_virtual_channel_allocation();
  void switch_allocation();
  void switch_traversal();
  void upper_port_second_virtual_channel_allocation();
  void upper_port_allocation();
  void upper_port_forward_flit();

  void lower_port_select_packet();
  void lower_port_forward_packet(int32_t po, std::deque<flit *> &buffer);

  virtual int32_t get_curr_level() = 0;
  virtual int32_t get_routing_port(flit *f) = 0;
  virtual int32_t get_next_port(int32_t port) = 0;
  virtual bool connect_to_node(int32_t port) = 0;
  virtual int32_t get_node_num() = 0;
  virtual int32_t get_next_routing_port(flit *f) = 0;

  void flit_cb(omnetpp::cMessage *msg);
  void credit_cb(omnetpp::cMessage *msg);
  void exchange_cb(omnetpp::cMessage *msg);

  bool channel_is_available(int32_t port);
  std::string get_id();
  std::string get_log(log_levels level, const std::string &msg);

  int32_t get_switched_port(int32_t in_port);
  int32_t get_root_id();
  int32_t get_nrm_id();
  bool is_matched(int32_t request_po, int32_t switched_po);
  bool is_right_time();

  std::string credit_string(credit *cdt);
  std::string exchange_string(exchange *exc);

  void update_credit(int32_t type, int32_t os, int32_t port, int32_t vc,
                     int32_t credit);

  omnetpp::cMessage *self_timer_ = nullptr;
  std::deque<flit *> inbuf_[P][V];
  int32_t routing_result_[P][V];
  int32_t next_routing_result_[P][V];
  bool vca1_state_[H][W][W][H][V];
  int32_t vca1_result_[H][W][H][V];
  int32_t vca1_result_prev_[H][W][H][V];
  std::deque<flit *> swtbuf_[H][W][W][H][V];
  int32_t sa1_result_[H][W][H];
  std::deque<flit *> colbuf_[H][W][H][V];
  int32_t vca2_port_[H / 2][W][V];
  int32_t vca2_port_prev_[H / 2][W][V];
  int32_t sa2_vcid_[P];  // FIXME
  int32_t credit_[P][V]; // FIXME
  std::deque<credit *> credit_queue_[P];
  int32_t root_remote_credit_counter_[48][12][V];
  uint64_t clock_ = 0;
};
