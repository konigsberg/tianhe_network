//
// Generated file, do not edit! Created by nedtool 5.3 from flit.msg.
//

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#ifndef __FLIT_M_H
#define __FLIT_M_H

#include <omnetpp.h>

// nedtool version check
#define MSGC_VERSION 0x0503
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif



/**
 * Class generated from <tt>flit.msg:2</tt> by nedtool.
 * <pre>
 * packet flit
 * {
 *     int hop_count;
 *     int src_id;
 *     int dest_id;
 *     bool is_head;
 *     bool is_tail;
 *     int vcid;
 *     int port;
 *     int next_port; // for debug
 *     double send_time;
 *     int credit_os;
 *     int credit_port;
 *     int credit_vc;
 * }
 * </pre>
 */
class flit : public ::omnetpp::cPacket
{
  protected:
    int hop_count;
    int src_id;
    int dest_id;
    bool is_head;
    bool is_tail;
    int vcid;
    int port;
    int next_port;
    double send_time;
    int credit_os;
    int credit_port;
    int credit_vc;

  private:
    void copy(const flit& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const flit&);

  public:
    flit(const char *name=nullptr, short kind=0);
    flit(const flit& other);
    virtual ~flit();
    flit& operator=(const flit& other);
    virtual flit *dup() const override {return new flit(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
    virtual int getHop_count() const;
    virtual void setHop_count(int hop_count);
    virtual int getSrc_id() const;
    virtual void setSrc_id(int src_id);
    virtual int getDest_id() const;
    virtual void setDest_id(int dest_id);
    virtual bool getIs_head() const;
    virtual void setIs_head(bool is_head);
    virtual bool getIs_tail() const;
    virtual void setIs_tail(bool is_tail);
    virtual int getVcid() const;
    virtual void setVcid(int vcid);
    virtual int getPort() const;
    virtual void setPort(int port);
    virtual int getNext_port() const;
    virtual void setNext_port(int next_port);
    virtual double getSend_time() const;
    virtual void setSend_time(double send_time);
    virtual int getCredit_os() const;
    virtual void setCredit_os(int credit_os);
    virtual int getCredit_port() const;
    virtual void setCredit_port(int credit_port);
    virtual int getCredit_vc() const;
    virtual void setCredit_vc(int credit_vc);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const flit& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, flit& obj) {obj.parsimUnpack(b);}


#endif // ifndef __FLIT_M_H

