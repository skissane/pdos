/* Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish,
 * use, compile, sell and distribute this work
 * and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions,
 * without complying with any conditions
 * and by any means.
 */

#define read_makefile _Prmkfile
int read_makefile (const char *filename);
#define read_memory_makefile _Prmemmk
void read_memory_makefile (char *memory);

void include_dir_add (const char *name);
void include_dirs_destroy (void);
