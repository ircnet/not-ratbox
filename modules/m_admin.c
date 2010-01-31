/*
 *  ircd-ratbox: A slightly useful ircd.
 *  m_admin.c: Sends administrative information to a user.
 *
 *  Copyright (C) 1990 Jarkko Oikarinen and University of Oulu, Co Center
 *  Copyright (C) 1996-2002 Hybrid Development Team
 *  Copyright (C) 2002-2005 ircd-ratbox development team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
 *
 *  $Id$
 */

#include "stdinc.h"
#include "struct.h"
#include "client.h"
#include "ircd.h"
#include "numeric.h"
#include "s_conf.h"
#include "s_serv.h"
#include "send.h"
#include "parse.h"
#include "hook.h"
#include "modules.h"
#include "match.h"

static int m_admin(struct Client *, struct Client *, int, const char **);
static int mr_admin(struct Client *, struct Client *, int, const char **);
static int ms_admin(struct Client *, struct Client *, int, const char **);
static void do_admin(struct Client *source_p);

static void admin_spy(struct Client *);

struct Message admin_msgtab = {
	"ADMIN", 0, 0, 0, MFLG_SLOW | MFLG_UNREG,
	{{mr_admin, 0}, {m_admin, 0}, {ms_admin, 0}, mg_ignore, mg_ignore, {ms_admin, 0}}
};

int doing_admin_hook;

mapi_clist_av2 admin_clist[] = { &admin_msgtab, NULL };

mapi_hlist_av2 admin_hlist[] = {
	{"doing_admin", &doing_admin_hook},
	{NULL, NULL}
};

DECLARE_MODULE_AV2(admin, NULL, NULL, admin_clist, admin_hlist, NULL, "$Revision$");

/*
 * mr_admin - ADMIN command handler
 *      parv[0] = sender prefix   
 *      parv[1] = servername   
 */
static int
mr_admin(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	static time_t last_used = 0L;

	if((last_used + ConfigFileEntry.pace_wait) > rb_current_time())
	{
		sendto_one(source_p, form_str(RPL_LOAD2HI),
			   me.name, EmptyString(source_p->name) ? "*" : source_p->name, "ADMIN");
		return 0;
	}
	else
		last_used = rb_current_time();

	do_admin(source_p);

	return 0;
}

/*
 * m_admin - ADMIN command handler
 *      parv[0] = sender prefix
 *      parv[1] = servername
 */
static int
m_admin(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	static time_t last_used = 0L;

	if(parc > 1)
	{
		if((last_used + ConfigFileEntry.pace_wait_simple) > rb_current_time())
		{
			sendto_one(source_p, form_str(RPL_LOAD2HI),
				   me.name, source_p->name, "ADMIN");
			return 0;
		}
		else
			last_used = rb_current_time();

		if(hunt_server(client_p, source_p, ":%s ADMIN :%s", 1, parc, parv) != HUNTED_ISME)
			return 0;
	}

	do_admin(source_p);

	return 0;
}


/*
 * ms_admin - ADMIN command handler, used for OPERS as well
 *      parv[0] = sender prefix
 *      parv[1] = servername
 */
static int
ms_admin(struct Client *client_p, struct Client *source_p, int parc, const char *parv[])
{
	if(hunt_server(client_p, source_p, ":%s ADMIN :%s", 1, parc, parv) != HUNTED_ISME)
		return 0;

	do_admin(source_p);

	return 0;
}


/*
 * do_admin
 *
 * inputs	- pointer to client to report to
 * output	- none
 * side effects	- admin info is sent to client given
 */
static void
do_admin(struct Client *source_p)
{
	const char *myname;
	const char *nick;

	if(IsClient(source_p))
		admin_spy(source_p);

	myname = get_id(&me, source_p);
	nick = EmptyString(source_p->name) ? "*" : get_id(source_p, source_p);
	SetCork(source_p);
	sendto_one(source_p, form_str(RPL_ADMINME), myname, nick, me.name);
	if(AdminInfo.name != NULL)
		sendto_one(source_p, form_str(RPL_ADMINLOC1), myname, nick, AdminInfo.name);
	if(AdminInfo.description != NULL)
		sendto_one(source_p, form_str(RPL_ADMINLOC2), myname, nick, AdminInfo.description);
	if(AdminInfo.email != NULL)
		sendto_one(source_p, form_str(RPL_ADMINEMAIL), myname, nick, AdminInfo.email);
	ClearCork(source_p);
	send_pop_queue(source_p);
}

/* admin_spy()
 *
 * input	- pointer to client
 * output	- none
 * side effects - event doing_admin is called
 */
static void
admin_spy(struct Client *source_p)
{
	hook_data hd;

	hd.client = source_p;
	hd.arg1 = hd.arg2 = NULL;

	call_hook(doing_admin_hook, &(hook_data){.client = source_p, .arg1 = NULL, .arg2 = NULL});
}
