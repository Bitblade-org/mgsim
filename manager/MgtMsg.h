#ifndef MGTMSG_H_
#define MGTMSG_H_

#ifdef __cplusplus
#	include <cstdint>
#	define MGT_ENUM(name) enum class name : uint64_t
#	define MGT_UNION(name, contents) union name{contents};
	namespace manager{
#else
#	include <stddef.h>
#	define MGT_ENUM(name) enum name
#	define MGT_UNION(name, contents) typedef union {contents}name ## _t;
#endif

MGT_ENUM(MgtMsgType){
	NO_MSG	= 0,
	MISS 	= 1,
	INV_RQ 	= 2,
	REFILL 	= 3,
	INV		= 4,
	SET_PT  = 5
};

MGT_ENUM(TlbType){
	ITLB	= 0,
	DTLB	= 1
};

//C, C++
struct MgtMsgData{
	uint64_t	part[2];
};

//C, C++
struct InvalidateRequest{
	uint64_t	type			: 3; //3
	uint64_t	filterContext	: 1; //21
	uint64_t	filterVAddr 	: 1; //22
	uint64_t	contextId		:16; //38
	uint64_t	reserved		:27; //48
	uint64_t	caller			:16; //64
	uint64_t	vAddr;			 	 //128
};

//C, C++
struct MissRequest{
	uint64_t	type		: 3; // 3
	uint64_t	lineIndex	:24; //27 - Needs to be at same loc in MissR and TLBRef
	uint64_t	tlbType		: 1; //28 - Needs to be at the same loc as TLBRef.present
	uint64_t	contextId	:16; //44
	uint64_t	reserved	: 4; //48
	uint64_t	caller		:16; //64
	uint64_t	vAddr; 			 //128
};

//C, C++
struct TlbRefill_D {
	uint64_t type		: 3;
	uint64_t lineIndex 	:24; //- Needs to be at same loc in MissR and TLBRef
	uint64_t present	: 1; // Needs to be at the same place as MissR.tlbType
	uint64_t table		: 8;
	uint64_t read		: 1;
	uint64_t write		: 1;
	uint64_t ignored	:26;
	uint64_t pAddr;
};

//C, C++
struct TlbRefill_I {
	uint64_t type		: 3; //  3
	uint64_t lineIndex  :24; // 27 - Needs to be at same loc in MissR and TLBRef
	uint64_t present	: 1; // 28 - Needs to be at the same place as MissR.tlbType
	uint64_t table		: 8; // 36
	uint64_t execute	: 1; // 37
	uint64_t _padding	: 1; // 38
	uint64_t ignored    :26; // 64
	uint64_t pAddr;			 //128
};

//C, C++
struct TlbRefill {
	uint64_t type		: 3; //  3
	uint64_t lineIndex  :24; // 27 - Needs to be at same loc in MissR and TLBRef
	uint64_t present	: 1; // 28 - Needs to be at the same place as MissR.tlbType
	uint64_t table		: 8; // 36
	uint64_t _padding	: 2; // 28
	uint64_t ignored    :26; // 64
	uint64_t pAddr;			 //128
};

struct SetPT {
	uint64_t type		: 3; //  3
	uint64_t reserved	:61; // 64
	uint64_t pointer;		 //128
};

MGT_UNION(MgtMsg,
	uint64_t		 			type:3;
	struct MgtMsgData			data;
	struct MissRequest			mReq;
	struct InvalidateRequest	iReq;
	struct TlbRefill			refill;
	struct TlbRefill_I			iRefill;
	struct TlbRefill_D			dRefill;
	struct SetPT				setPT;
)

#undef MGT_ENUM
#undef MGT_UNION
#ifdef __cplusplus
} /* namespace manager */
#endif

#endif /* MGTMSG_H_ */
