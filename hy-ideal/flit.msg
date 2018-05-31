packet flit
{
  int hop_count;
  int src_id;
  int dest_id;
  bool is_head;
  bool is_tail;
  int vcid;
  int port;
  int next_port; // for debug
  double send_time;
}
