/*
 * Public domain
 */

#ifndef _FOLDER_H
#define _FOLDER_H
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Abstract folder structure
 */
typedef struct _folder FOLDER;

/**
 * Open a folder stream corresponding to the fldrname argument.
 * @param fldrname The name of the folder
 * @return On success returns a FOLDER object. On failure returns NULL.
 */
FOLDER *openfldr(const char *fldrname);

/**
 * Get the next entry in the folder.
 * If the entry is itself a folder the returned name ends with a '/'.
 * @param ffd The folder stream object
 * @return The name of the folder entry. NULL if there is no more entry or on failure.
 */
char *readfldr(FOLDER *ffd);

/**
 * Release the folder stream.
 * @param ffd The stream
 * @return 0 on success.
 */
int closefldr(FOLDER *ffd);

/**
 * Recursively create folders.
 * @param path relative or absolute path to create.
 * @return 0 on success.
 */
int mkfldr(const char *path);

/**
 * Delete an empty folder.
 * @param path relative or absolute path to delete.
 * @return  0 on success.
 */
int rmfldr(const char *path);


#ifdef __cplusplus
}
#endif
#endif /*_FOLDER_H*/
