#ifdef HAVE_FOLDER
/*

          21 August MMXXIV PUBLIC DOMAIN by JML

     The authors and contributors disclaim copyright, patents
           and all related rights to this software.

 Anyone is free to copy, modify, publish, use, compile, sell, or
 distribute this software, either in source code form or as a
 compiled binary, for any purpose, commercial or non-commercial,
 and by any means.

 The authors waive all rights to patents, both currently owned
 by the authors or acquired in the future, that are necessarily
 infringed by this software, relating to make, have made, repair,
 use, sell, import, transfer, distribute or configure hardware
 or software in finished or intermediate form, whether by run,
 manufacture, assembly, testing, compiling, processing, loading
 or applying this software or otherwise.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT OF ANY PATENT, COPYRIGHT, TRADE SECRET OR OTHER
 PROPRIETARY RIGHT.  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR
 ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 */

#include "folder.h"

/****************************************************************************/

#if defined(POS_EXTEND)
#include <string.h>
#include <pos.h>
struct _folder
{
    int dummy;
};

FOLDER *openfldr(const char *fldrname)
{
    return NULL;
}

char *readfldr(FOLDER *ffd)
{
    return NULL;
}

int closefldr(FOLDER *ffd)
{
    return 0;
}


#elif defined(_MSC_VER)
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef MAX_PATH
#define MAX_PATH 4096
#endif

struct _folder
{
    char fldrname[MAX_PATH + 4];
    size_t namelen;
    HANDLE hFind;
    WIN32_FIND_DATAA ffd;
    BOOL state;
};

FOLDER *openfldr(const char *fldrname)
{
    FOLDER *ffd;
    char buf[MAX_PATH];

    if (!fldrname)
    {
        return NULL;
    }
    if (strlen(fldrname) > (MAX_PATH - 3))
    {
        return NULL;
    }
    ffd = malloc(sizeof(*ffd));
    if (!ffd)
    {
        return NULL;
    }
#ifdef HAVE_SNPRINTF
    snprintf(buf, MAX_PATH, "%s\\*", fldrname);
#else
    sprintf(buf, "%s\\*", fldrname);
#endif
    ffd->hFind = FindFirstFileA(buf, &(ffd->ffd));
    if (ffd->hFind == INVALID_HANDLE_VALUE)
    {
        free(ffd);
        return NULL;
    }
    ffd->namelen = strlen(fldrname);
    memcpy(ffd->fldrname, fldrname, ffd->namelen);
    ffd->fldrname[ffd->namelen] = '/';
    ffd->namelen++;
    ffd->fldrname[ffd->namelen] = '\0';
    ffd->state = TRUE;
    return ffd;
}

char *readfldr(FOLDER *ffd)
{
    size_t l;

    while (ffd->state && (!strcmp(ffd->ffd.cFileName, ".") || !strcmp(ffd->ffd.cFileName, "..")))
    {
        ffd->state = FindNextFileA(ffd->hFind, &(ffd->ffd));
    }
    if (!ffd->state)
    {
        return NULL;
    }
    l = strlen(ffd->ffd.cFileName);
    if (l + ffd->namelen >= sizeof(ffd->fldrname) - 2)
    {
        return NULL;
    }
    memcpy(ffd->fldrname + ffd->namelen, ffd->ffd.cFileName, l);
    ffd->fldrname[ffd->namelen + l] = '\0';

    if (ffd->ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        ffd->fldrname[ffd->namelen + l] = '/';
        ffd->fldrname[ffd->namelen + l + 1] = '\0';
    }
    ffd->state = FindNextFileA(ffd->hFind, &(ffd->ffd));
    return ffd->fldrname + ffd->namelen;
}

int closefldr(FOLDER *ffd)
{
    int r = 0;
    DWORD dwError;

    dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES)
    {
        r = -1;
    }
    FindClose(ffd->hFind);
    free(ffd);
    return r;
}

#else /* defined(_MSC_VER) */
/****************************************************************************/
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef FILENAME_MAX
#define FILENAME_MAX 4096
#endif

struct _folder
{
    char fldrname[FILENAME_MAX + 4];
    size_t namelen;
    DIR *dfd;
};

FOLDER *openfldr(const char *fldrname)
{
    FOLDER *ffd;

    if (!fldrname)
    {
        return NULL;
    }
    ffd = malloc(sizeof(*ffd));
    if (!ffd)
        return NULL;
    ffd->dfd = opendir(fldrname);
    if (!ffd->dfd)
    {
        free(ffd);
        return NULL;
    }
    ffd->namelen = strlen(fldrname);
    memcpy(ffd->fldrname, fldrname, ffd->namelen);
    ffd->fldrname[ffd->namelen] = '/';
    ffd->namelen++;
    ffd->fldrname[ffd->namelen] = '\0';
    return ffd;
}

char *readfldr(FOLDER *ffd)
{
    struct dirent *d;
    size_t l;
    struct stat sb;
    do
    {
        d = readdir(ffd->dfd);
    } while (d && (!strcmp(d->d_name, ".") || !strcmp(d->d_name, "..")));
    if (!d)
    {
        return NULL;
    }
    l = strlen(d->d_name);
    if (l + ffd->namelen >= sizeof(ffd->fldrname) - 2)
    {
        return NULL;
    }
    memcpy(ffd->fldrname + ffd->namelen, d->d_name, l);
    ffd->fldrname[ffd->namelen + l] = '\0';

    if (stat(ffd->fldrname, &sb) == -1)
    {
        return NULL;
    }
    if (S_ISDIR(sb.st_mode))
    {
        ffd->fldrname[ffd->namelen + l] = '/';
        ffd->fldrname[ffd->namelen + l + 1] = '\0';
    }
    return ffd->fldrname + ffd->namelen;
}

int closefldr(FOLDER *ffd)
{
    int r;
    r = closedir(ffd->dfd);
    free(ffd);
    return r;
}

/****************************************************************************/
#endif /* !defined(_MSC_VER)*/
/****************************************************************************/

int mkfldr(const char *path)
{
    char temp[FILENAME_MAX];
    char *b;
    char c;
    int r = 0;
    size_t l;
    l = strlen(path);
    if (l >= sizeof(temp) -1) {
        return -1;
    }
    memcpy(temp, path, l + 1);
    b = temp;
    while (*b)
    {
        b++;
        while (*b && *b != '/' && *b != '\\')
        {
            b++;
        }
        c = *b;
        *b = '\0';
#if defined(__MINGW32__)
        r = mkdir(temp);
#elif defined(POS_EXTEND)
        r = PosMakeFullDir(temp);
#elif defined(_MSC_VER)
        /* r = _mkdir(temp); */
        r = CreateDirectory(temp, 0);
#else
        r = mkdir(temp, 511);
#endif
        *b = c;
    }
    return r;
}

int rmfldr(const char *path)
{
#if defined(_MSC_VER)
    /* return _rmdir(path); */
    return (RemoveDirectory(path));
#elif defined(POS_EXTEND)
    return PosRemoveDir(path);
#else
    return rmdir(path);
#endif
}
#endif
