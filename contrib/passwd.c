#include <stdio.h>
#include <string.h>
#include <assert.h>

int main (void)
{
	const char *const pw = "MySuperSecretPwd";	

	const size_t pw_len = strlen (pw);
	assert ( pw_len == 16 );

	int p = 0;
	for (p = 0; p < pw_len; ++p) {
		if ( p == 8 ) { puts ("\n"); }
		printf ("%x", 42 ^ pw [p]);
	}
	puts ("\n");
}
