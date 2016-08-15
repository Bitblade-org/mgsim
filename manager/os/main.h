#ifndef MANAGER_MAIN_H_
#define MANAGER_MAIN_H_

#include <stddef.h>
#include <stdint.h>

#include "../ioLib/IO.h"

#define PTS_PBASE ((void*)0x420000) //2MiB aligned = 4KiB aligned
#define PTS_VBASE ((void*)0x420000) //2MiB aligned = 4KiB aligned
#define OS_CONTEXT_ID 0
#define MANAGER_CHANNEL 1


sl_decl(getRemoteIOAddr,,
		sl_shparm(unsigned, addr)
		);


int main(void);
void init_manager(void);
void init_memreader(void);



#endif /* MANAGER_MAIN_H_ */
