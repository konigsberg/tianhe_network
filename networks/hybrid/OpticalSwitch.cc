#include <cassert>
#include <cstdint>
#include <omnetpp.h>

#include "../../th2/flit_m.h"
#include "../../th2/sim.h"

class OpticalSwitch : public omnetpp::cSimpleModule {
public:
  OpticalSwitch(){};
  ~OpticalSwitch(){};

protected:
  void handleMessage(omnetpp::cMessage *msg) {

    flit *f = omnetpp::check_and_cast<flit *>(msg);
    auto p = f->getPort();
    std::cerr << get_log(log_levels::info, std::string("received flit: ") +
                                               f->getName() + " from port " +
                                               std::to_string(p));

    auto switched_port = get_switched_port(p);
    // std::cerr << "next port in flit is " << f->getNext_port()
    // << ", switched port now is " << switched_port << std::endl;
    assert(is_matched(f->getNext_port(), switched_port));
    assert(get_channel_available_time(switched_port) <= omnetpp::simTime());
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

  omnetpp::simtime_t get_channel_available_time(int32_t port) {
    char poCstr[20];
    sprintf(poCstr, "port_%d$o", port);
    omnetpp::cChannel *channel = gate(poCstr)->getTransmissionChannel();
    omnetpp::simtime_t finishTime = channel->getTransmissionFinishTime();
    return finishTime;
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
