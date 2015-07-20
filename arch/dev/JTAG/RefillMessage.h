#ifndef ARCH_DEV_JTAG_REFILLMESSAGE_H_
#define ARCH_DEV_JTAG_REFILLMESSAGE_H_


struct tlbRefill_D {
	uint64_t dTLB		: 1; // 1	 (must be 1)
	uint64_t present	: 1; // 2
	uint64_t table		: 4; // 6    (needed + 1)
	uint64_t read		: 1; // 7
	uint64_t write		: 1; // 8
	uint64_t lineIndex  :16; // 24   (needed + 4)
	uint64_t pAddr		:40; // 64
};

struct tlbRefill_I {
	uint64_t dTLB		: 1; // 1	 (must be 0)
	uint64_t present	: 1; // 2
	uint64_t table		: 4; // 6    (needed + 1)
	uint64_t execute	: 1; // 7
	uint64_t _padding	: 1; // 8
	uint64_t lineIndex  :16; // 24   (needed + 4)
	uint64_t pAddr		:40; // 64
};

struct tlbRefill_base {
	uint64_t dTLB		: 1; // 1	 (must be 0)
	uint64_t present	: 1; // 2
	uint64_t table		: 4; // 6    (needed + 1)
	uint64_t _padding	: 2; // 8
	uint64_t lineIndex  :16; // 24   (needed + 4)
	uint64_t pAddr		:40; // 64
};

typedef union{
	struct tlbRefill_base base;
	struct tlbRefill_D d;
	struct tlbRefill_I i;
} tlbRefillMsg;


#endif /* ARCH_DEV_JTAG_REFILLMESSAGE_H_ */
