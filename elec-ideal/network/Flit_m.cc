//
// Generated file, do not edit! Created by nedtool 5.1 from Flit.msg.
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
#include "Flit_m.h"

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

Register_Class(Flit)

Flit::Flit(const char *name, short kind) : ::omnetpp::cPacket(name,kind)
{
    this->flitCount = 0;
    this->srcId = 0;
    this->destId = 0;
    this->isHead = false;
    this->isTail = false;
    this->vcid = 0;
    this->port = 0;
    this->hopCount = 0;
    this->sendTime = 0;
}

Flit::Flit(const Flit& other) : ::omnetpp::cPacket(other)
{
    copy(other);
}

Flit::~Flit()
{
}

Flit& Flit::operator=(const Flit& other)
{
    if (this==&other) return *this;
    ::omnetpp::cPacket::operator=(other);
    copy(other);
    return *this;
}

void Flit::copy(const Flit& other)
{
    this->flitCount = other.flitCount;
    this->srcId = other.srcId;
    this->destId = other.destId;
    this->isHead = other.isHead;
    this->isTail = other.isTail;
    this->vcid = other.vcid;
    this->port = other.port;
    this->hopCount = other.hopCount;
    this->sendTime = other.sendTime;
}

void Flit::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cPacket::parsimPack(b);
    doParsimPacking(b,this->flitCount);
    doParsimPacking(b,this->srcId);
    doParsimPacking(b,this->destId);
    doParsimPacking(b,this->isHead);
    doParsimPacking(b,this->isTail);
    doParsimPacking(b,this->vcid);
    doParsimPacking(b,this->port);
    doParsimPacking(b,this->hopCount);
    doParsimPacking(b,this->sendTime);
}

void Flit::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->flitCount);
    doParsimUnpacking(b,this->srcId);
    doParsimUnpacking(b,this->destId);
    doParsimUnpacking(b,this->isHead);
    doParsimUnpacking(b,this->isTail);
    doParsimUnpacking(b,this->vcid);
    doParsimUnpacking(b,this->port);
    doParsimUnpacking(b,this->hopCount);
    doParsimUnpacking(b,this->sendTime);
}

int Flit::getFlitCount() const
{
    return this->flitCount;
}

void Flit::setFlitCount(int flitCount)
{
    this->flitCount = flitCount;
}

int Flit::getSrcId() const
{
    return this->srcId;
}

void Flit::setSrcId(int srcId)
{
    this->srcId = srcId;
}

int Flit::getDestId() const
{
    return this->destId;
}

void Flit::setDestId(int destId)
{
    this->destId = destId;
}

bool Flit::getIsHead() const
{
    return this->isHead;
}

void Flit::setIsHead(bool isHead)
{
    this->isHead = isHead;
}

bool Flit::getIsTail() const
{
    return this->isTail;
}

void Flit::setIsTail(bool isTail)
{
    this->isTail = isTail;
}

int Flit::getVcid() const
{
    return this->vcid;
}

void Flit::setVcid(int vcid)
{
    this->vcid = vcid;
}

int Flit::getPort() const
{
    return this->port;
}

void Flit::setPort(int port)
{
    this->port = port;
}

int Flit::getHopCount() const
{
    return this->hopCount;
}

void Flit::setHopCount(int hopCount)
{
    this->hopCount = hopCount;
}

double Flit::getSendTime() const
{
    return this->sendTime;
}

void Flit::setSendTime(double sendTime)
{
    this->sendTime = sendTime;
}

class FlitDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
  public:
    FlitDescriptor();
    virtual ~FlitDescriptor();

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

Register_ClassDescriptor(FlitDescriptor)

FlitDescriptor::FlitDescriptor() : omnetpp::cClassDescriptor("Flit", "omnetpp::cPacket")
{
    propertynames = nullptr;
}

FlitDescriptor::~FlitDescriptor()
{
    delete[] propertynames;
}

bool FlitDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<Flit *>(obj)!=nullptr;
}

const char **FlitDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *FlitDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int FlitDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 9+basedesc->getFieldCount() : 9;
}

unsigned int FlitDescriptor::getFieldTypeFlags(int field) const
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

const char *FlitDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "flitCount",
        "srcId",
        "destId",
        "isHead",
        "isTail",
        "vcid",
        "port",
        "hopCount",
        "sendTime",
    };
    return (field>=0 && field<9) ? fieldNames[field] : nullptr;
}

int FlitDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0]=='f' && strcmp(fieldName, "flitCount")==0) return base+0;
    if (fieldName[0]=='s' && strcmp(fieldName, "srcId")==0) return base+1;
    if (fieldName[0]=='d' && strcmp(fieldName, "destId")==0) return base+2;
    if (fieldName[0]=='i' && strcmp(fieldName, "isHead")==0) return base+3;
    if (fieldName[0]=='i' && strcmp(fieldName, "isTail")==0) return base+4;
    if (fieldName[0]=='v' && strcmp(fieldName, "vcid")==0) return base+5;
    if (fieldName[0]=='p' && strcmp(fieldName, "port")==0) return base+6;
    if (fieldName[0]=='h' && strcmp(fieldName, "hopCount")==0) return base+7;
    if (fieldName[0]=='s' && strcmp(fieldName, "sendTime")==0) return base+8;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *FlitDescriptor::getFieldTypeString(int field) const
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

const char **FlitDescriptor::getFieldPropertyNames(int field) const
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

const char *FlitDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int FlitDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    Flit *pp = (Flit *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *FlitDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    Flit *pp = (Flit *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string FlitDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    Flit *pp = (Flit *)object; (void)pp;
    switch (field) {
        case 0: return long2string(pp->getFlitCount());
        case 1: return long2string(pp->getSrcId());
        case 2: return long2string(pp->getDestId());
        case 3: return bool2string(pp->getIsHead());
        case 4: return bool2string(pp->getIsTail());
        case 5: return long2string(pp->getVcid());
        case 6: return long2string(pp->getPort());
        case 7: return long2string(pp->getHopCount());
        case 8: return double2string(pp->getSendTime());
        default: return "";
    }
}

bool FlitDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    Flit *pp = (Flit *)object; (void)pp;
    switch (field) {
        case 0: pp->setFlitCount(string2long(value)); return true;
        case 1: pp->setSrcId(string2long(value)); return true;
        case 2: pp->setDestId(string2long(value)); return true;
        case 3: pp->setIsHead(string2bool(value)); return true;
        case 4: pp->setIsTail(string2bool(value)); return true;
        case 5: pp->setVcid(string2long(value)); return true;
        case 6: pp->setPort(string2long(value)); return true;
        case 7: pp->setHopCount(string2long(value)); return true;
        case 8: pp->setSendTime(string2double(value)); return true;
        default: return false;
    }
}

const char *FlitDescriptor::getFieldStructName(int field) const
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

void *FlitDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    Flit *pp = (Flit *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}


