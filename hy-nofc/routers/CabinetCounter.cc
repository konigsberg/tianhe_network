#include <cstdint>
#include <omnetpp.h>

#include "../exchange_m.h"
#include "../macros.h"

class CabinetCounter : public omnetpp::cSimpleModule {
public:
  CabinetCounter(){};
  ~CabinetCounter(){};

protected:
  void initialize() override {
    std::fill(remote_[0][0], remote_[0][0] + sizeof(remote_) / sizeof(int32_t),
              inbuf_capacity);
    std::fill(local_[0][0], local_[0][0] + sizeof(local_) / sizeof(int32_t),
              inbuf_capacity);
  }

  void handleMessage(omnetpp::cMessage *msg) {
    exchange *exc = omnetpp::check_and_cast<exchange *>(msg);
    auto type = exc->getType();
    auto os = exc->getOs();
    auto port = exc->getPort();
    auto vc = exc->getVc();

    int32_t *ptr = nullptr;
    if (type == local)
      ptr = &(local_[os][port][vc]);
    else
      ptr = &(remote_[os][port][vc]);
    *ptr += exc->getCredit();

    broadcast(type, os, port, vc, *ptr);
    delete exc;
  }

  void broadcast(int32_t type, int32_t os, int32_t port, int32_t vc,
                 int32_t credit) {
    for (auto po = 0; po < 36; po++) {
      exchange *exc = new exchange("exchange");
      exc->setType(type);
      exc->setOs(os);
      exc->setPort(port);
      exc->setVc(vc);
      exc->setCredit(credit);
      char po_cstr[20];
      sprintf(po_cstr, "port_%d$o", po);
      send(exc, po_cstr);
    }
  }

  int32_t remote_[36][12][V];
  int32_t local_[36][12][V];
};

Define_Module(CabinetCounter);
