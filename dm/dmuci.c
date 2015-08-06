/*
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	Copyright (C) 2015 Inteno Broadband Technology AB
 *	  Author MOHAMED Kallel <mohamed.kallel@pivasoftware.com>
 *	  Author Imen Bhiri <imen.bhiri@pivasoftware.com>
 *	  Author Feten Besbes <feten.besbes@pivasoftware.com>
 *
 */

#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <uci.h>
#include "dmuci.h"
#include "dmmem.h"

struct uci_context *uci_ctx;

char *dmuci_list_to_string(struct uci_list *list, char *delimitor)
{
	struct uci_element *e = NULL;
	char val[512] = {0};
	int del_len = strlen(delimitor);
	if (list) {
		uci_foreach_element(list, e)
		{
			int len = strlen(val);
			if (len != 0) {
				memcpy(val + len, delimitor, del_len);
				strcpy(val + len + del_len, e->name);
			}
			else
				strcpy(val, e->name);
		}
		return (dmstrdup(val));
	}
	else 
	{
		return dmstrdup("");
	}
}

static inline bool check_section_name(const char *str, bool name)
{
	if (!*str)
		return false;
	for (; *str; str++) {
		unsigned char c = *str;
		if (isalnum(c) || c == '_') 
			continue;
		if (name || (c < 33) || (c > 126)) 
			return false;
	}
	return true;
}

//////////////////////////////
void add_list_package_change(struct list_head *clist, char *package)
{
	struct package_change *pc;
	list_for_each_entry(pc, clist, list) {
		if (strcmp(pc->package, package) == 0)
			return;
	}
	pc = calloc(1, sizeof(struct package_change));//TODO !!!!! Do not use dmcalloc here
	list_add_tail(&pc->list, clist);
	pc->package = strdup(package); //TODO !!!!! Do not use dmstrdup here
}

void free_all_list_package_change(struct list_head *clist)
{
	struct package_change *pc;
	while (clist->next != clist) {
		pc = list_entry(clist->next, struct package_change, list);
		list_del(&pc->list);
		free(pc->package);//TODO !!!!! Do not use dmfree here
		free(pc);//TODO !!!!! Do not use dmfree here
	}
}
///////////////////////////

/**** UCI LOOKUP ****/
int dmuci_lookup_ptr(struct uci_context *ctx, struct uci_ptr *ptr, char *package, char *section, char *option, char *value)
{
	/*package*/
	if (!package && !package[0])
		return -1;
	ptr->package = package;

	/*section*/
	if (!section || !section[0]) {
		ptr->target = UCI_TYPE_PACKAGE;
		goto lookup;
	}
	ptr->section = section;
	if (ptr->section &&  !check_section_name(ptr->section , true))
		ptr->flags |= UCI_LOOKUP_EXTENDED;

	/*option*/
	if (!option || !option[0]) {
		ptr->target = UCI_TYPE_SECTION;
		goto lookup;
	}
	ptr->target = UCI_TYPE_OPTION;
	ptr->option = option;

	/*value*/
	if (!value || !value[0]) {
		goto lookup;
	}
	ptr->value = value;

lookup:
	if (uci_lookup_ptr(ctx, ptr, NULL, true) != UCI_OK || !UCI_LOOKUP_COMPLETE) {
		return -1;
	}
	return 0;
}

/**** UCI GET *****/
int dmuci_get_section_type(char *package, char *section,char **value)
{
	struct uci_element *e;
	struct uci_ptr ptr = {0};
	if (dmuci_lookup_ptr(uci_ctx, &ptr, package, section, NULL, NULL)) {
		*value = dmstrdup("");
		return -1;
	}
	if (ptr.s) {
		*value = dmstrdup(ptr.s->type);
	} else {
		*value = dmstrdup("");
		return -1;
	}
	return 0;
}

int dmuci_get_option_value_string(char *package, char *section, char *option, char **value)
{
	struct uci_element *e;
	struct uci_ptr ptr = {0};
	if (dmuci_lookup_ptr(uci_ctx, &ptr, package, section, option, NULL)) {
		*value = dmstrdup("");
		return -1;
	}
	if (ptr.o) {
		*value = dmstrdup(ptr.o->v.string);
	} else {
		*value = dmstrdup("");
		return -1;
	}
	return 0;
}

int dmuci_get_option_value_list(char *package, char *section, char *option, struct uci_list **value)
{
	struct uci_element *e;
	struct uci_ptr ptr = {0};
	*value = NULL;
	if (dmuci_lookup_ptr(uci_ctx, &ptr, package, section, option, NULL)) {
		return -1;
	}
	if (ptr.o) {
		*value = &ptr.o->v.list;
	} else {
		return -1;
	}
	return 0;
}


struct uci_option *dmuci_get_option_ptr(char *cfg_path, char *package, char *section, char *option)
{
	int ret;
	struct uci_option *o = NULL;
	struct uci_element *e;
	struct uci_ptr ptr = {0};
	char *oconfdir;

	oconfdir = uci_ctx->confdir;
	uci_ctx->confdir = cfg_path;
	if (dmuci_lookup_ptr(uci_ctx, &ptr, package, section, option, NULL)) {
		goto end;
	}
	e = ptr.last;
	switch(e->type) {
		case UCI_TYPE_OPTION:
			o = ptr.o;
		default:
			break;
	}
end:
	uci_ctx->confdir = oconfdir;
	return o;
}

int db_get_value_string(char *package, char *section, char *option, char **value)
{
	struct uci_option *o;
	o = dmuci_get_option_ptr(DB_CONFIG, package, section, option);
	if (o) {
		*value = dmstrdup(o->v.string);
	} else {
		*value = dmstrdup("");
		return -1;
	}
	return 0;
}

int db_get_value_list(char *package, char *section, char *option, struct uci_list **value)
{
	struct uci_option *o;
	*value = NULL;

	o = dmuci_get_option_ptr(DB_CONFIG, package, section, option);
	if (o) {
		*value = &o->v.list;
	} else {
		return -1;
	}
	return 0;
}

/**** UCI Commit *****/
int dmuci_commit_package(char *package)
{
	struct uci_ptr ptr = {0};

	if (uci_lookup_ptr(uci_ctx, &ptr, package, true) != UCI_OK) {
		return -1;
	}
	if (uci_commit(uci_ctx, &ptr.p, false) != UCI_OK) {
		return -1;
	}

	return 0;
}

int dmuci_commit(void)
{
	char **configs = NULL;
	char **p;

	if ((uci_list_configs(uci_ctx, &configs) != UCI_OK) || !configs) {
		return -1;
	}
	for (p = configs; *p; p++) {
		dmuci_commit_package(*p);
	}
	return 0;
}

/**** UCI REVERT *****/
int dmuci_revert_package(char *package)
{
	struct uci_ptr ptr = {0};

	if (uci_lookup_ptr(uci_ctx, &ptr, package, true) != UCI_OK) {
		return -1;
	}
	if (uci_revert(uci_ctx, &ptr) != UCI_OK) {
		return -1;
	}

	return 0;
}

int dmuci_revert(void)
{
	char **configs = NULL;
	char **p;

	if ((uci_list_configs(uci_ctx, &configs) != UCI_OK) || !configs) {
		return -1;
	}
	for (p = configs; *p; p++) {
		dmuci_revert_package(*p);
	}
	return 0;
}

/**** UCI CHANGES PACKAGES *****/
int dmuci_change_packages(struct list_head *clist)
{
	char **configs = NULL;
	char **p;
	if ((uci_list_configs(uci_ctx, &configs) != UCI_OK) || !configs) {
		return -1;
	}
	for (p = configs; *p; p++) {
		struct uci_ptr ptr = {0};
		if (uci_lookup_ptr(uci_ctx, &ptr, *p, true) != UCI_OK) {
			continue;
		}
		if (ptr.p->saved_delta.next == &ptr.p->saved_delta)
			continue;
		add_list_package_change(clist, *p);
	}
	return 0;
}

/**** UCI SET *****/
char *dmuci_set_value(char *package, char *section, char *option, char *value)
{
	struct uci_ptr ptr = {0};

	if (dmuci_lookup_ptr(uci_ctx, &ptr, package, section, option, value)) {
		return "";
	}

	if (uci_set(uci_ctx, &ptr) != UCI_OK) {
		return "";
	}
	if (ptr.o) {
		return ptr.o->v.string;
	}
	return "";
}

/**** UCI ADD LIST *****/
int dmuci_add_list_value(char *package, char *section, char *option, char *value)
{

	struct uci_ptr ptr = {0};

	if (dmuci_lookup_ptr(uci_ctx, &ptr, package, section, option, value))
		return -1;

	if (uci_add_list(uci_ctx, &ptr) != UCI_OK)
		return -1;

	return 0;
}

/**** UCI DEL LIST *****/
int dmuci_del_list_value(char *package, char *section, char *option, char *value)
{
	struct uci_ptr ptr = {0};

	if (dmuci_lookup_ptr(uci_ctx, &ptr, package, section, option, value))
		return -1;

	if (uci_del_list(uci_ctx, &ptr) != UCI_OK)
		return -1;

	return 0;
}

/****** UCI ADD *******/
int dmuci_add_section(char *package, char *stype, struct uci_section **s, char **value)
{
	struct uci_ptr ptr = {0};
	*s = NULL;

	if (dmuci_lookup_ptr(uci_ctx, &ptr, package, NULL, NULL, NULL)) {
		*value = dmstrdup("");
		return -1;
	}
	if (uci_add_section(uci_ctx, ptr.p, stype, s) != UCI_OK) {
		*value = dmstrdup("");
		return -1;
	}

	*value = dmstrdup((*s)->e.name);
	return 0;
}

/**** UCI DELETE *****/
int dmuci_delete(char *package, char *section, char *option, char *value)
{
	struct uci_ptr ptr = {0};

	if (dmuci_lookup_ptr(uci_ctx, &ptr, package, section, option, NULL))
		return -1;
	if (uci_delete(uci_ctx, &ptr) != UCI_OK)
		return -1;

	return 0;
}

/**** UCI LOOKUP by section pointer ****/

int dmuci_lookup_ptr_by_section(struct uci_context *ctx, struct uci_ptr *ptr, struct uci_section *s, char *option, char *value)
{
	struct uci_element *e;

	ptr->flags |= UCI_LOOKUP_DONE;

	/*package*/
	ptr->package = s->package->e.name;
	ptr->p = s->package;

	/* section */
	ptr->section = s->e.name;
	ptr->s = s;
	ptr->last = e;

	/*option*/
	if (!option || !option[0]) {
		ptr->target = UCI_TYPE_SECTION;
		goto lookup;
	}
	ptr->target = UCI_TYPE_OPTION;
	ptr->option = option;

	/*value*/
	if (!value || !value[0]) {
		goto lookup;
	}
	ptr->value = value;

lookup:
	if (ptr->option) {
		uci_foreach_element(&ptr->s->options, e) {
			if (strcmp(e->name, ptr->option) == 0) {
				ptr->o = uci_to_option(e);
				ptr->last = e;
				break;
			}
		}
	}
	ptr->flags |= UCI_LOOKUP_COMPLETE;
	return 0;
}

/**** UCI GET by section pointer*****/
int dmuci_get_value_by_section_string(struct uci_section *s, char *option, char **value)
{
	struct uci_element *e;
	struct uci_option *o;
	uci_foreach_element(&s->options, e) {
		o = (uci_to_option(e));
		if (!strcmp(o->e.name, option)) {
			*value = dmstrdup(o->v.string);
			return 0;
		}
	}
	*value = dmstrdup("");
	return -1;
}

int dmuci_get_value_by_section_list(struct uci_section *s, char *option, struct uci_list **value)
{
	struct uci_element *e;
	struct uci_option *o;
	uci_foreach_element(&s->options, e) {
		o = (uci_to_option(e));
		if (!strcmp(o->e.name, option)) {
			*value = &o->v.list;
			return 0;
		}
	}
	*value = NULL;
	return -1;
}

/**** UCI SET by section pointer ****/
char *dmuci_set_value_by_section(struct uci_section *s, char *option, char *value)
{
	struct uci_ptr up = {0};

	dmuci_lookup_ptr_by_section(uci_ctx, &up, s, option, value);
	if (uci_set(uci_ctx, &up) != UCI_OK)
		return "";

	if (up.o) {
		return up.o->v.string;
	}
	return "";

}

/**** UCI DELETE by section pointer *****/
int dmuci_delete_by_section(struct uci_section *s, char *option, char *value)
{
	struct uci_ptr up = {0};

	dmuci_lookup_ptr_by_section(uci_ctx, &up, s, option, value);

	if (uci_delete(uci_ctx, &up) != UCI_OK)
			return -1;

	return 0;
}

/**** UCI ADD LIST by section pointer *****/
int dmuci_add_list_value_by_section(struct uci_section *s, char *option, char *value)
{
	struct uci_ptr up = {0};

	dmuci_lookup_ptr_by_section(uci_ctx, &up, s, option, value);

	if (uci_add_list(uci_ctx, &up) != UCI_OK)
		return -1;

	return 0;
}

/**** UCI DEL LIST by section pointer *****/
int dmuci_del_list_value_by_section(struct uci_section *s, char *option, char *value)
{
	struct uci_ptr up = {0};

	dmuci_lookup_ptr_by_section(uci_ctx, &up, s, option, value);

	if (uci_del_list(uci_ctx, &up) != UCI_OK)
		return -1;

	return 0;
}

/**** UCI WALK SECTIONS *****/
struct uci_section *dmuci_walk_section (char *package, char *stype, void *arg1, void *arg2, int cmp , int (*filter)(struct uci_section *s, void *value), struct uci_section *prev_section, int walk) {
	struct uci_section *s = NULL;
	struct uci_element *e, *m;
	char *value;
	struct uci_list *list_value, *list_section;
	struct uci_ptr ptr = {0};
	if (walk == GET_FIRST_SECTION) {
		if (dmuci_lookup_ptr(uci_ctx, &ptr, package, NULL, NULL, NULL) != UCI_OK) {
			goto end;
		}
		list_section = &(ptr.p)->sections;
		e = list_to_element(list_section->next);
	}
	else {
		list_section = &prev_section->package->sections;
		e = list_to_element(prev_section->e.list.next);
	}
	while(&e->list != list_section) {
		s = uci_to_section(e);
		if (strcmp(s->type, stype) == 0) {
			switch(cmp) {
				case CMP_SECTION:
					goto end;
				case CMP_OPTION_EQUAL:
						dmuci_get_value_by_section_string(s, (char *)arg1, &value);
						if (strcmp(value, (char *)arg2) == 0)
							goto end;
					break;
				case CMP_OPTION_CONTAINING:
					dmuci_get_value_by_section_string(s, (char *)arg1, &value);
					if (strstr(value, (char *)arg2)) {						
						goto end;
					}
					break;
				case CMP_OPTION_CONT_WORD:
					dmuci_get_value_by_section_string(s, (char *)arg1, &value);					
					char *pch = strtok(value, " ");
					while (pch != NULL) {
						if (strcmp((char *)arg2, pch) == 0) {
							goto end;
						}
						pch = strtok(NULL, " ");
					}	
					break;
				case CMP_LIST_CONTAINING:
					dmuci_get_value_by_section_list(s, (char *)arg1, &list_value);
					if (list_value != NULL) {
						uci_foreach_element(list_value, m) {
							if (strcmp(m->name, (char *)arg2) == 0)
								goto end;
						}
					}										
					break;
				case CMP_FILTER_FUNC:
					if (filter(s, arg1) == 0)
						goto end;
					break;
				default:
					break;
			}
		}
		e = list_to_element(e->list.next);
		s = NULL;
	}
end:
	return s;
}
