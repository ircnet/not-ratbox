/*
 *  sidmap.c: Channel sidmaps.
 *
 *  Copyright (C) 2010 !ratbox development team
 *  $Id$
 */

#include "stdinc.h"
#include "ratbox_lib.h"
#include "match.h"
#include "ircd.h"
#include "numeric.h"
#include "send.h"
#include "hash.h"
#include "struct.h"
#include "sidmap.h"
#include "channel.h"

#define SCACHE_MAX_BITS 8
#define SCACHE_MAX (1<<SCACHE_MAX_BITS)

#define hash_id(x) (fnv_hash((const unsigned char *)(x), U_MAX_BITS, 0))


#define SMW_BYTES		(sizeof(sidmap_t))
#define SMW_BITS	(SMW_BYTES*8)
#define SM_WORDS(n)    (((n)+SMW_BITS-1)/SMW_BITS)
                               

static rb_dlink_list scache_hash[SCACHE_MAX];
static sidmap_t *sm_sidmap;
static int sid_count = 0;
static int sid_words = 0;

struct scache_entry
{
	rb_dlink_node node;
	char *name;
	int id;
};

static int sidcache_add(const char *name)
{
	struct scache_entry *sc;
	unsigned int hashv;
	rb_dlink_node *ptr;

	if(EmptyString(name))
		return -1;

	hashv = hash_id(name);

	RB_DLINK_FOREACH(ptr, scache_hash[hashv].head)
	{
		sc = ptr->data;
		if(!irccmp(sc->name, name))
			return sc->id;
	}

	sc = rb_malloc(sizeof(struct scache_entry));
	sc->name = rb_strdup(name);
	sc->id = sid_count++;
	rb_dlinkAdd(sc, &sc->node, &scache_hash[hashv]);
	return sc->id;
}

static void sidmap_realloc(void **smp, int nw)
{
	char *p = *smp = rb_realloc(*smp, SMW_BYTES * nw);
	memset(p + SMW_BYTES * sid_words, 0, (nw - sid_words) * SMW_BYTES);
}

static void check_words(int id)
{
	rb_dlink_node *ptr;

	int nw = SM_WORDS(id);
	if (nw <= sid_words)
		return;

	sidmap_realloc((void **) &sm_sidmap, nw);

	RB_DLINK_FOREACH(ptr, global_channel_list.head)
	{
		struct Channel *chptr = ptr->data;

		if (!chptr->sidmap)
			return;

		sidmap_realloc(&chptr->sidmap, nw);
	}

	sid_words = nw;
}

/* mark the sid in channel */
int sidmap_mark(struct Channel *chptr, const char *sid)
{
	int id = sidcache_add(sid);
	check_words(id);
	if (!chptr->sidmap)
		chptr->sidmap = rb_malloc(sid_words * SMW_BYTES);
	((sidmap_t *) chptr->sidmap)[id/SMW_BITS] |= (1<<(id%SMW_BITS));
	sm_sidmap[id/SMW_BITS] |= (1<<(id%SMW_BITS));
	return id;
}

/* check if channel is sidmapped */
int sidmap_check(struct Channel *chptr)
{
	int i;

	if (!chptr)
		return 0;

	if (!chptr->channelts)
		return 0;

	if (!chptr->sidmap)
		return 0;

	for (i = 0; i < sid_words; i++)
		if (((sidmap_t *) chptr->sidmap)[i] & sm_sidmap[i])
			return 1;

	if (chptr->sidmap)
		rb_free(chptr->sidmap);
	chptr->sidmap = NULL;
	return 0;
}

/* sid is present, clear it */
int sidmap_clear(struct Client *srv)
{
	int id = sidcache_add(srv->id);
	check_words(id);
	sm_sidmap[id/SMW_BITS] &= ~(1<<(id%SMW_BITS));
	return id;
}

