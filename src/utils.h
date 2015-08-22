#ifndef UTILS_H
#define UTILS_H

#include <string.h>

#define fatal() \
	do { fprintf("%s: %s: %d: %s: %s\n", \
		tokens[0], __FILE__, __LINE__, \
		__PRETTY_FUNCTION__, strerror(errno)); \
		exit(EXIT_FAILURE);} while(0)
//fazer uma funcao de warning

#endif /* UTILS_H */
