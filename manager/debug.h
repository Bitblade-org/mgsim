#ifndef MANAGER_DEBUG_H_
#define MANAGER_DEBUG_H_

#define MANAGER_DEBUG 0

#if MANAGER_DEBUG == 1
#	include <svp/testoutput.h>
#	define PRINT_STRING(s) 	output_string(s, 2)
#	define PRINT_HEX(i)		output_hex(i, 2)
#	define PRINT_UINT(i)	output_uint(i, 2)
#	define PRINT_INT(i)		output_int(i, 2)
#	define PRINT_CHAR(c)	output_char(c, 2)
#else
#	define PRINT_STRING(s)
#	define PRINT_HEX(i)
#	define PRINT_UINT(i)
#	define PRINT_INT(i)
#	define PRINT_CHAR(c)
#endif

#endif /* MANAGER_DEBUG_H_ */
