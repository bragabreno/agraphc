#include <stdckdint.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "error.h"

#define AGC_VEC_API [[maybe_unused]] static

#ifndef AGC_VEC_NAMESPACE
	#error "You must define AGC_VEC_NAMESPACE prior to the inclusion of vector.h"
#endif
#ifndef T
	#error "You must define T prior to the inclusion of vector.h"
#endif

/* Handle macro-generated names nicely */
#define agc_vec_t agc_paste2(AGC_VEC_NAMESPACE, _t)
#define agc_vec_fn(name) agc_paste3(AGC_VEC_NAMESPACE, _, name)

/* ---------------------- Vector Interface Configuration ---------------------- */
#warning "The functions corresponding to the implementation of the interface must match:"
#warning "<AGC_VEC_NAMESPACE>_element_<operation>"
#warning "Otherwise, you might get cryptic error messages from preprocessor failures."

/* Trivial cleanup handler */
AGC_VEC_API void
agc_vec_fn(noop)(T *)

{
	(void)1;
}
#ifdef agc_vec_implements_element_cleanup
	#define agc_vec_element_cleanup agc_vec_fn(element_cleanup)
#else
	#define agc_vec_element_cleanup agc_vec_fn(noop)
#endif

/* Stack buffer support */
#ifdef agc_vec_implements_stack_buf
	#define agc_vec_may_use_stack 1
#else
	#define agc_vec_may_use_stack 0
#endif

/* Deep-copy support */
#ifdef agc_vec_implements_element_deepcopy
	#define agc_vec_may_use_element_deepcopy 1
	#define agc_vec_element_deepcopy agc_vec_fn(element_deepcopy)
#else
	#define agc_vec_may_use_element_deepcopy 0
#endif

/* Comparison support */
#ifdef agc_vec_implements_element_compare
	#define agc_vec_may_use_element_compare 1
	#define agc_vec_element_compare agc_vec_fn(element_compare)
#else
	#define agc_vec_may_use_element_compare 0
#endif

/* Allocator configuration */
#ifdef agc_vec_implements_custom_alloc
	#define agc_vec_alloc agc_vec_fn(alloc)
	#define agc_vec_realloc agc_vec_fn(realloc)
	#define agc_vec_free agc_vec_fn(free)
#else
	#warning                                                                                   \
	        "Defaulting to stdlib memory management functions. The inclusion of stdlib.h is needed"
	#define agc_vec_alloc malloc
	#define agc_vec_realloc realloc
	#define agc_vec_free free
#endif

/* Growth and capacity defaults */
#ifdef AGC_VEC_GROWTH_FACTOR
#else
	#define AGC_VEC_GROWTH_FACTOR 2
#endif

#ifdef AGC_VEC_DEFAULT_CAP
#else
	#define AGC_VEC_DEFAULT_CAP 8
#endif

// clang-format off
/* Interface Validation */
#if (AGC_VEC_GROWTH_FACTOR) < 1
#  error "AGC_VEC_GROWTH_FACTOR must be >= 1"
#endif

#if (AGC_VEC_DEFAULT_CAP) <= 0
#  error "AGC_VEC_DEFAULT_CAP must be > 0"
#endif

agc_validate_interface(agc_vec_element_cleanup, void (*)(T *))

#if agc_vec_may_use_element_deepcopy
agc_validate_interface(agc_vec_element_deepcopy, T* (*)(const T *))
#endif

#if agc_vec_may_use_element_compare
agc_validate_interface(agc_vec_element_compare, int32_t (*)(const T *, const T *))
#endif
/* ------------------------------------------------------------------------ */


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
	for (auto(element) = (vec)->buf; (element) < (vec)->buf + (vec)->len; ++(element))



/* The "move"/"copy" behavior may be a bit deceiving at first,
 * so I might document it eventually                        */
AGC_VEC_API agc_err_t 
agc_vec_fn(init)(agc_vec_t OUT_vec[static 1], int32_t init_cap);

#if agc_vec_may_use_stack
AGC_VEC_API agc_err_t 
agc_vec_fn(init_from_stack_buf)(agc_vec_t OUT_vec[static 1],
                                                      int32_t   count,
                                                      T         stack_buf[static count]);
#endif

AGC_VEC_API void 
agc_vec_fn(cleanup)(agc_vec_t vec[static 1]);

AGC_VEC_API int32_t
agc_vec_fn(len)(const agc_vec_t vec[static 1]);

AGC_VEC_API int32_t
agc_vec_fn(cap)(const agc_vec_t vec[static 1]);

AGC_VEC_API int32_t
agc_vec_fn(empty)(const agc_vec_t vec[static 1]);

AGC_VEC_API T
agc_vec_fn(at)(const agc_vec_t vec[static 1], int32_t pos);

AGC_VEC_API T *
agc_vec_fn(ptr_at)(const agc_vec_t vec[static 1], int32_t pos);

AGC_VEC_API agc_err_t 
agc_vec_fn(reserve)(agc_vec_t vec[static 1], int32_t new_cap);

AGC_VEC_API agc_err_t 
agc_vec_fn(grow)(agc_vec_t vec[static 1], int32_t min_cap);

AGC_VEC_API agc_err_t 
agc_vec_fn(resize)(agc_vec_t vec[static 1], int32_t new_len);

AGC_VEC_API agc_err_t 
agc_vec_fn(shrink_to_fit)(agc_vec_t vec[static 1]);

#if agc_vec_may_use_stack
AGC_VEC_API agc_err_t 
agc_vec_fn(buf_switch_to_heap)(agc_vec_t vec[static 1]);
#endif

AGC_VEC_API agc_err_t 
agc_vec_fn(put_cpy)(agc_vec_t vec[static 1], int32_t pos, T value);

AGC_VEC_API agc_err_t 
agc_vec_fn(put_mv)(agc_vec_t vec[static 1], int32_t pos, T **value);

AGC_VEC_API agc_err_t 
agc_vec_fn(push_mv)(agc_vec_t vec[static 1], T **value);

AGC_VEC_API agc_err_t 
agc_vec_fn(push_cpy)(agc_vec_t vec[static 1], T value);

AGC_VEC_API agc_err_t 
agc_vec_fn(array_cpy)(agc_vec_t vec[static 1],
                                            int32_t   pos,
                                            int32_t   count,
                                            T         arr[static count]);

AGC_VEC_API agc_err_t 
agc_vec_fn(array_mv)(agc_vec_t vec[static 1],
                                           int32_t   pos,
                                           T       **arr,
                                           int32_t   count);

AGC_VEC_API agc_err_t 
agc_vec_fn(pop_at)(agc_vec_t vec[static 1], int32_t pos, T *OUT_value);

AGC_VEC_API agc_err_t 
agc_vec_fn(pop)(agc_vec_t vec[static 1], T *OUT_value);

AGC_VEC_API agc_err_t 
agc_vec_fn(erase_range)(agc_vec_t vec[static 1], int32_t first, int32_t last);

AGC_VEC_API agc_err_t 
agc_vec_fn(erase)(agc_vec_t vec[static 1], int32_t pos);

AGC_VEC_API void 
agc_vec_fn(clear)(agc_vec_t vec[static 1]);

AGC_VEC_API agc_err_t 
agc_vec_fn(swap_elements)(agc_vec_t vec[static 1], int32_t i, int32_t j);

#if agc_vec_may_use_element_compare
AGC_VEC_API agc_err_t
agc_vec_fn(find)(const agc_vec_t vec[static 1], const T *value, int32_t *OUT_pos);

AGC_VEC_API bool
agc_vec_fn(contains)(const agc_vec_t vec[static 1], const T *value);
#endif

AGC_VEC_API agc_err_t 
agc_vec_fn(merge_subvec)(agc_vec_t           vec[static 1],
                                   int32_t             pos,
                                   agc_vec_t          *subvec,
                                   int32_t             first,
                                   int32_t             last);
#if agc_vec_may_use_element_deepcopy
AGC_VEC_API agc_err_t 
agc_vec_fn(get_deepcopy)(const agc_vec_t vec[static 1],
                                               int32_t         pos,
                                               T             **OUT_value);
#endif

// clang-format on

/* Simple accessors do not return agc_err_t, which is a bit odd,
 * but a necessary evil, as the out parameter convention used
 * here might become a hassle to client code and to the compiler.*/
AGC_VEC_API int32_t
agc_vec_fn(len)(const agc_vec_t vec[static 1])
{
	if (!vec) return -1;
	return vec->len;
}

AGC_VEC_API int32_t
agc_vec_fn(cap)(const agc_vec_t vec[static 1])
{
	if (!vec) return -1;
	return vec->cap;
}

AGC_VEC_API int32_t
agc_vec_fn(empty)(const agc_vec_t vec[static 1])
{
	if (!vec) return -1;
	return (vec->len == 0);
}

/* This returns an empty object on failure */
AGC_VEC_API T
agc_vec_fn(at)(const agc_vec_t vec[static 1], int32_t pos)
{
	if (!vec || !vec->buf || pos >= vec->len || pos < 0) return (T){ };
	return vec->buf[pos];
}

AGC_VEC_API T *
agc_vec_fn(ptr_at)(const agc_vec_t vec[static 1], int32_t pos)
{
	if (!vec || !vec->buf || pos >= vec->len || pos < 0) return nullptr;
	return vec->buf + pos;
}

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
#endif

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

	int32_t new_cap   = { };
	int32_t grown_cap = { };

	if (ckd_mul(&grown_cap, vec->cap, AGC_VEC_GROWTH_FACTOR))
	{
		// Overflow occurred
		return AGC_ERR_OVERFLOW;
	}

	new_cap = agc_max(grown_cap, min_cap);

	size_t alloc_size;
	if (ckd_mul(&alloc_size, (size_t)new_cap, sizeof(T)))
	{
		return AGC_ERR_OVERFLOW;
	}

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
agc_vec_fn(array_cpy)(agc_vec_t vec[static 1], int32_t pos, int32_t count, T arr[static count])
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

	err = agc_vec_fn(array_cpy)(vec, pos, count, *arr);
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

	err = agc_vec_fn(array_cpy)(vec, 0, old_len, stack_buf);
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
agc_vec_fn(pop)(agc_vec_t vec[static 1], T *OUT_value)
{
	return agc_vec_fn(pop_at)(vec, vec->len - 1, OUT_value);
}

AGC_VEC_API agc_err_t
agc_vec_fn(erase_range)(agc_vec_t vec[static 1], int32_t first, int32_t last)
{
	if (!vec) return AGC_ERR_NULL;
	if (first < 0 || last < 0 || first >= vec->len || last > vec->len || first > last)
		return AGC_ERR_OOB;

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

#if agc_vec_may_use_element_compare
AGC_VEC_API agc_err_t
agc_vec_fn(find)(const agc_vec_t vec[static 1], const T *value, int32_t *OUT_pos)
{
	if (!vec || !value) return AGC_ERR_NULL;
	for (int32_t i = 0; i < vec->len; i++)
	{
		if (agc_vec_element_compare(vec->buf + i, value) == 0)
		{
			if (OUT_pos) *OUT_pos = i;
			return AGC_OK;
		}
	}
	return AGC_ERR_NOT_FOUND;
}

AGC_VEC_API bool
agc_vec_fn(contains)(const agc_vec_t vec[static 1], const T *value)
{
	return (agc_vec_fn(find)(vec, value, nullptr) == AGC_OK);
}
#endif

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

	err = agc_vec_fn(array_cpy)(vec, pos, count, subvec->buf + first);
	if (err) return err;

	memmove(subvec->buf + first, subvec->buf + last, (subvec->len - last) * sizeof(T));
	subvec->len -= count;

	return err;
}

#if agc_vec_may_use_element_deepcopy
AGC_VEC_API agc_err_t
agc_vec_fn(get_deepcopy)(const agc_vec_t vec[static 1], int32_t pos, T **OUT_value)
{
	if (!vec || !OUT_value) return AGC_ERR_NULL;
	if (pos < 0 || pos >= vec->len) return AGC_ERR_OOB;

	*OUT_value = agc_vec_element_deepcopy(vec->buf + pos);
	return *OUT_value ? AGC_OK : AGC_ERR_CALLBACK;
}
#endif

/* ---------------------- Vector Interface Cleanup ---------------------- */
#ifdef AGC_VEC_NAMESPACE
	#undef AGC_VEC_NAMESPACE
#endif

/* Growth & Capacity */
#ifdef AGC_VEC_GROWTH_FACTOR
	#undef AGC_VEC_GROWTH_FACTOR
#endif
#ifdef AGC_VEC_DEFAULT_CAP
	#undef AGC_VEC_DEFAULT_CAP
#endif

/* Cleanup Handler */
#ifdef agc_vec_element_cleanup
	#undef agc_vec_element_cleanup
#endif
#ifdef agc_vec_implements_element_cleanup
	#undef agc_vec_implements_element_cleanup
#endif

/* Stack Buffer */
#ifdef agc_vec_may_use_stack
	#undef agc_vec_may_use_stack
#endif
#ifdef agc_vec_implements_stack_buf
	#undef agc_vec_implements_stack_buf
#endif

/* Deep Copy */
#ifdef agc_vec_may_use_element_deepcopy
	#undef agc_vec_may_use_element_deepcopy
#endif
#ifdef agc_vec_element_deepcopy
	#undef agc_vec_element_deepcopy
#endif
#ifdef agc_vec_implements_element_deepcopy
	#undef agc_vec_implements_element_deepcopy
#endif

/* Comparison */
#ifdef agc_vec_may_use_element_compare
	#undef agc_vec_may_use_element_compare
#endif
#ifdef agc_vec_element_compare
	#undef agc_vec_element_compare
#endif
#ifdef agc_vec_implements_element_compare
	#undef agc_vec_implements_element_compare
#endif

/* Allocator */
#ifdef agc_vec_alloc
	#undef agc_vec_alloc
#endif
#ifdef agc_vec_realloc
	#undef agc_vec_realloc
#endif
#ifdef agc_vec_free
	#undef agc_vec_free
#endif
#ifdef agc_vec_implements_custom_alloc
	#undef agc_vec_implements_custom_alloc
#endif
