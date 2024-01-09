/******************************************************************************
 * @file            terminal.c
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#include <windows.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define INPUT_BUF_SIZE 250

static HANDLE write_pipe_handle_in;
static HANDLE write_pipe_handle_out;
static HANDLE read_pipe_handle_out;
static HANDLE read_pipe_handle_in;

static HANDLE write_handle;
static HANDLE read_handle;

static int writing_to_pipe;
static char write_buf[INPUT_BUF_SIZE];
static OVERLAPPED write_overlapped;

static char read_buf[2];
static OVERLAPPED read_overlapped;

static void write_pipe (void);
static void read_pipe (void);

static HWND g_hwnd;

static char input_buf[INPUT_BUF_SIZE];
static size_t input_buf_len;

static char newbuf[80 * 25] = "N";

static void write_screen (int c);

/* Child process things. */
PROCESS_INFORMATION pi;
STARTUPINFO si;

static void make_pipe (void)
{
    SECURITY_ATTRIBUTES sa;

    sa.nLength = sizeof (sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;
    
    write_pipe_handle_in = CreateNamedPipe ("\\\\.\\pipe\\testpipe",
                                            PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED,
                                            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
                                            1,
                                            1000,
                                            1000,
                                            PIPE_NOWAIT,
                                            NULL);
    
    if (write_pipe_handle_in == INVALID_HANDLE_VALUE) {
        printf ("bad!\n");
        for (;;);
    }
    
    if (ConnectNamedPipe (write_pipe_handle_in, &write_overlapped)) {
        printf ("bad!\n");
        for (;;);
    }

    write_pipe_handle_out = CreateFile ("\\\\.\\pipe\\testpipe",
                                        GENERIC_READ,
                                        0,
                                        &sa,
                                        OPEN_EXISTING,
                                        0 /* No FILE_FLAG_OVERLAPPED, the other side is synchronous. */,
                                        NULL);

    if (write_pipe_handle_out == INVALID_HANDLE_VALUE) {
        printf ("bad!\n");
        for (;;);
    }

    read_pipe_handle_out = CreateNamedPipe ("\\\\.\\pipe\\testpipe2",
                                            PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,
                                            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
                                            1,
                                            1000,
                                            1000,
                                            PIPE_NOWAIT,
                                            NULL);
    
    if (read_pipe_handle_out == INVALID_HANDLE_VALUE) {
        printf ("bad!\n");
        for (;;);
    }
    
    if (ConnectNamedPipe (read_pipe_handle_out, &write_overlapped)) {
        printf ("bad!\n");
        for (;;);
    }

    read_pipe_handle_in = CreateFile ("\\\\.\\pipe\\testpipe2",
                                      GENERIC_WRITE,
                                      0,
                                      &sa,
                                      OPEN_EXISTING,
                                      0 /* No FILE_FLAG_OVERLAPPED, the other side is synchronous. */,
                                      NULL);

    if (write_pipe_handle_out == INVALID_HANDLE_VALUE) {
        printf ("bad!\n");
        for (;;);
    }

    write_handle = write_pipe_handle_in;
    read_handle = read_pipe_handle_out;
}

static void close_pipe (void)
{
    CloseHandle (write_pipe_handle_in);
    CloseHandle (write_pipe_handle_out);
    CloseHandle (read_pipe_handle_out);
    CloseHandle (read_pipe_handle_in);
}

static void __stdcall write_pipe_callback (DWORD dwErrorCode,
                                           DWORD dwNumberOfBytesTransferred,
                                           LPOVERLAPPED lpOverlapped)
{
    writing_to_pipe = 0;
    if (input_buf_len) write_pipe ();
}

static void write_pipe (void)
{
    int ret;

    if (writing_to_pipe) return;
    writing_to_pipe = 1;
    
    memcpy (write_buf, input_buf, input_buf_len);

    ret = WriteFileEx (write_handle,
                       write_buf,
                       input_buf_len,
                       &write_overlapped,
                       &write_pipe_callback);

    if (ret == 0) {
        printf ("bad write_pipe!\n");
        for (;;);
    }

    input_buf_len = 0;
}

static void __stdcall read_pipe_callback (DWORD dwErrorCode,
                                          DWORD dwNumberOfBytesTransferred,
                                          LPOVERLAPPED lpOverlapped)
{
    write_screen (read_buf[0]);

    read_pipe ();
}

static void read_pipe (void)
{
    int ret;
    
    ret = ReadFileEx (read_handle,
                      read_buf,
                      1,
                      &read_overlapped,
                      &read_pipe_callback);

    if (ret == 0) {
        printf ("bad read_pipe!\n");
        for (;;);
    }
}

static void create_child_process (void)
{
    BOOL rc;
    char cmd_line[MAX_PATH];

    strcpy (cmd_line, "world.exe");

    memset (&pi, 0, sizeof (pi));

    memset (&si, 0, sizeof (si));
    si.cb = sizeof si;
    si.hStdError = read_pipe_handle_in;
    si.hStdOutput = read_pipe_handle_in;
    si.hStdInput = write_pipe_handle_out;
    si.dwFlags |= STARTF_USESTDHANDLES;

    rc = CreateProcess (NULL,
                        cmd_line,
                        NULL,
                        NULL,
                        TRUE,
                        CREATE_NO_WINDOW,
                        NULL,
                        NULL,
                        &si,
                        &pi);

    if (rc == 0) {
        printf ("CreateProcess failed\n");
    }
}

static void terminate_child_process (void)
{
    DWORD ExitCode;

    if (GetExitCodeProcess (pi.hProcess, &ExitCode) == 0
        || (ExitCode == STILL_ACTIVE
            && TerminateProcess (pi.hProcess, 0) == 0)) {
        printf ("TerminateProcess failed\n");
    }

    CloseHandle (pi.hProcess);
    CloseHandle (pi.hThread);
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int __stdcall WinMain (HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpCmdLine,
                    int nShowCmd)
{
    WNDCLASS wndClass = { 0, (WNDPROC)WndProc, 0, 0, 0, NULL, NULL, 
                          (HBRUSH)(COLOR_WINDOW+1), "testterminal", "testterminal" };
    HWND hwnd;            
    MSG msg;
    
    wndClass.hInstance = hInstance;
    RegisterClass (&wndClass);
    
    hwnd = CreateWindow ("testterminal", "Test terminal for Windows", 
                         WS_SYSMENU | WS_CAPTION,
                         CW_USEDEFAULT, CW_USEDEFAULT, 
                         CW_USEDEFAULT, CW_USEDEFAULT,
                         NULL, NULL, hInstance, NULL);


    ShowWindow (hwnd, nShowCmd);
    UpdateWindow (hwnd);

    g_hwnd = hwnd;

    make_pipe ();
    create_child_process ();        
    read_pipe ();

    while (1) {
        switch (MsgWaitForMultipleObjectsEx (0,
                                             NULL,
                                             INFINITE,
                                             QS_ALLINPUT,
                                             MWMO_ALERTABLE | MWMO_INPUTAVAILABLE)) {
            case 0:
                while (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE)) {
                    if (msg.message == WM_QUIT) goto out;
                    TranslateMessage (&msg);
                    DispatchMessage (&msg);
                }
                break;

            case WAIT_IO_COMPLETION:
                break;

            default: goto out;
        }
    }
out:

    terminate_child_process ();

    close_pipe ();
    
    return msg.wParam;
}

int main ()
{
    return WinMain (GetModuleHandle (NULL), NULL, NULL, 1);
}

static int lcontrol_down = 0;
static int line_buffering = 1;

static void write_str_to_input_buf (const char *str)
{
    strcpy (input_buf + input_buf_len, str);
    input_buf_len += strlen (str);
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{    
    switch (msg) {
        case WM_CREATE:
            break;
        
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc;
            RECT rect;

            InvalidateRect (hwnd, NULL, TRUE); /* Always redraw the entire window. */

            hdc = BeginPaint (hwnd, &ps);
            GetClientRect (hwnd, &rect);

            {
                int i;

                for (i = 0; i < 25; i++) {
                    TextOut (hdc, 0, i * 16, newbuf + 80 * i, 80);
                }
            }
            
            EndPaint (hwnd, &ps);
            break;
        }

        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        
        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_CHAR:
            if (line_buffering) {
                write_screen (wParam);
                if (wParam == '\r') write_screen ('\n');
            }
        
            if (write_handle) {
                if (lcontrol_down) {
                    write_str_to_input_buf ("\x1b[1;5D");
                }

                if (line_buffering && wParam == '\b') {
                    if (input_buf_len) {
                        input_buf_len--;
                        input_buf[input_buf_len] = '\0';
                    }
                }

                /* Duplicate ESC. */
                if (wParam == 0x1b) input_buf[input_buf_len++] = wParam;
                
                input_buf[input_buf_len++] = wParam;
                /* ENTER gives '\r', not '\n'. */
                if (wParam == '\r') input_buf[input_buf_len++] = '\n';
                
                if (!line_buffering || wParam == '\r') write_pipe ();
            }
            break;

        case WM_KEYDOWN:
            if (wParam == VK_LCONTROL) lcontrol_down = 1;
            switch (wParam) {
                case VK_LEFT: write_str_to_input_buf ("\x1b[D"); break;
                case VK_UP: write_str_to_input_buf ("\x1b[A"); break;
                case VK_RIGHT: write_str_to_input_buf ("\x1b[C"); break;
                case VK_DOWN: write_str_to_input_buf ("\x1b[B"); break;

                default: break;
            }
            break;

        case WM_KEYUP:
            if (wParam == VK_LCONTROL) lcontrol_down = 0;
            break;
        
        default:
            return DefWindowProc (hwnd, msg, wParam, lParam);
    }

    return 0;
}

static unsigned int g_row = 0;
static unsigned int g_column = 0;

static int numansi = 0;
static char ansibuf[50];

static void clear_screen (void)
{
    memset (newbuf, ' ', sizeof (newbuf));
}

static void get_cursor_position (unsigned int *row_p, unsigned int *column_p)
{
    *row_p = g_row;
    *column_p = g_column;
}

static void set_cursor_position (unsigned int row, unsigned int column)
{
    g_row = row;
    g_column = column;
}

static void write_char (unsigned char ch)
{
    if (ch == 0x8) {
        if (g_column > 0) g_column--;
        newbuf[g_row * 80 + g_column] = ' ';
    } else if (ch == '\r') {
        g_column = 0;
    } else if (ch == '\n') {
        g_row++;
    } else {
        newbuf[g_row * 80 + g_column] = ch;
        g_column++;
        if (g_column > 79) {
            g_column = 0;
            g_row++;
        }
    }

    if (g_row == 25) {
        memmove (newbuf, newbuf + 80, 80 * 24);
        memset (newbuf + 80 * 24, ' ', 80);
        g_row--;
    }
}

static void clear_in_line (unsigned int from, unsigned int to)
{
    for (; from < to; from++) {
        newbuf[g_row * 80 + from] = ' ';
    }
}

static void write_screen (int ch)
{
    unsigned int row;
    unsigned int column;
    
    if ((ch == 0x1b) && (numansi == 0))
    {
        ansibuf[numansi++] = ch;
        return;
    }
    if (numansi == 1)
    {
        if (ch == '[')
        {
            ansibuf[numansi++] = ch;
            return;
        }
        else
        {
            numansi = 0;
        }
    }
    if (numansi > 0)
    {
        if ((ch >= 0x40) && (ch <= 0x7e))
        {
            ansibuf[numansi] = '\0';
            if (ch == 'J')
            {
                clear_screen ();
            }
            else if (ch == 'H')
            {
                char *p;
                row = atoi (ansibuf + 2);
                if (row == 0) row++;
                column = 0;
                p = strchr (ansibuf + 2, ';');
                if (p != NULL)
                {
                    column = atoi (p + 1);
                }
                if (column == 0) column++;
                set_cursor_position (row-1, column-1);
            }
            else if (ch == 'K')
            {
                get_cursor_position (&row, &column);
                clear_in_line (column, 80);
                InvalidateRect (g_hwnd, NULL, TRUE);
            }
            numansi = 0;
            return;
        }
        else
        {
            ansibuf[numansi++] = ch;
            if (numansi == sizeof ansibuf)
            {
                numansi = 0;
            }
            return;
        }
    }
    
    write_char (ch);
    InvalidateRect (g_hwnd, NULL, TRUE);
}
