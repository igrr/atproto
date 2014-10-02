#ifndef __DCE_UTILS
#define __DCE_UTILS


int dce_ishex(char c);

char dce_htoi(char c);

int dce_expect_number(const char** buf, size_t *psize, int def_val);
void dce_itoa(int val, char* buf, size_t bufsize, size_t* outsize);
void dce_itoa_zeropad(int val, char* buf, size_t bufsize);  // works for nonnegative numbers
#endif //__DCE_UTILS
