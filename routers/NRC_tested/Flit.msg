packet Flit
{
  int flitCount; // how many flit in this packet
  int srcId;
  int destId;
  bool isHead;
  bool isTail;
  // If in link, vcid is the next router input vcid;
  // If in router and before RC, vcid is the current router input vcid;
  // If in router and after RC, vcid is the output vcid,
  // say, input vcid of next hop router.
  int vcid;
  // If in link, port is the next router input port;
  // If in router and before RC, vcid is the current router input port;
  // If in router and after RC, vcid is the output port,
  // say, input port of next hop router.
  int port;
  int hopCount;
  double sendTime;
}
