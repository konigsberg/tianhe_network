packet flit
{
  int src_id;
  int dest_id;
  bool is_head;
  bool is_tail;
  int vcid;
  int port;
  double send_time;
  int credit_os;
  int credit_port;
  int credit_vc;
}
