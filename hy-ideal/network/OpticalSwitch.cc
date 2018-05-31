#include <cassert>
#include <cstdint>
#include <cstdio>
#include <omnetpp.h>

#include "../credit_m.h"
#include "../flit_m.h"
#include "../sim.h"

class OpticalSwitch : public omnetpp::cSimpleModule {
public:
  OpticalSwitch(){};
  ~OpticalSwitch(){};

protected:
  void handleMessage(omnetpp::cMessage *msg) {
    if (strcmp(msg->getName(), "credit_upper") == 0 ||
        strcmp(msg->getName(), "credit_lower") == 0)
      credit_cb(msg);
    else
      flit_cb(msg);
  }

  void flit_cb(omnetpp::cMessage *msg) {
    flit *f = omnetpp::check_and_cast<flit *>(msg);
    auto p = f->getPort();
    std::cerr << get_log(log_levels::info, std::string("received flit: ") +
                                               f->getName() + " from port " +
                                               std::to_string(p));

    auto switched_port = get_switched_port(p);
    assert(is_matched(f->getNext_port(), switched_port));
    assert(channel_is_available(switched_port));
    f->setHop_count(f->getHop_count() + 1);

    auto next_pi = get_next_port(switched_port);
    f->setPort(next_pi);

    char po_str[20];
    sprintf(po_str, "port_%d$o", switched_port);
    send(f, po_str);
    std::cerr << get_log(log_levels::info, std::string("forwarded flit: ") +
                                               f->getName() + " at port " +
                                               std::to_string(switched_port));
  }

  void credit_cb(omnetpp::cMessage *msg) {
    credit *cdt = omnetpp::check_and_cast<credit *>(msg);
    if (strcmp(cdt->getName(), "credit_lower") == 0) {
      for (auto po = P / 2; po < P; po++)
        if (channel_is_available(po)) {
          char po_cstr[20];
          sprintf(po_cstr, "port_%d$o", po);
          send(cdt, po_cstr);
          return;
        }
    }

    if (strcmp(cdt->getName(), "credit_upper") == 0) {
      for (auto po = 0; po < P / 2; po++)
        if (channel_is_available(po)) {
          char po_cstr[20];
          sprintf(po_cstr, "port_%d$o", po);
          send(cdt, po_cstr);
          return;
        }
    }

    std::cerr << "invalid credit received" << std::endl;
    assert(false);
  }

  bool channel_is_available(int32_t port) {
    char poCstr[20];
    sprintf(poCstr, "port_%d$o", port);
    omnetpp::cChannel *channel = gate(poCstr)->getTransmissionChannel();
    omnetpp::simtime_t finishTime = channel->getTransmissionFinishTime();
    return finishTime <= omnetpp::simTime();
  }

  int32_t get_switched_port(int32_t in_port) {
    auto clk = uint64_t(omnetpp::simTime().dbl() / period);
    return (clk % 24 + in_port) % 24;
  }

  std::string get_id() {
    char name[30];
    sprintf(name, "(OpticalSwitch_%d)", getIndex());
    return std::string(name);
  }

  std::string get_log(log_levels level, const std::string &msg) {
    if (log_lvl < level)
      return "";
    std::string lvl_type[4] = {"CRI", "WARN", "INFO", "DBG"};
    std::string log_msg = lvl_type[uint8_t(level)] + "|at " +
                          std::to_string(omnetpp::simTime().dbl() * 1e9) +
                          "ns in " + get_id() + ", ";
    log_msg += msg + '\n';
    return log_msg;
  }

  bool is_matched(int32_t request_po, int32_t switched_po) {
    if (request_po >= P / 2 && switched_po >= P / 2)
      return true;
    if (request_po == switched_po)
      return true;
    return false;
  }

  int32_t get_next_port(int32_t this_port) {
    if (this_port < 12)
      return (getIndex() % 36) % 12 + 12;
    return (getIndex() / 36) % 12;
  }
};

Define_Module(OpticalSwitch);
