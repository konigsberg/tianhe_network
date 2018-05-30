/*
 * NRC.h
 *
 *  Created on: Dec 23, 2017
 *      Author: Konigsberg
 */

#ifndef NRC_H_
#define NRC_H_

// libraries and STL
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <omnetpp.h>
#include <string>

// inherited class
// None

// network head file
#include "BufferFreeMsg_m.h"
#include "Flit_m.h"
#include "globalParam.h"

using namespace omnetpp;

using std::cout;
using std::deque;
using std::endl;
using std::string;
using std::to_string;

typedef std::deque<Flit *> FlitBuf;

class NRC : public cSimpleModule {

public:
  NRC();
  ~NRC();

protected:
  void initialize() override;
  void handleMessage(cMessage *msg) override;
  void finish() override;

  void handleTimer();

  void RC();
  void rowMove();
  void VA1();
  void SA1();
  void ST();
  void VA2();
  void SA2();
  void sendBufferFreeMsg();
  void pipe();
  void forwardFlit();

  virtual int routingPort(Flit *flit) = 0;
  virtual int nextPort(int port) = 0;
  virtual bool connectToNode(int port) = 0;

  void handleBufferFreeMsg(cMessage *msg);
  void handleFlit(cMessage *msg);

  int bestVcid(int port);
  simtime_t channelAvailTime(int port);
  string getId();

private:
  cMessage *selfTimer;

  deque<Flit *> inputBuffer[P][V];
  int RCResult[P][V];

  bool VA1State[H][W][W][H][V];
  int VA1Result[H][W][H][V];
  int VA1ResultPrev[H][W][H][V];

  deque<Flit *> switchBuffer[H][W][W][H][V];

  int SA1Result[H][W][H];

  deque<Flit *> columnBuffer[H][W][H][V];

  bool VA2State[H][W][H][V];
  int VA2Port[H][W][V];
  int VA2PortPrev[H][W][V];

  int SA2Vcid[P];

  deque<BufferFreeMsg *> bufferMsgQueue[P];

  int credit[P][V];

  Flit *forwardQueue[P][PipelineDepth - 1];
};

NRC::NRC() { selfTimer = nullptr; }

NRC::~NRC() {
  cancelAndDelete(selfTimer);

  // All flit buffers store dynamically-allocated Flit objects. So memory should
  // be freed using delete operator.
  for (int i = 0; i < P; i++) {
    while (!bufferMsgQueue[i].empty()) {
      delete bufferMsgQueue[i].front();
      bufferMsgQueue[i].pop_front();
    }

    for (int j = 0; j < PipelineDepth - 1; j++)
      if (forwardQueue[i][j] != nullptr)
        delete forwardQueue[i][j];

    for (int j = 0; j < V; j++)
      while (!inputBuffer[i][j].empty()) {
        delete inputBuffer[i][j].front();
        inputBuffer[i][j].pop_front();
      }
  }

  for (int i = 0; i < H; i++)
    for (int j = 0; j < W; j++)
      for (int k = 0; k < W; k++)
        for (int b = 0; b < H; b++)
          for (int v = 0; v < V; v++)
            while (!switchBuffer[i][j][k][b][v].empty()) {
              delete switchBuffer[i][j][k][b][v].front();
              switchBuffer[i][j][k][b][v].pop_front();
            }

  for (int i = 0; i < H; i++)
    for (int j = 0; j < W; j++)
      for (int k = 0; k < H; k++)
        for (int v = 0; v < V; v++)
          while (!columnBuffer[i][j][k][v].empty()) {
            delete columnBuffer[i][j][k][v].front();
            columnBuffer[i][j][k][v].pop_front();
          }
}

void NRC::initialize() {

  selfTimer = new cMessage("selfTimer");
  scheduleAt(SimStartTime, selfTimer);

  // initialize input tile
  for (int pi = 0; pi < P; pi++)
    for (int vi = 0; vi < V; vi++) {
      // RCResult[pi][vi] gives the RC result cache of queue-head flit
      // of inputBuffer[pi][vi]. Calculated by head-flit, and reset
      // by tail-flit.
      RCResult[pi][vi] = -1;
      credit[pi][vi] = InputBufferDepth;
    }

  // initialize VA1State
  for (int ti = 0; ti < H; ti++)
    for (int tj = 0; tj < W; tj++)
      for (int x = 0; x < W; x++)
        for (int y = 0; y < H; y++)
          for (int vi = 0; vi < V; vi++)
            VA1State[ti][tj][x][y][vi] = false;

  // initialize VA1Result and SA1Result
  for (int ti = 0; ti < H; ti++)
    for (int tj = 0; tj < W; tj++)
      for (int po = 0; po < H; po++) {
        for (int vo = 0; vo < V; vo++) {
          VA1Result[ti][tj][po][vo] = -1;
          VA1ResultPrev[ti][tj][po][vo] = -1;
        }
        SA1Result[ti][tj][po] = -1;
      }

  // initialize VA2State
  for (int ti = 0; ti < H; ti++)
    for (int tj = 0; tj < W; tj++)
      for (int pi = 0; pi < H; pi++)
        for (int vo = 0; vo < V; vo++)
          VA2State[ti][tj][pi][vo] = false;

  // initialize VA2Port
  for (int ti = 0; ti < H; ti++)
    for (int tj = 0; tj < W; tj++)
      for (int vo = 0; vo < V; vo++) {
        VA2Port[ti][tj][vo] = -1;
        VA2PortPrev[ti][tj][vo] = -1;
      }

  // initialize SA2Vcid
  for (int po = 0; po < P; po++)
    SA2Vcid[po] = -1;

  // initialize forwardQueue
  for (int po = 0; po < P; po++)
    for (int st = 0; st < PipelineDepth - 1; st++)
      forwardQueue[po][st] = nullptr;
}

void NRC::handleMessage(cMessage *msg) {
  if (msg->isSelfMessage()) { // only one selfTimer message for each router
    assert(msg == selfTimer);
    handleTimer();
  } else { // all BufferFreeMsg has name of "bufferFreeMsg"
    if (strcmp("bufferFreeMsg", msg->getName()) == 0)
      handleBufferFreeMsg(msg);
    else // Flit
      handleFlit(msg);
  }
}

void NRC::handleTimer() {
  scheduleAt(simTime() + ClockCycle, selfTimer);

  RC();
  rowMove();
  VA1();
  SA1();
  ST();
  VA2();
  SA2();
  sendBufferFreeMsg();
  pipe();
  forwardFlit();
}

/***************** Pipeline started *****************/

void NRC::RC() { // packet-wise
  // Register:
  // RCResult[P][V], saving the output port and vcid of current inputBuffer's
  // front flit.

  // For each queue, if queue-front is head-flit, calculate
  // output port and vcid, set them in flit, and save them
  // in RCResult.
  // If body-flit or tail-flit, take cache in RCResult and set them in flit.
  // Reset RCResult at tail-flit is necessary.
  for (int pi = 0; pi < P; pi++)
    for (int vi = 0; vi < V; vi++) {
      if (inputBuffer[pi][vi].empty())
        continue;

      Flit *flit = inputBuffer[pi][vi].front();

      if (flit->getIsHead() && RCResult[pi][vi] == -1) {
        int po = routingPort(flit);
        int vo = bestVcid(po); // vc of next-hop router having greatest credit

        flit->setPort(po);
        flit->setVcid(vo);
        RCResult[pi][vi] = po * V + vo;

        if (Verbose >= DETAIL)
          cout << "DETAIL: [RC] At time " << simTime() << " in " << getId()
               << ", flit " << flit->getName() << " routed to output vc(" << po
               << "," << vo << ")" << endl;

      } else { // use cached RC result
        if (RCResult[pi][vi] < 0) {
          cout << "ERROR! flit: " << flit->getName()
               << " should use a cached result, but the result is reset..."
               << endl;
          assert(false);
        }
        int po = RCResult[pi][vi] / V;
        int vo = RCResult[pi][vi] % V;

        flit->setPort(po);
        flit->setVcid(vo);

        if (Verbose >= DETAIL)
          cout << "DETAIL: [RC] At time " << simTime() << " in " << getId()
               << ", flit " << flit->getName() << " routed to output vc(" << po
               << "," << vo << ")" << endl;

        /*
        if (flit->getIsTail())
                RCResult[pi][vi] = -1;*/
      }
    }
}

void NRC::rowMove() { // flit-wise
  // move flit to corresponding rowBuffer of cross-point tile.
  // The destination port has been stored in flit itself.
  // Movement happens only if rowBuffer is not full.
  for (int pi = 0; pi < P; pi++)
    for (int vi = 0; vi < V; vi++) {
      if (inputBuffer[pi][vi].empty())
        continue;

      // from inputBuffer to switchBuffer
      FlitBuf &source = inputBuffer[pi][vi];
      Flit *flit = source.front();

      int po = flit->getPort();

      // Find cross-point tile, which locates at the same row with
      // input tile, and the same column of output tile
      int ti = pi / W; //
      int tj = po % W;

      int si = pi % W;
      int sj = po % H;

      // Flit should go to tile(ti,tj)'s switchBuffer(si, sj)
      // switchBuffer's vi corresponding to inputBuffer's vi
      FlitBuf &target = switchBuffer[ti][tj][si][sj][vi]; //

      if (target.size() == SwitchBufferDepth) { // ensure switchBuffer not full
        if (Verbose >= DETAIL)
          cout << "DETAIL: [rowMove] Warning: At time " << simTime() << " in "
               << getId() << ", switchBuffer is FULL!" << endl;
        continue;
      }

      // switchBuffer not full ensured

      if (flit->getIsTail())
        RCResult[pi][vi] = -1;

      target.push_back(flit);
      source.pop_front();

      if (Verbose >= DETAIL)
        cout << "DETAIL: [rowMove] At time " << simTime() << " in " << getId()
             << ", flit " << flit->getName() << " moved to switchBuffer(" << si
             << "," << sj << "," << vi << ") of tile(" << ti << "," << tj << ")"
             << endl;

      // inputBuffer releasd. Credit++ msg is sent to upstream router.
      BufferFreeMsg *bufferFreeMsg = new BufferFreeMsg("bufferFreeMsg");

      int port = nextPort(pi);
      bufferFreeMsg->setPort(port);
      bufferFreeMsg->setVcid(vi);

      bufferMsgQueue[pi].push_back(bufferFreeMsg);
    }
}

void NRC::VA1() { // packet-wise
  // Registers: VA1Result[H][W][H][V], represent output vc is allocated to which
  // input vcid. -1 for not allocated. The input vcid is numbered in a local
  // tile. The first three indices specify an output port. Actually, because
  // input flits have been stored in switchBuffer, it is just a virtual channel
  // matching. VA1State[H][W][W][H][V], related to every vc in each switchBuffer
  // at cross-point of sub-switch, represents whether this vc of flit has been
  // allocated an output vc.

  // VA executed at each sub-switch
  for (int ti = 0; ti < H; ti++)
    for (int tj = 0; tj < W; tj++)

      // Pay attention to area of port, 0~P, 0~W, 0~H
      // each sub-switch from now on
      for (int po = 0; po < H; po++)
        for (int vo = 0; vo < V; vo++) {
          if (VA1Result[ti][tj][po][vo] != -1)
            // Sub-switch out[po][vo] is assigned
            continue;

          // Find sub-switch out[po][vo] can assigned to
          // which in[pi][vi]. If matched, break

          int r = VA1ResultPrev[ti][tj][po][vo] + 1, count = 0;
          assert(r >= 0);

          // traverse all vc in this column
          for (; count < W * V; r++, count++) {
            int result = r % (W * V);
            int pi = result / V;
            int vi = result % V;

            FlitBuf &buffer = switchBuffer[ti][tj][pi][po][vi];
            if (buffer.empty()) // skip empty switchBuffers
              continue;

            Flit *flit = buffer.front();

            int flit_po = flit->getPort();
            int sub_po = flit_po % H; //

            // The flit didn't go to the wrong column of switchBuffer
            if (sub_po != po) {
              cout << "sub_po is " << sub_po << ", but po is " << po << endl;
              assert(false);
            }

            int sub_vo = flit->getVcid();

            if (sub_vo == vo) {
              assert(flit->getIsHead()); // Only head-flit needs VA.
              // VA1Result will be reset by tail flit
              VA1Result[ti][tj][po][vo] = result;

              // VA1ResultPrev won't be reset by tail flit
              VA1ResultPrev[ti][tj][po][vo] = result;

              VA1State[ti][tj][pi][po][vi] = true;

              if (Verbose >= DETAIL)
                cout << "DETAIL: [VA1] At time " << simTime() << " in "
                     << getId() << ", tile(" << ti << "," << tj
                     << ")'s output vc(" << po << "," << vo
                     << ") was assigned to input vc(" << pi << "," << vi << ")"
                     << endl;

              break;
            }
          }

          if (count == W * V) // should be -1, and never assigned
            assert(VA1Result[ti][tj][po][vo] == -1);
        }
  // The case that multiple out[po][vo] is assigned to the same in[pi][vi]
  // is impossible, because one flit can only go to a certain out[po][vo].
  // VA1Result and VA1State is reset in ST(),
  // VA1ResultPrev is not reset for round-robin.
}

void NRC::SA1() { // flit-wise
  // Registers: SA1Result[H][W][H], represent sub-switch port is allocated to
  // which vc in its column.

  for (int ti = 0; ti < H; ti++)
    for (int tj = 0; tj < W; tj++)

      // each sub-switch from now on
      for (int po = 0; po < H; po++) {
        // Determine sub-switch(ti, tj)'s outPort po should
        // assigned to which inPortcolumnBuffer

        int r = SA1Result[ti][tj][po] + 1, count = 0;
        for (; count < W * V; r++, count++) {
          int result = r % (W * V);
          int pi = result / V;
          int vi = result % V;
          if (!VA1State[ti][tj][pi][po][vi]) // lost in VA
            continue;

          // Last step, make sure columnBuffer has free space
          FlitBuf &source = switchBuffer[ti][tj][pi][po][vi];
          // TODO
          if (source.empty())
            continue;
          assert(!source.empty());

          Flit *flit = source.front();
          int flit_po = flit->getPort();
          int flit_vo = flit->getVcid();
          assert(flit_po % H == po);

          int dest_ti = flit_po / W;
          int dest_tj = flit_po % W;
          assert(dest_tj == tj);

          FlitBuf &target = columnBuffer[dest_ti][dest_tj][ti][flit_vo]; //

          if (target.size() == ColumnBufferDepth) {
            if (Verbose >= DETAIL)
              cout << "DETAIL: [SA1] Warning: At time " << simTime() << " in "
                   << getId() << ", columnBuffer is FULL!" << endl;
            continue;
          }

          // vc(pi,vi) won SA1
          SA1Result[ti][tj][po] = result;
          if (Verbose >= DETAIL)
            cout << "DETAIL: [SA1] At time " << simTime() << " in " << getId()
                 << ", tile(" << ti << "," << tj << ")'s output port " << po
                 << " was assigned to input vc(" << pi << "," << vi << ")"
                 << endl;
          break;
        }

        if (count == W * V) {
          SA1Result[ti][tj][po] = -1;
          if (Verbose >= DETAIL)
            cout << "DETAIL: [SA1] At time " << simTime() << " in " << getId()
                 << ", tile(" << ti << "," << tj << ")'s output port " << po
                 << " assigned failed" << endl;
        }
      }
}

void NRC::ST() { // flit-wise
  for (int ti = 0; ti < H; ti++)
    for (int tj = 0; tj < W; tj++)

      // each sub-switch from now on
      // registers must have free space, which is ensured by SA1.
      for (int po = 0; po < H; po++) {
        int result = SA1Result[ti][tj][po];

        if (result == -1) // No flit wanted go to po...
          continue;

        int pi = result / V;
        int vi = result % V;

        FlitBuf &source = switchBuffer[ti][tj][pi][po][vi];
        assert(!source.empty());
        assert(VA1State[ti][tj][pi][po][vi]);

        Flit *flit = source.front();

        int flit_po = flit->getPort();
        assert(flit_po % H == po);
        int flit_vo = flit->getVcid();
        assert(SA1Result[ti][tj][po] != -1);

        int dest_ti = flit_po / W;
        int dest_tj = flit_po % W;
        assert(dest_tj ==
               tj); // dest tile locates in the same row as switch tile

        FlitBuf &target = columnBuffer[dest_ti][dest_tj][ti][flit_vo];
        assert(target.size() < ColumnBufferDepth);

        target.push_back(flit);
        source.pop_front();

        if (Verbose >= DETAIL)
          cout << "DETAIL: [ST] At time " << simTime() << " in " << getId()
               << " in tile(" << ti << "," << tj << "), flit "
               << flit->getName() << " traversed from input port " << pi
               << " to output port " << po << ", and went to tile(" << dest_ti
               << "," << dest_tj << ")" << endl;

        if (flit->getIsTail()) {
          VA1State[ti][tj][pi][po][vi] = false;
          VA1Result[ti][tj][po][flit_vo] = -1;
        }
      }
}

void NRC::VA2() { // packet-wise
  for (int ti = 0; ti < H; ti++)
    for (int tj = 0; tj < W; tj++)

      // each tile from now on
      for (int vo = 0; vo < V; vo++) {

        if (VA2Port[ti][tj][vo] != -1)
          continue;

        int p = VA2PortPrev[ti][tj][vo] + 1, count = 0;

        for (; count < H; p++, count++) {
          int port = p % H;
          FlitBuf &buffer = columnBuffer[ti][tj][port][vo];

          if (buffer.empty()) {
            if (Verbose >= DETAIL)
              cout << "DETAIL: [VA2] At time " << simTime() << " in " << getId()
                   << ", columnBuffer[" << ti << "][" << tj << "][" << port
                   << "][" << vo << "] is empty" << endl;
            continue;
          }

          if (credit[ti * W + tj][vo] < PacketLength) {
            if (Verbose >= DETAIL)
              cout << "DETAIL: [VA2] Warning: At time " << simTime() << " in "
                   << getId() << ", next router's inputBuffer[" << ti * W + tj
                   << "][" << vo << "] is FULL" << endl;
            continue;
          }

          if (Verbose >= DETAIL)
            cout << "DETAIL: [VA2] At time " << simTime() << getId()
                 << ", columnBuffer[" << ti << "][" << tj << "][" << port
                 << "][" << vo << "] win." << endl;
          VA2Port[ti][tj][vo] = port;
          VA2PortPrev[ti][tj][vo] = port;
          break;
        }

        if (count == H) {
          assert(VA2Port[ti][tj][vo] == -1);
          if (Verbose >= DETAIL)
            cout << "DETAIL: [VA2] At time " << simTime() << " in " << getId()
                 << ", output port " << ti * W + tj << " vo " << vo
                 << " cannot be allocated to any columnBuffer because nobody"
                    " requests it"
                 << endl;
        }
      }
  /*
      for (int ti = 0; ti < H; ti++)
          for (int tj = 0; tj < W; tj++)
              for (int vo = 0; vo < V; vo++) {
                  cout << "ti:" << ti << ", tj:" << tj << ", vo:" << vo << ", "
                       << VA2Port[ti][tj][vo] << endl;
                  assert(VA2Port[ti][tj][vo] == -1 ||
                        !columnBuffer[ti][tj][VA2Port[ti][tj][vo]][vo].empty());
              }
              */
}

void NRC::SA2() { // flit-wise
  for (int po = 0; po < P; po++) {

    int v = SA2Vcid[po] + 1, count = 0;
    for (; count < V; v++, count++) {
      int vcid = v % V;
      int ti = po / W, tj = po % W;

      /*
      int vo = vcid;
      assert(VA2Port[ti][tj][vo] == -1 ||
                !columnBuffer[ti][tj][VA2Port[ti][tj][vo]][vo].empty());*/

      if (VA2Port[ti][tj][vcid] == -1)
        continue;

      // assert(!columnBuffer[ti][tj][VA2Port[ti][tj][vcid]][vcid].empty());
      assert(credit[po][vcid] != 0);
      SA2Vcid[po] = vcid;

      if (Verbose >= DETAIL)
        cout << "DETAIL: [SA2] At time " << simTime() << " in " << getId()
             << ", output port " << po << " is allocated to vo " << vcid
             << endl;
      break;
    }

    if (count == V) {
      SA2Vcid[po] = -1;
      if (Verbose >= DETAIL)
        cout << "DETAIL: [SA2] At time " << simTime() << " in " << getId()
             << ", output port " << po << " cannot be allocated to any vo"
             << endl;
    }
    /*
    int ti = po / W, tj = po % W;
    assert(SA2Vcid[po] == -1 ||
           !columnBuffer[ti][tj][VA2Port[ti][tj][SA2Vcid[po]]][SA2Vcid[po]].empty());
           */
  }
}

void NRC::sendBufferFreeMsg() {
  // send queue-front msg at every port
  for (int p = 0; p < P; p++) {
    // check queue
    if (bufferMsgQueue[p].empty())
      continue;

    // check channel
    if (channelAvailTime(p) > simTime())
      continue;

    BufferFreeMsg *bufferFreeMsg = bufferMsgQueue[p].front();

    char poCstr[20];
    sprintf(poCstr, "port_%d$o", p);
    send(bufferFreeMsg, poCstr);

    if (Verbose >= LOG)
      cout << "LOG: [sendBufferFreeMsg] At time " << simTime() << ", "
           << getId() << " sent BufferFreeMsg " << bufferFreeMsg->getName()
           << " through port " << p << endl;

    bufferMsgQueue[p].pop_front();
  }
}

void NRC::pipe() {
  int N = PipelineDepth - 1;

  for (int po = 0; po < P; po++) {
    if (forwardQueue[po][N - 1] != nullptr) {
      Flit *flitSent = forwardQueue[po][N - 1];
      int vi = flitSent->getVcid();

      char poCstr[22];
      sprintf(poCstr, "port_%d$o", po);
      assert(channelAvailTime(po) <= simTime());
      send(flitSent, poCstr);

      if (Verbose >= LOG)
        cout << "LOG: [pipe] At time " << simTime() << ", " << getId()
             << " forwarded flit " << flitSent->getName() << " through port "
             << po << " vo " << flitSent->getVcid() << endl;

      if (!connectToNode(po))
        --credit[po][vi];
    }

    for (int i = N - 1; i > 0; i--)
      forwardQueue[po][i] = forwardQueue[po][i - 1];
    forwardQueue[po][0] = nullptr;
  }
}

void NRC::forwardFlit() {
  for (int po = 0; po < P; po++) {

    int vcid = SA2Vcid[po];
    if (vcid == -1)
      continue;

    // result != -1 means credit is positive, send is ok
    int ti = po / W;
    int tj = po % W;
    int pi = VA2Port[ti][tj][vcid];

    FlitBuf &source = columnBuffer[ti][tj][pi][vcid];

    // TODO
    if (source.empty())
      continue;

    assert(!source.empty());
    Flit *flit = source.front();

    assert(po == flit->getPort());
    assert(vcid == flit->getVcid());

    flit->setHopCount(flit->getHopCount() + 1);

    int next_pi = nextPort(po);
    flit->setPort(next_pi);

    forwardQueue[po][0] = flit;
    source.pop_front();

    if (flit->getIsTail()) // reset VA2 Vcid
      VA2Port[ti][tj][vcid] = -1;
  }
}

/************ Pipeline terminated ***************/

void NRC::handleBufferFreeMsg(cMessage *msg) {

  if (Verbose >= LOG)
    cout << "LOG: At time " << simTime() << ", " << getId()
         << " received a BufferFreeMsg" << endl;

  BufferFreeMsg *bufferFreeMsg = check_and_cast<BufferFreeMsg *>(msg);
  int port = bufferFreeMsg->getPort();
  int vcid = bufferFreeMsg->getVcid();
  ++credit[port][vcid];

  delete bufferFreeMsg;
}

void NRC::handleFlit(cMessage *msg) {

  Flit *flit = check_and_cast<Flit *>(msg);
  int p = flit->getPort();
  int v = flit->getVcid();

  assert(inputBuffer[p][v].size() < InputBufferDepth);
  inputBuffer[p][v].push_back(flit);

  if (Verbose >= LOG)
    cout << "LOG: At time " << simTime() << ",_" << getId() << " received flit "
         << flit->getName() << ", saved in inputBuffer[" << p << "][" << v
         << "]" << endl;
}

int NRC::bestVcid(int port) {
  // Uncomment the following code block if there is more than 1 router with 24
  // nodes.
  int maxCredit = 0, maxVcid = 0;
  for (int v = 0; v < V; v++) {
    if (credit[port][v] > maxCredit) {
      maxCredit = credit[port][v];
      maxVcid = v;
    }
  }

  return maxVcid;

  // Comment this line if there is more than 1 router with 24 nodes.
  // return intuniform(0, V-1);
}

simtime_t NRC::channelAvailTime(int port) {
  char poCstr[20];
  sprintf(poCstr, "port_%d$o", port);
  cChannel *channel = gate(poCstr)->getTransmissionChannel();
  simtime_t finishTime = channel->getTransmissionFinishTime();
  return finishTime;
}

void NRC::finish() {}

string NRC::getId() {
  string name = par("name").stdstringValue();

  if (name != "")
    assert(name == "Leaf");
  else {
    assert(getParentModule() != nullptr);
    name = getParentModule()->par("name").stdstringValue();
  }

  string id = name == "Leaf" ? name + to_string(getIndex())
                             : name + to_string(getParentModule()->getIndex()) +
                                   "_nrc" + to_string(getIndex());

  return id;
}

#endif /* NRC_H_ */
