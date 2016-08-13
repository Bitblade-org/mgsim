#ifndef OLD_SYSCALLMSG_H_
#define OLD_SYSCALLMSG_H_

#define MAX_PARTS 2

#ifdef __cplusplus
#	include <cstdint>
#	define MSG_ENUM(name) enum class name : uint64_t
#	define MSG_UNION(name, contents) union name{contents};
#else
#	include <stddef.h>
#	include <stdint.h>
#	define MSG_ENUM(name) enum name
#	define MSG_UNION(name, contents) typedef union {contents}name ## _t;
#endif

MSG_ENUM(SysCall_types){
	SC_NO_MSG		= 0,
	SC_ABORT 		= 1,
	SC_INVALIDATE 	= 2,
};

struct SysCall_data{
	uint64_t	part[MAX_PARTS];
};

struct SysCall_Header{
	char		type			: 3;
	char		_padding		: 2;
	char		parts			: 3;
};

struct SysCall_InvalidateRequest{
	uint64_t	header			: 8; //8
	uint64_t	filterContext	: 1; //9
	uint64_t	filterVAddr 	: 1; //10
	uint64_t	contextId		:16; //26	MLDTODO Find a better way
	uint64_t	reserved		:22; //48
	uint64_t	caller			:16; //64	MLDTODO Find a better way
	uint64_t	vAddr;			 	 //128
};

struct SysCall_AbortRequest{
	uint64_t	header			: 8; //8
};

struct SysCall_Response{
	uint64_t	header			: 8; //8
	uint64_t	responseCode	: 8; //16
	uint64_t	_padding		:48; //64
};

MSG_UNION(SysCallMsg,
	uint64_t		 					type:3;
	struct SysCall_Header				header;
	struct SysCall_InvalidateRequest	invalidateRq;
	struct SysCall_AbortRequest			abortRq;
	struct SysCall_data					data;
)

#undef MSG_ENUM
#undef MSG_UNION

#endif /* OLD_SYSCALLMSG_H_ */
