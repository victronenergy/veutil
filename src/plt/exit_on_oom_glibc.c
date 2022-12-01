/* to include features.h for __GLIBC__ ... */
#include <stdio.h>

#ifdef __GLIBC__

#include <errno.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/param.h>

#include <veutil/platform/plt.h>

/*
 * On Venus devices the processes are supervised and run under softlimit to
 * limit memory resources for each process. For normal operation there
 * should be enough memory. If the whole device runs out of memory it will
 * be rebooted. So when memory allocation fails there is most likely a leak
 * and the intend is to exit and the program will be restarted.
 *
 * Instead of decorating the whole program with exit statement or causing
 * undefined behaviour, overwrite the allocation functions and exit on oom.
 */

static bool doExit;

void pltExitOnOom(void)
{
	doExit = true;
}

void pltContinueOnOom(void)
{
	doExit = false;
}

static void oom(void)
{
	if (!doExit)
		return;

	fputs("out of memory, exit\n", stderr);
	fflush(stderr);
	/*
	 * This used to be exit, but in the c++ world that will trigger,
	 * all destructor to be run from a completely arbitrary place
	 * while being OOM. We might be holding locks, deadlocking ourself
	 * etc. So abort instead of cleaning up.
	 */
	abort();
}

extern __typeof(malloc) __libc_malloc;
extern __typeof(calloc) __libc_calloc;
extern __typeof(realloc) __libc_realloc;
extern __typeof(memalign) __libc_memalign;

void *malloc(size_t size)
{
	void *ret = __libc_malloc(size);
	if (!ret)
		oom();
	return ret;
}

void *calloc(size_t n, size_t size)
{
	void *ret = __libc_calloc(n, size);
	if (!ret)
		oom();
	return ret;
}

void *realloc(void *ptr, size_t size)
{
	void *ret = __libc_realloc(ptr, size);
	if (size && !ret)
		oom();
	return ret;
}

void *memalign(size_t alignment, size_t size)
{
	void *ret = __libc_memalign(alignment, size);
	if (!ret)
		oom();
	return ret;
}

/*
 * There doesn't seem to be a symbol for the real posix_memalign, so
 * use memalign.
 */
int posix_memalign(void **memptr, size_t alignment, size_t size)
{
	if (!powerof2(alignment) || (alignment % sizeof(void *)) != 0) {
		*memptr = NULL;
		return EINVAL;
	}

	*memptr = memalign(alignment, size);
	if (!*memptr)
		oom();
	return 0;
}

/*
 * The function aligned_alloc() is the same as memalign(), except for
 * the added restriction that size should be a multiple of alignment.
 */
void *aligned_alloc(size_t alignment, size_t size)
{
	if ((size % alignment) != 0)
		return NULL;

	return memalign(alignment, size);
}

/* Deprecated */
static void no_stub(char const *function)
{
	fprintf(stderr, "missing stub for %s, don't use the function or add the stub, exit\n", function);
	fflush(stderr);
	exit(66);
}

void *pvalloc(size_t size)
{
	VE_UNUSED(size);

	no_stub(__FUNCTION__);
	return NULL;
}

void *valloc(size_t size)
{
	VE_UNUSED(size);

	no_stub(__FUNCTION__);
	return NULL;
}

#endif
