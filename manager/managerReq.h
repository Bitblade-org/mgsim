#ifndef MANAGER_MANAGERREQ_H_
#define MANAGER_MANAGERREQ_H_

	struct mr_base{
		uint16_t 	isInvalidate: 1;
		uint16_t	free		: 1;
	};

	struct mr_miss{	// Will be 8-byte aligned, totals 16 bytes.
		uint16_t 	isInvalidate: 1; //Must be 0
		uint16_t	free		: 1;
		uint16_t	_padding1	: 13; //Must be 0
		uint16_t 	tlbType 	: 1;
		uint16_t	lineIndex;
		uint16_t 	processId;
		uint16_t 	dest;
		uint64_t	vAddr;
	};

	struct mr_inv{	// Will be 8-byte aligned, totals 16 bytes.
		uint16_t 	isInvalidate: 1; //Must be 1
		uint16_t	free		: 1;
		uint16_t	_padding1	: 4; //Must be 0
		uint16_t 	filterPid	: 1;
		uint16_t	filterVAddr : 1;
		uint16_t 	processId;
		uint32_t	_padding2;
		uint64_t  	vAddr;
	};

typedef union{
	struct mr_base base;
	struct mr_miss miss;
	struct mr_inv  inv;

} managerReq_t;


#endif /* MANAGER_MANAGERREQ_H_ */
