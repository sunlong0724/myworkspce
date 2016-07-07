/**
 * Autogenerated by Thrift Compiler (0.9.1)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef AcquireStoreService_TYPES_H
#define AcquireStoreService_TYPES_H

#include <thrift/Thrift.h>
#include <thrift/TApplicationException.h>
#include <thrift/protocol/TProtocol.h>
#include <thrift/transport/TTransport.h>

#include <thrift/cxxfunctional.h>


namespace hawkeye {

typedef struct _MinMaxStruct__isset {
  _MinMaxStruct__isset() : min(false), max(false) {}
  bool min;
  bool max;
} _MinMaxStruct__isset;

class MinMaxStruct {
 public:

  static const char* ascii_fingerprint; // = "EA2086D2BB14222991D7B0497DE7B58B";
  static const uint8_t binary_fingerprint[16]; // = {0xEA,0x20,0x86,0xD2,0xBB,0x14,0x22,0x29,0x91,0xD7,0xB0,0x49,0x7D,0xE7,0xB5,0x8B};

  MinMaxStruct() : min(0), max(0) {
  }

  virtual ~MinMaxStruct() throw() {}

  double min;
  double max;

  _MinMaxStruct__isset __isset;

  void __set_min(const double val) {
    min = val;
  }

  void __set_max(const double val) {
    max = val;
  }

  bool operator == (const MinMaxStruct & rhs) const
  {
    if (!(min == rhs.min))
      return false;
    if (!(max == rhs.max))
      return false;
    return true;
  }
  bool operator != (const MinMaxStruct &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const MinMaxStruct & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

void swap(MinMaxStruct &a, MinMaxStruct &b);

} // namespace

#endif
