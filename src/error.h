#ifndef AGC_ERROR_H
#define AGC_ERROR_H
#include <stdio.h>

#define AGC_ERROR_ENUM(X)                                                                          \
	X(AGC_OK, "Success")                                                                       \
	X(AGC_ERR_MEMORY, "Memory issue")                                                          \
	X(AGC_ERR_NULL, "Null pointer")                                                            \
	X(AGC_ERR_OOB, "Out of bounds")                                                            \
	X(AGC_ERR_NOT_FOUND, "Not found")                                                          \
	X(AGC_ERR_EXISTS, "Already exists")                                                        \
	X(AGC_ERR_INVALID, "Invalid argument")                                                     \
	X(AGC_ERR_CALLBACK, "Callback error")

#define AGC_ERROR_ENUM_DECLARE(E, MSG) E,

typedef enum
{
	AGC_ERROR_ENUM(AGC_ERROR_ENUM_DECLARE)
} agc_err_t;

char const *
agc_error_str(agc_err_t e);
void
agc_write_error(agc_err_t e, FILE *f);

#endif // !AGC_ERROR_H
