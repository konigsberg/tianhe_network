#pragma once

#include <deque>
#include <omnetpp.h>

#include "../macros.h"
#include "Flit_m.h"
#include "FreeMsg_m.h"

class NRC : public omnetpp::cSimpleModule {
public:
  NRC();
  ~NRC();

protected:
  void initialize() override;
  void handleMessage(omnetpp::cMessage *msg) override;
  void finish() override;
  void handle_timer();
  void route_compute();
  void row_move();
  void first_virtual_channel_allocation();
  void switch_allocation();
  void switch_traversal();
  void second_virtual_channel_allocation();
  void port_allocation();
  void send_free_msg();
  void pipe();
  void forward_flit();

  virtual int32_t get_curr_level() = 0;
  virtual int32_t get_routing_port(Flit *flit) = 0;
  virtual int32_t get_next_port(int32_t port) = 0;
  virtual bool connect_to_node(int32_t port) = 0;
  virtual int32_t get_node_num() = 0;

  void handle_free_msg(omnetpp::cMessage *msg);
  void handle_flit(omnetpp::cMessage *msg);

  int32_t get_best_vcid(int32_t port);
  omnetpp::simtime_t get_channel_available_time(int32_t port);
  std::string get_id();
  std::string get_log(log_levels level, const std::string &msg);

private:
  omnetpp::cMessage *self_timer_ = nullptr;
  std::deque<Flit *> input_buffer_[P][V];
  int32_t routing_result_[P][V];
  bool vca1_state_[H][W][W][H][V];
  int32_t vca1_result_[H][W][H][V];
  int32_t vca1_result_prev_[H][W][H][V];
  std::deque<Flit *> switch_buffer_[H][W][W][H][V];
  int32_t sa1_result_[H][W][H];
  std::deque<Flit *> column_buffer_[H][W][H][V];
  bool vca2_state_[H][W][H][V];
  int32_t vca2_port_[H][W][V];
  int32_t vca2_port_prev_[H][W][V];
  int32_t sa2_vcid_[P];
  std::deque<FreeMsg *> free_msg_queue_[P];
  int32_t credit_[P][V];
  Flit *forward_queue_[P][PipelineDepth - 1];
};
