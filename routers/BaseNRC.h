#pragma once

#include <deque>
#include <omnetpp.h>
#include <string>

#include "../th2/credit_m.h"
#include "../th2/flit_m.h"
#include "../th2/macros.h"
#include "../th2/sim.h"

class BaseNRC : public omnetpp::cSimpleModule {
public:
  BaseNRC();
  ~BaseNRC();

protected:
  void initialize() override;
  void handleMessage(omnetpp::cMessage *msg) override;
  void finish() override;
  void timer_cb();
  void route_compute();
  void row_move();
  void first_virtual_channel_allocation();
  void switch_allocation();
  void switch_traversal();
  void second_virtual_channel_allocation();
  void port_allocation();
  void forward_flit();
  void dump_colbuf();

  virtual int32_t get_curr_level() = 0;
  virtual int32_t get_routing_port(flit *f) = 0;
  virtual int32_t get_next_port(int32_t port) = 0;
  virtual bool connect_to_node(int32_t port) = 0;
  virtual int32_t get_node_num() = 0;

  void flit_cb(omnetpp::cMessage *msg);

  int32_t get_best_vcid(int32_t port);
  omnetpp::simtime_t get_channel_available_time(int32_t port);
  std::string get_id();
  std::string get_log(log_levels level, const std::string &msg);

private:
  omnetpp::cMessage *self_timer_ = nullptr;
  std::deque<flit *> inbuf_[P][V];
  int32_t routing_result_[P][V];
  bool vca1_state_[H][W][W][H][V];
  int32_t vca1_result_[H][W][H][V];
  int32_t vca1_result_prev_[H][W][H][V];
  std::deque<flit *> swtbuf_[H][W][W][H][V];
  int32_t sa1_result_[H][W][H];
  std::deque<flit *> colbuf_[H][W][H][V];
  int32_t vca2_port_[H][W][V];
  int32_t vca2_port_prev_[H][W][V];
  int32_t sa2_vcid_[P];
  int32_t credit_[P][V];
  std::deque<credit *> credit_queue_[P];
};
