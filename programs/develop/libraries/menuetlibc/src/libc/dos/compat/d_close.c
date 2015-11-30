/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * D_CLOSE.C.
 *
 * Written by Peter Sulyok 1995 <sulyok@math.klte.hu>.
 *
 * This file is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <libc/dosio.h>
#include <errno.h>
#include <dos.h>

unsigned int _dos_close(int handle)
{
 return dosemu_close(handle);
}
