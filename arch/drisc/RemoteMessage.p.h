// -*- c++ -*-
#ifndef ARCH_DRISC_REMOTEMESSAGE_P_H_
#define ARCH_DRISC_REMOTEMESSAGE_P_H_

namespace Simulator{
namespace drisc{

	// {% from "sim/macros.p.h" import gen_variant,gen_struct %}

	// RemoteMessage: delegation messages between cores
	// {% call gen_variant() %}
	((name RemoteMessage)
	 (variants
	  (MSG_NONE)

	  (MSG_TLB_MISS_MESSAGE TlbMissMessage
	   (state
		(TlbType    tlb)
		(Addr		lineIndex)
		(Addr    	processId)
		(PID        dest)
		(Addr		addr)
		   ))

	  (MSG_TLB_SET_PROPERTY tlbProperty
	   (state
		(TlbType			tlb)
		(Addr				value)
		(TlbPropertyMsgType	type)
		   ))

	  (MSG_TLB_INVALIDATE tlbInvalidate
	   (state
		(bool           filterPid)		//MLDTODO Enum
		(Addr          processId)
		(bool			filterAddr)
		(Addr			addr)
		   ))

	  (MSG_DTLB_STORE dTlbStore
	   (state
		(uint8_t        table)
		(Addr			lineIndex)
		(Addr			pAddr)
		(bool			read)
		(bool 			write)
		   ))

	  (MSG_ALLOCATE allocate
	   (state
		(PlaceID        place)           ///< The place to allocate at
		(PID            completion_pid)  ///< PID where the thread runs that issued the allocate
		(RegIndex       completion_reg)  ///< Register to write FID back to
		(AllocationType type)            ///< Type of the allocation
		(bool           suspend)         ///< Queue request if no context available?
		(bool           exclusive)       ///< Allocate the exclusive context?

		(bool           bundle)          ///< Is this allocation also bundling a create?
		(MemAddr        pc)              ///< Bundled program counter
		(Integer        parameter)       ///< Bundled program-specified parameter
		(SInteger       index)           ///< Bundled table-specified parameter
		   ))

	  (MSG_SET_PROPERTY property
	   (state
		(Integer        value)           ///< The new value of the property
		(FID            fid)             ///< Family to set the property of
		(FamilyProperty type)            ///< The property to set
		   ))

	  (MSG_CREATE create
	   (state
		(MemAddr  address)               ///< Address of the thread program
		(FID      fid)                   ///< Family to start creation of
		(PID      completion_pid)        ///< PID where the thread that issued the request is running
		(RegIndex completion_reg)        ///< Register to write create-completion to

		(bool     bundle)                ///< Whether this is a create resulting from a bundle
		(Integer  parameter)             ///< Bundled program-specified parameter
		(SInteger index)                 ///< Bundled table-specified parameter
		   ))

	  (MSG_SYNC sync
	   (state
		(FID      fid)                   ///< Family to sync on
		(RegIndex completion_reg)        ///< Register to write sync-completion to
		   ))

	  (MSG_DETACH detach
	   (state
		(FID fid)                        ///< Family to detach
		   ))

	  (MSG_BREAK brk
	   (state
		(PID   pid)                      ///< Core to send break to
		(LFID  fid)                      ///< Family to break
		   ))

	  (MSG_RAW_REGISTER rawreg
	   (state
		(RegAddr   addr noserialize)     ///< Register address to write to
		(RegValue  value noserialize)    ///< Value to write in register
		(PID       pid)                  ///< Processor where the register is
		   )
	   (serializer_append "__a & Serialization::reg(addr, value);"))

	  (MSG_FAM_REGISTER famreg
	   (state
		(FID           fid)              ///< Family hosting the register to read or write
		(RemoteRegType kind)             ///< Type of register (global, shared, local)
		(bool          write)            ///< Whether to read or write
		(RegAddr       addr)             ///< Address of register to access
		(union (state
				(RegValue value)            ///< Value in case of write
				(RegIndex completion_reg)   ///< Register to send value back to in case of read
			) noserialize))
	   (serializer_append
		"if (write) __a & Serialization::reg(addr, value);
		 else __a & addr & completion_reg;"))

		 )
	 (raw "std::string str() const;")
		)
	// {% endcall %}

} /* namespace drisc */
} /* namespace Simulator */


#endif /* ARCH_DRISC_REMOTEMESSAGE_P_H_ */
