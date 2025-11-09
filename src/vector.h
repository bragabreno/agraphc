#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "error.h"

#define AGC_VEC_API [[maybe_unused]] static

#ifndef AGC_VEC_GROWTH_FACTOR
	#define AGC_VEC_GROWTH_FACTOR 2
#endif

#ifndef AGC_VEC_DEFAULT_CAP
	#define AGC_VEC_DEFAULT_CAP 8
#endif

#define agc_vec_noop(ptr) ((void)0)

#ifndef AGC_VEC_PREFIX
	#error "You must define AGC_VEC_PREFIX prior to the inclusion of vector.h"
#endif
#ifndef T
	#error "You must define T prior to the inclusion of vector.h"
#endif

/* Vector interface */
#ifndef agc_vec_element_cleanup
	#define agc_vec_element_cleanup agc_vec_noop
#endif
#ifndef agc_vec_may_use_stack
	#define agc_vec_may_use_stack 1
#endif
#ifndef agc_vec_element_deepcopy
	#define agc_vec_element_deepcopy 0
#endif
#ifndef agc_vec_alloc
	#define agc_vec_alloc malloc
#endif
#ifndef agc_vec_realloc
	#define agc_vec_realloc realloc
#endif
#ifndef agc_vec_free
	#define agc_vec_free free
#endif

#define agc_vec_t agc_paste2(AGC_VEC_PREFIX, _t)
#define agc_vec_fn(name) agc_paste3(AGC_VEC_PREFIX, _, name)

#define agc_vec_empty(vec) ((vec)->len == 0)

#define agc_vec_len(vec) ((vec)->len)
#define agc_vec_cap(vec) ((vec)->cap)

#define agc_vec_beg(vec) ((vec)->buf)
#define agc_vec_end(vec) ((vec)->buf + (vec)->len)

#define agc_vec_at(vec, pos) ((vec)->buf[pos])
#define agc_vec_ptr_at(vec, pos) ((vec)->buf + (pos))

#define agc_vec_front(vec) ((vec)->buf[0])
#define agc_vec_back(vec) ((vec)->buf[(vec)->len - 1])

typedef struct agc_vec_t
{
	int32_t len;
	int32_t cap;
#if agc_vec_may_use_stack
	bool stack_buf;
#endif
	T *buf;
} agc_vec_t;

#define agc_vec_foreach_do(vec, func)                                                              \
	do                                                                                         \
	{                                                                                          \
		for (auto _i = (vec)->buf; _i < (vec)->buf + (vec)->len; ++_i)                     \
		{                                                                                  \
			func(_i);                                                                  \
		}                                                                                  \
	} while (0)

#define agc_vec_foreach(vec, element)                                                              \
	for (auto element = (vec)->buf; element < (vec)->buf + (vec)->len; ++element)

#if agc_vec_may_use_stack
AGC_VEC_API agc_err_t
agc_vec_fn(init_from_stack_buf)(agc_vec_t OUT_vec[static 1],
                                int32_t   count,
                                T         stack_buf[static count])
{
	if (!OUT_vec) return AGC_ERR_NULL;
	if (!stack_buf) return AGC_ERR_NULL;

	OUT_vec->len       = 0;
	OUT_vec->cap       = count;
	OUT_vec->stack_buf = true;
	OUT_vec->buf       = stack_buf;

	return AGC_OK;
}

	#define agc_vec_init_from_stack_buf(vec, arr)                                              \
		agc_vec_fn(init_from_stack_buf)((vec), agc_countof(arr), (arr))
#endif

#if agc_vec_may_use_stack
AGC_VEC_API agc_err_t agc_vec_fn(buf_switch_to_heap)(agc_vec_t vec[static 1]);
#endif

AGC_VEC_API agc_err_t
agc_vec_fn(init)(agc_vec_t OUT_vec[static 1], int32_t init_cap)
{
	if (!OUT_vec) return AGC_ERR_NULL;
	if (init_cap <= 0) init_cap = AGC_VEC_DEFAULT_CAP;

	T *buf = agc_vec_alloc(sizeof(T) * init_cap);
	if (!buf) return AGC_ERR_MEMORY;

	OUT_vec->len = 0;
	OUT_vec->cap = init_cap;
#if agc_vec_may_use_stack
	OUT_vec->stack_buf = false;
#endif
	OUT_vec->buf = buf;

	return AGC_OK;
}

AGC_VEC_API void
agc_vec_fn(cleanup)(agc_vec_t vec[static 1])
{
	if (!vec) return;

	for (int32_t i = 0; i < vec->len; i++)
		agc_vec_element_cleanup(vec->buf + i);

#if agc_vec_may_use_stack
	if (vec->stack_buf)
	{
		vec->len = 0;
		return;
	}
#endif
	agc_vec_free(vec->buf);
	vec->buf = nullptr;
	vec->cap = 0;
	vec->len = 0;
}

AGC_VEC_API agc_err_t
agc_vec_fn(reserve)(agc_vec_t vec[static 1], int32_t new_cap)
{
	if (!vec) return AGC_ERR_NULL;
	if (new_cap <= vec->cap) return AGC_OK;

#if agc_vec_may_use_stack
	if (vec->stack_buf)
	{
		agc_err_t err = agc_vec_fn(buf_switch_to_heap)(vec);
		if (err) return err;
	}
#endif
	T *new_buf = agc_vec_realloc(vec->buf, sizeof(T) * new_cap);
	if (!new_buf) return AGC_ERR_MEMORY;

	vec->buf = new_buf;
	vec->cap = new_cap;
	return AGC_OK;
}

AGC_VEC_API agc_err_t
agc_vec_fn(grow)(agc_vec_t vec[static 1], int32_t min_cap)
{
	if (!vec) return AGC_ERR_NULL;
	if (min_cap <= vec->cap) return AGC_OK;

	int32_t new_cap = agc_max(vec->cap * AGC_VEC_GROWTH_FACTOR, min_cap);
	if (vec->cap == 0 && new_cap < 1) new_cap = 1;

	return agc_vec_fn(reserve)(vec, new_cap);
}

AGC_VEC_API agc_err_t
agc_vec_fn(resize)(agc_vec_t vec[static 1], int32_t new_len)
{
	if (!vec) return AGC_ERR_NULL;
	if (new_len < 0) return AGC_ERR_INVALID;
	agc_err_t err = AGC_OK;

	if (new_len < vec->len)
	{
		for (int32_t i = new_len; i < vec->len; ++i)
			agc_vec_element_cleanup(vec->buf + i);
		memset(vec->buf + new_len, 0, (vec->len - new_len) * sizeof(T));
	}

	err = agc_vec_fn(grow)(vec, new_len);
	if (err) return err;

	if (new_len > vec->len) memset(vec->buf + vec->len, 0, (new_len - vec->len) * sizeof(T));

	vec->len = new_len;
	return err;
}

AGC_VEC_API agc_err_t
agc_vec_fn(shrink_to_fit)(agc_vec_t vec[static 1])
{
	if (!vec) return AGC_ERR_NULL;
	if (vec->len == vec->cap) return AGC_OK;
#if agc_vec_may_use_stack
	if (vec->stack_buf)
	{
		agc_err_t err = agc_vec_fn(buf_switch_to_heap)(vec);
		if (err) return err;
	}
#endif

	if (vec->len == 0)
	{
		agc_vec_free(vec->buf);
		vec->buf = nullptr;
		vec->cap = 0;
		return AGC_OK;
	}

	T *new_buf = agc_vec_realloc(vec->buf, sizeof(T) * vec->len);
	if (!new_buf) return AGC_ERR_MEMORY;

	vec->buf = new_buf;
	vec->cap = vec->len;
	return AGC_OK;
}

AGC_VEC_API agc_err_t
agc_vec_fn(put_cpy)(agc_vec_t vec[static 1], int32_t pos, T value)
{
	if (!vec) return AGC_ERR_NULL;
	if (pos < 0) return AGC_ERR_INVALID;
	if (pos > vec->len) return AGC_ERR_OOB;
	agc_err_t err = AGC_OK;

	err = agc_vec_fn(grow)(vec, vec->len + 1);
	if (err) return err;

	memmove(vec->buf + pos + 1, vec->buf + pos, (vec->len - pos) * sizeof(T));
	vec->buf[pos] = value;
	vec->len++;

	return err;
}

AGC_VEC_API agc_err_t
agc_vec_fn(put_mv)(agc_vec_t vec[static 1], int32_t pos, T **value)
{
	if (!vec || !value) return AGC_ERR_NULL;
	if (pos < 0) return AGC_ERR_INVALID;
	if (pos > vec->len) return AGC_ERR_OOB;
	agc_err_t err = AGC_OK;

	err = agc_vec_fn(grow)(vec, vec->len + 1);
	if (err) return err;

	memmove(vec->buf + pos + 1, vec->buf + pos, (vec->len - pos) * sizeof(T));
	memcpy(vec->buf + pos, *value, sizeof(T));
	agc_vec_free(*value);
	*value = nullptr;
	vec->len++;

	return err;
}

AGC_VEC_API agc_err_t
agc_vec_fn(push_mv)(agc_vec_t vec[static 1], T **value)
{
	return agc_vec_fn(put_mv)(vec, vec->len, value);
}

AGC_VEC_API agc_err_t
agc_vec_fn(push_cpy)(agc_vec_t vec[static 1], T value)
{
	return agc_vec_fn(put_cpy)(vec, vec->len, value);
}

AGC_VEC_API agc_err_t
agc_vec_fn(array_cpy)(agc_vec_t vec[static 1], int32_t pos, T arr[static 1], int32_t count)
{
	if (!vec || !arr) return AGC_ERR_NULL;
	if (pos < 0 || count < 0) return AGC_ERR_INVALID;
	if (pos > vec->len) return AGC_ERR_OOB;
	agc_err_t err = AGC_OK;

	err = agc_vec_fn(grow)(vec, vec->len + count);
	if (err) return err;

	memmove(vec->buf + pos + count, vec->buf + pos, (vec->len - pos) * sizeof(T));
	memcpy(vec->buf + pos, arr, count * sizeof(T));
	vec->len += count;

	return err;
}

AGC_VEC_API agc_err_t
agc_vec_fn(array_mv)(agc_vec_t vec[static 1], int32_t pos, T **arr, int32_t count)
{
	if (!vec || !arr) return AGC_ERR_NULL;
	agc_err_t err = AGC_OK;

	err = agc_vec_fn(array_cpy)(vec, pos, *arr, count);
	if (err) return err;

	agc_vec_free(*arr);
	*arr = nullptr;
	return err;
}

#if agc_vec_may_use_stack
AGC_VEC_API agc_err_t
agc_vec_fn(buf_switch_to_heap)(agc_vec_t vec[static 1])
{
	if (!vec) return AGC_ERR_NULL;
	if (!vec->stack_buf) return AGC_ERR_INVALID;
	agc_err_t err = AGC_OK;

	T      *stack_buf = vec->buf;
	int32_t old_len   = vec->len;
	int32_t old_cap   = vec->cap;

	err = agc_vec_fn(init)(vec, old_cap);
	if (err)
	{
		vec->buf = stack_buf;
		return err;
	}

	err = agc_vec_fn(array_cpy)(vec, 0, stack_buf, old_len);
	if (err)
	{
		agc_vec_free(vec->buf);
		vec->buf = stack_buf;
		vec->len = old_len;
		vec->cap = old_cap;

		return err;
	}

	vec->stack_buf = false;
	return err;
}
#endif

AGC_VEC_API agc_err_t
agc_vec_fn(pop_at)(agc_vec_t vec[static 1], int32_t pos, T *OUT_value)
{
	if (!vec) return AGC_ERR_NULL;
	if (pos < 0 || pos >= vec->len) return AGC_ERR_OOB;

	if (OUT_value)
	{
		memcpy(OUT_value, vec->buf + pos, sizeof(T));
	}
	else
	{
		agc_vec_element_cleanup(vec->buf + pos);
	}

	memmove(vec->buf + pos, vec->buf + pos + 1, (vec->len - pos - 1) * sizeof(T));
	vec->len--;

	return AGC_OK;
}

AGC_VEC_API agc_err_t
agc_vec_fn(pop)(agc_vec_t vec[static 1], T OUT_value[static 1])
{
	return agc_vec_fn(pop_at)(vec, vec->len - 1, OUT_value);
}

AGC_VEC_API agc_err_t
agc_vec_fn(erase_range)(agc_vec_t vec[static 1], int32_t first, int32_t last)
{
	if (!vec) return AGC_ERR_NULL;
	if (first >= vec->len || last > vec->len || first > last) return AGC_ERR_OOB;

	int32_t count = last - first;

	for (int32_t i = first; i < last; i++)
		agc_vec_element_cleanup(&vec->buf[i]);

	memmove(vec->buf + first, vec->buf + first + count, (vec->len - last) * sizeof(T));
	vec->len -= count;

	return AGC_OK;
}

AGC_VEC_API agc_err_t
agc_vec_fn(erase)(agc_vec_t vec[static 1], int32_t pos)
{
	return agc_vec_fn(erase_range)(vec, pos, pos + 1);
}

AGC_VEC_API void
agc_vec_fn(clear)(agc_vec_t vec[static 1])
{
	if (!vec) return;

	for (int32_t i = 0; i < vec->len; i++)
		agc_vec_element_cleanup(vec->buf + i);

	memset(vec->buf, 0, vec->len * sizeof(T));
	vec->len = 0;
}

AGC_VEC_API agc_err_t
agc_vec_fn(swap_elements)(agc_vec_t vec[static 1], int32_t i, int32_t j)
{
	if (!vec) return AGC_ERR_NULL;
	if (i < 0 || j < 0 || i >= vec->len || j >= vec->len) return AGC_ERR_OOB;
	if (i == j) return AGC_OK;

	T tmp;
	memcpy(&tmp, vec->buf + i, sizeof(T));
	memcpy(vec->buf + i, vec->buf + j, sizeof(T));
	memcpy(vec->buf + j, &tmp, sizeof(T));

	return AGC_OK;
}

AGC_VEC_API agc_err_t
agc_vec_fn(merge_subvec)(agc_vec_t  vec[static 1],
                         int32_t    pos,
                         agc_vec_t *subvec,
                         int32_t    first,
                         int32_t    last)
{
	if (!subvec) return AGC_OK;
	if (vec == subvec) return AGC_ERR_INVALID;
	if (first >= subvec->len || last > subvec->len || first > last) return AGC_ERR_OOB;
	agc_err_t err = AGC_OK;

	int32_t count = last - first;

	err = agc_vec_fn(array_cpy)(vec, pos, subvec->buf + first, count);
	if (err) return err;

	memmove(subvec->buf + first, subvec->buf + last, (subvec->len - last) * sizeof(T));
	subvec->len -= count;

	return err;
}

#if agc_vec_element_deepcopy
AGC_VEC_API agc_err_t
agc_vec_fn(get_deepcopy)(const agc_vec_t vec[static 1], int32_t pos, T **OUT_value)
{
	if (!vec || !OUT_value) return AGC_ERR_NULL;
	if (pos < 0 || pos >= vec->len) return AGC_ERR_OOB;

	*OUT_value = agc_vec_element_deepcopy(vec->buf + pos);
	return *OUT_value ? AGC_OK : AGC_ERR_CALLBACK;
}
#endif
