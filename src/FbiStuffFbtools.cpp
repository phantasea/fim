/* $LastChangedDate: 2013-07-04 21:56:00 +0200 (Thu, 04 Jul 2013) $ */
/*
 FbiStuffFbtools.cpp : fbi functions from fbtools.c, modified for fim

 (c) 2008-2013 Michele Martone
 (c) 1998-2006 Gerd Knorr <kraxel@bytesex.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/
/*
 * This file comes from fbi, and will undergo severe reorganization.
 * */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>	// sigaction, ...
#include <setjmp.h>
#ifdef HAVE_SYS_USER_H
#include <sys/user.h>	  /* this should be a safer replacement */
#endif /* HAVE_SYS_USER_H */

#include "FbiStuffFbtools.h"
#include "FramebufferDevice.h"

namespace fim
{




/* -------------------------------------------------------------------- */
/* handle fatal errors                                                  */

//static jmp_buf fb_fatal_cleanup;	/* old, broken */
static sigjmp_buf fb_fatal_cleanup;	/* posix */

static void
fb_catch_exit_signal(int signal)
{
    siglongjmp(fb_fatal_cleanup,signal);
}

void fb_catch_exit_signals(void)
{
    struct sigaction act,old;
    int termsig;

    fim_bzero(&act,sizeof(act));
    act.sa_handler = fb_catch_exit_signal;
    sigemptyset(&act.sa_mask);
    sigaction(SIGINT, &act,&old);
    sigaction(SIGQUIT,&act,&old);
    sigaction(SIGTERM,&act,&old);

    sigaction(SIGABRT,&act,&old);
    sigaction(SIGTSTP,&act,&old);

    sigaction(SIGBUS, &act,&old);
    sigaction(SIGILL, &act,&old);
    sigaction(SIGSEGV,&act,&old);

    if (0 == (termsig = sigsetjmp(fb_fatal_cleanup,0)))
	return;

    /* console cleanup should happen here */
    cc.cleanup();
#ifdef HAVE_SYS_SIGLIST
    FIM_FBI_PRINTF("Oops: %s\n",sys_siglist[termsig]);
#endif /* HAVE_SYS_SIGLIST */
    std::exit(FIM_PERR_OOPS);
}



}
