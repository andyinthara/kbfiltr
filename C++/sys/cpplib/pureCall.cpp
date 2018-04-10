#pragma code_seg()

/*****************************************************************************
 * _purecall()
 *****************************************************************************
 * The C++ compiler loves me.
 * TODO: Figure out how to put this into portcls.sys
 */
int __cdecl
_purecall( void )
{
    ASSERT( !"Pure virutal function called" );
    return 0;
}