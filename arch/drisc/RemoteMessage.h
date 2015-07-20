// -*- c++ -*-
#ifndef ARCH_DRISC_REMOTEMESSAGE_P_H_
#define ARCH_DRISC_REMOTEMESSAGE_P_H_

#include <cstdint>
#include <string>

#include <sim/serialization.h>
#include <arch/Address.h>
#include <arch/simtypes.h>

namespace Simulator{
namespace drisc{

//MLDQUESTION Hoe zorg ik dat RemoteMessage.h gegenereert wordt?

	// 

	// RemoteMessage: delegation messages between cores
	// 
// ***** GENERATED CODE - DO NOT MODIFY  *****

struct RemoteMessage
{
 enum Type
 {
    MSG_NONE,
    MSG_TLB_MISS_MESSAGE,
    MSG_TLB_SET_PROPERTY,
    MSG_TLB_INVALIDATE,
    MSG_DTLB_STORE,
    MSG_ALLOCATE,
    MSG_SET_PROPERTY,
    MSG_CREATE,
    MSG_SYNC,
    MSG_DETACH,
    MSG_BREAK,
    MSG_RAW_REGISTER,
    MSG_FAM_REGISTER
 };
 Type type;

 union
 {
  struct {
      
   TlbType tlb;
   Addr lineIndex;
   Addr processId;
   PID dest;
   Addr addr;
      SERIALIZE(__a) {
    __a & "[3ea6";
     __a & tlb;
     __a & lineIndex;
     __a & processId;
     __a & dest;
     __a & addr;
  __a & "]";
}
  } TlbMissMessage;
  struct {
      
   TlbType tlb;
   Addr value;
   TlbPropertyMsgType type;
      SERIALIZE(__a) {
    __a & "[74f4";
     __a & tlb;
     __a & value;
     __a & type;
  __a & "]";
}
  } tlbProperty;
  struct {
      
   bool filterPid;
   Addr processId;
   bool filterAddr;
   Addr addr;
      SERIALIZE(__a) {
    __a & "[a89e";
     __a & filterPid;
     __a & processId;
     __a & filterAddr;
     __a & addr;
  __a & "]";
}
  } tlbInvalidate;
  struct {
      
   uint8_t table;
   Addr lineIndex;
   Addr pAddr;
   bool read;
   bool write;
      SERIALIZE(__a) {
    __a & "[2bfc";
     __a & table;
     __a & lineIndex;
     __a & pAddr;
     __a & read;
     __a & write;
  __a & "]";
}
  } dTlbStore;
  struct {
      
   PlaceID place;
   PID completion_pid;
   RegIndex completion_reg;
   AllocationType type;
   bool suspend;
   bool exclusive;
   bool bundle;
   MemAddr pc;
   Integer parameter;
   SInteger index;
      SERIALIZE(__a) {
    __a & "[96c0";
     __a & place;
     __a & completion_pid;
     __a & completion_reg;
     __a & type;
     __a & suspend;
     __a & exclusive;
     __a & bundle;
     __a & pc;
     __a & parameter;
     __a & index;
  __a & "]";
}
  } allocate;
  struct {
      
   Integer value;
   FID fid;
   FamilyProperty type;
      SERIALIZE(__a) {
    __a & "[86b4";
     __a & value;
     __a & fid;
     __a & type;
  __a & "]";
}
  } property;
  struct {
      
   MemAddr address;
   FID fid;
   PID completion_pid;
   RegIndex completion_reg;
   bool bundle;
   Integer parameter;
   SInteger index;
      SERIALIZE(__a) {
    __a & "[1a9c";
     __a & address;
     __a & fid;
     __a & completion_pid;
     __a & completion_reg;
     __a & bundle;
     __a & parameter;
     __a & index;
  __a & "]";
}
  } create;
  struct {
      
   FID fid;
   RegIndex completion_reg;
      SERIALIZE(__a) {
    __a & "[634b";
     __a & fid;
     __a & completion_reg;
  __a & "]";
}
  } sync;
  struct {
      
   FID fid;
      SERIALIZE(__a) {
    __a & "[6f86";
     __a & fid;
  __a & "]";
}
  } detach;
  struct {
      
   PID pid;
   LFID fid;
      SERIALIZE(__a) {
    __a & "[7667";
     __a & pid;
     __a & fid;
  __a & "]";
}
  } brk;
  struct {
      
   RegAddr addr;
   RegValue value;
   PID pid;
      SERIALIZE(__a) {
    __a & "[0db3";
     __a & pid;
  __a & Serialization::reg(addr, value);
  __a & "]";
}
  } rawreg;
  struct {
      
   FID fid;
   RemoteRegType kind;
   bool write;
   RegAddr addr;
   union {
       
   RegValue value;
   RegIndex completion_reg;
   };
      SERIALIZE(__a) {
    __a & "[df2a";
     __a & fid;
     __a & kind;
     __a & write;
     __a & addr;
  if (write) __a & Serialization::reg(addr, value);
		 else __a & addr & completion_reg;
  __a & "]";
}
  } famreg;
 };

 SERIALIZE(__a) {
  __a & type;
  switch(type) {
  case MSG_NONE:break;
  case MSG_TLB_MISS_MESSAGE:__a & TlbMissMessage;break;
  case MSG_TLB_SET_PROPERTY:__a & tlbProperty;break;
  case MSG_TLB_INVALIDATE:__a & tlbInvalidate;break;
  case MSG_DTLB_STORE:__a & dTlbStore;break;
  case MSG_ALLOCATE:__a & allocate;break;
  case MSG_SET_PROPERTY:__a & property;break;
  case MSG_CREATE:__a & create;break;
  case MSG_SYNC:__a & sync;break;
  case MSG_DETACH:__a & detach;break;
  case MSG_BREAK:__a & brk;break;
  case MSG_RAW_REGISTER:__a & rawreg;break;
  case MSG_FAM_REGISTER:__a & famreg;break;
  }
 }
     std::string str() const;
   
};
// ***** END GENERATED CODE *****


} /* namespace drisc */
} /* namespace Simulator */


#endif /* ARCH_DRISC_REMOTEMESSAGE_P_H_ */
