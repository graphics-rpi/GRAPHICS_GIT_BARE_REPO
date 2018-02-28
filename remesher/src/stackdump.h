#ifndef STACKDUMP_H
#define STACKDUMP_H

void StackDump(void);
void assert_stackdump_func(const char *exp, const char *file, int line);

// similiar to assert.h
#ifdef NDEBUG
#undef assert_stackdump
#define assert_stackdump(EX) ((void)0)
#else
#define assert_stackdump(EX) \
 ((EX)?((void)0):assert_stackdump_func( # EX , __FILE__, __LINE__))
#endif

#endif

