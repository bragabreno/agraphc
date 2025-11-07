#include <stdio.h>

#include "error.h"

#define ERROR_ENUM_TO_STRING(V, MSG)                                                               \
	case V:                                                                                    \
		return MSG;

char const *
agc_error_str(agc_err_t e)
{
	switch (e)
	{
		AGC_ERROR_ENUM(ERROR_ENUM_TO_STRING)
		default:
			return "Unexpected error value";
	}
}

void
agc_write_error(agc_err_t e, FILE *f)
{
	fprintf(f, "%s: %s\n", agc_error_str(e));
}
