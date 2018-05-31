//
// Generated file, do not edit! Created by nedtool 5.3 from flit.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wshadow"
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Wunused-parameter"
#  pragma clang diagnostic ignored "-Wc++98-compat"
#  pragma clang diagnostic ignored "-Wunreachable-code-break"
#  pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wsuggest-attribute=noreturn"
#  pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif

#include <iostream>
#include <sstream>
#include "flit_m.h"

namespace omnetpp {

// Template pack/unpack rules. They are declared *after* a1l type-specific pack functions for multiple reasons.
// They are in the omnetpp namespace, to allow them to be found by argument-dependent lookup via the cCommBuffer argument

// Packing/unpacking an std::vector
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::vector<T,A>& v)
{
    int n = v.size();
    doParsimPacking(buffer, n);
    for (int i = 0; i < n; i++)
        doParsimPacking(buffer, v[i]);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::vector<T,A>& v)
{
    int n;
    doParsimUnpacking(buffer, n);
    v.resize(n);
    for (int i = 0; i < n; i++)
        doParsimUnpacking(buffer, v[i]);
}

// Packing/unpacking an std::list
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::list<T,A>& l)
{
    doParsimPacking(buffer, (int)l.size());
    for (typename std::list<T,A>::const_iterator it = l.begin(); it != l.end(); ++it)
        doParsimPacking(buffer, (T&)*it);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::list<T,A>& l)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i=0; i<n; i++) {
        l.push_back(T());
        doParsimUnpacking(buffer, l.back());
    }
}

// Packing/unpacking an std::set
template<typename T, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::set<T,Tr,A>& s)
{
    doParsimPacking(buffer, (int)s.size());
    for (typename std::set<T,Tr,A>::const_iterator it = s.begin(); it != s.end(); ++it)
        doParsimPacking(buffer, *it);
}

template<typename T, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::set<T,Tr,A>& s)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i=0; i<n; i++) {
        T x;
        doParsimUnpacking(buffer, x);
        s.insert(x);
    }
}

// Packing/unpacking an std::map
template<typename K, typename V, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::map<K,V,Tr,A>& m)
{
    doParsimPacking(buffer, (int)m.size());
    for (typename std::map<K,V,Tr,A>::const_iterator it = m.begin(); it != m.end(); ++it) {
        doParsimPacking(buffer, it->first);
        doParsimPacking(buffer, it->second);
    }
}

template<typename K, typename V, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::map<K,V,Tr,A>& m)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i=0; i<n; i++) {
        K k; V v;
        doParsimUnpacking(buffer, k);
        doParsimUnpacking(buffer, v);
        m[k] = v;
    }
}

// Default pack/unpack function for arrays
template<typename T>
void doParsimArrayPacking(omnetpp::cCommBuffer *b, const T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimPacking(b, t[i]);
}

template<typename T>
void doParsimArrayUnpacking(omnetpp::cCommBuffer *b, T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimUnpacking(b, t[i]);
}

// Default rule to prevent compiler from choosing base class' doParsimPacking() function
template<typename T>
void doParsimPacking(omnetpp::cCommBuffer *, const T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimPacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

template<typename T>
void doParsimUnpacking(omnetpp::cCommBuffer *, T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimUnpacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

}  // namespace omnetpp


// forward
template<typename T, typename A>
std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec);

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
inline std::ostream& operator<<(std::ostream& out,const T&) {return out;}

// operator<< for std::vector<T>
template<typename T, typename A>
inline std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec)
{
    out.put('{');
    for(typename std::vector<T,A>::const_iterator it = vec.begin(); it != vec.end(); ++it)
    {
        if (it != vec.begin()) {
            out.put(','); out.put(' ');
        }
        out << *it;
    }
    out.put('}');
    
    char buf[32];
    sprintf(buf, " (size=%u)", (unsigned int)vec.size());
    out.write(buf, strlen(buf));
    return out;
}

Register_Class(flit)

flit::flit(const char *name, short kind) : ::omnetpp::cPacket(name,kind)
{
    this->hop_count = 0;
    this->src_id = 0;
    this->dest_id = 0;
    this->is_head = false;
    this->is_tail = false;
    this->vcid = 0;
    this->port = 0;
    this->next_port = 0;
    this->send_time = 0;
}

flit::flit(const flit& other) : ::omnetpp::cPacket(other)
{
    copy(other);
}

flit::~flit()
{
}

flit& flit::operator=(const flit& other)
{
    if (this==&other) return *this;
    ::omnetpp::cPacket::operator=(other);
    copy(other);
    return *this;
}

void flit::copy(const flit& other)
{
    this->hop_count = other.hop_count;
    this->src_id = other.src_id;
    this->dest_id = other.dest_id;
    this->is_head = other.is_head;
    this->is_tail = other.is_tail;
    this->vcid = other.vcid;
    this->port = other.port;
    this->next_port = other.next_port;
    this->send_time = other.send_time;
}

void flit::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cPacket::parsimPack(b);
    doParsimPacking(b,this->hop_count);
    doParsimPacking(b,this->src_id);
    doParsimPacking(b,this->dest_id);
    doParsimPacking(b,this->is_head);
    doParsimPacking(b,this->is_tail);
    doParsimPacking(b,this->vcid);
    doParsimPacking(b,this->port);
    doParsimPacking(b,this->next_port);
    doParsimPacking(b,this->send_time);
}

void flit::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->hop_count);
    doParsimUnpacking(b,this->src_id);
    doParsimUnpacking(b,this->dest_id);
    doParsimUnpacking(b,this->is_head);
    doParsimUnpacking(b,this->is_tail);
    doParsimUnpacking(b,this->vcid);
    doParsimUnpacking(b,this->port);
    doParsimUnpacking(b,this->next_port);
    doParsimUnpacking(b,this->send_time);
}

int flit::getHop_count() const
{
    return this->hop_count;
}

void flit::setHop_count(int hop_count)
{
    this->hop_count = hop_count;
}

int flit::getSrc_id() const
{
    return this->src_id;
}

void flit::setSrc_id(int src_id)
{
    this->src_id = src_id;
}

int flit::getDest_id() const
{
    return this->dest_id;
}

void flit::setDest_id(int dest_id)
{
    this->dest_id = dest_id;
}

bool flit::getIs_head() const
{
    return this->is_head;
}

void flit::setIs_head(bool is_head)
{
    this->is_head = is_head;
}

bool flit::getIs_tail() const
{
    return this->is_tail;
}

void flit::setIs_tail(bool is_tail)
{
    this->is_tail = is_tail;
}

int flit::getVcid() const
{
    return this->vcid;
}

void flit::setVcid(int vcid)
{
    this->vcid = vcid;
}

int flit::getPort() const
{
    return this->port;
}

void flit::setPort(int port)
{
    this->port = port;
}

int flit::getNext_port() const
{
    return this->next_port;
}

void flit::setNext_port(int next_port)
{
    this->next_port = next_port;
}

double flit::getSend_time() const
{
    return this->send_time;
}

void flit::setSend_time(double send_time)
{
    this->send_time = send_time;
}

class flitDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
  public:
    flitDescriptor();
    virtual ~flitDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(flitDescriptor)

flitDescriptor::flitDescriptor() : omnetpp::cClassDescriptor("flit", "omnetpp::cPacket")
{
    propertynames = nullptr;
}

flitDescriptor::~flitDescriptor()
{
    delete[] propertynames;
}

bool flitDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<flit *>(obj)!=nullptr;
}

const char **flitDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *flitDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int flitDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 9+basedesc->getFieldCount() : 9;
}

unsigned int flitDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
    };
    return (field>=0 && field<9) ? fieldTypeFlags[field] : 0;
}

const char *flitDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "hop_count",
        "src_id",
        "dest_id",
        "is_head",
        "is_tail",
        "vcid",
        "port",
        "next_port",
        "send_time",
    };
    return (field>=0 && field<9) ? fieldNames[field] : nullptr;
}

int flitDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0]=='h' && strcmp(fieldName, "hop_count")==0) return base+0;
    if (fieldName[0]=='s' && strcmp(fieldName, "src_id")==0) return base+1;
    if (fieldName[0]=='d' && strcmp(fieldName, "dest_id")==0) return base+2;
    if (fieldName[0]=='i' && strcmp(fieldName, "is_head")==0) return base+3;
    if (fieldName[0]=='i' && strcmp(fieldName, "is_tail")==0) return base+4;
    if (fieldName[0]=='v' && strcmp(fieldName, "vcid")==0) return base+5;
    if (fieldName[0]=='p' && strcmp(fieldName, "port")==0) return base+6;
    if (fieldName[0]=='n' && strcmp(fieldName, "next_port")==0) return base+7;
    if (fieldName[0]=='s' && strcmp(fieldName, "send_time")==0) return base+8;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *flitDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",
        "int",
        "int",
        "bool",
        "bool",
        "int",
        "int",
        "int",
        "double",
    };
    return (field>=0 && field<9) ? fieldTypeStrings[field] : nullptr;
}

const char **flitDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *flitDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int flitDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    flit *pp = (flit *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *flitDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    flit *pp = (flit *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string flitDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    flit *pp = (flit *)object; (void)pp;
    switch (field) {
        case 0: return long2string(pp->getHop_count());
        case 1: return long2string(pp->getSrc_id());
        case 2: return long2string(pp->getDest_id());
        case 3: return bool2string(pp->getIs_head());
        case 4: return bool2string(pp->getIs_tail());
        case 5: return long2string(pp->getVcid());
        case 6: return long2string(pp->getPort());
        case 7: return long2string(pp->getNext_port());
        case 8: return double2string(pp->getSend_time());
        default: return "";
    }
}

bool flitDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    flit *pp = (flit *)object; (void)pp;
    switch (field) {
        case 0: pp->setHop_count(string2long(value)); return true;
        case 1: pp->setSrc_id(string2long(value)); return true;
        case 2: pp->setDest_id(string2long(value)); return true;
        case 3: pp->setIs_head(string2bool(value)); return true;
        case 4: pp->setIs_tail(string2bool(value)); return true;
        case 5: pp->setVcid(string2long(value)); return true;
        case 6: pp->setPort(string2long(value)); return true;
        case 7: pp->setNext_port(string2long(value)); return true;
        case 8: pp->setSend_time(string2double(value)); return true;
        default: return false;
    }
}

const char *flitDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

void *flitDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    flit *pp = (flit *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}


