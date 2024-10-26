/*********************************************************************/
/*                                                                   */
/*  This Program Written by Paul Edwards.                            */
/*  Released to the Public Domain                                    */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*  fpfuncsm.c - some support routines for floating point work       */
/*  and beyond for Microsoft Visual C targeting ARM64                */
/*                                                                   */
/*********************************************************************/

int _fltused = 0;

void __GSHandlerCheck(void)
{
    return;
}

void __security_pop_cookie(void)
{
    return;
}

void __security_push_cookie(void)
{
    return;
}

void __report_rangecheckfailure(void)
{
    return;
}
