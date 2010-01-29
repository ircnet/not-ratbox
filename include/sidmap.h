
#ifndef INCLUDED_SIDMAP_H
#define INCLUDED_SIDMAP_H

typedef unsigned long sidmap_t;
int sidmap_clear(struct Client *srv);
int sidmap_check(struct Channel *chptr);
int sidmap_mark(struct Channel *chptr, const char *sid);

#endif

