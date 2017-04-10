/*
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	Copyright (C) 2012-2014 PIVA SOFTWARE (www.pivasoftware.com)
 *		Author: Imen Bhiri <imen.bhiri@pivasoftware.com>
 *		Author: Feten Besbes <feten.besbes@pivasoftware.com>
 */

#include <ctype.h>
#include <uci.h>
#include <stdio.h>
#include "dmcwmp.h"
#include "dmuci.h"
#include "dmubus.h"
#include "dmcommon.h"
#include "softwaremodules.h"

struct software_module cur_software_module = {0};
inline int browsesoftwaremodules_deploymentunitInst(struct dmctx *dmctx, DMNODE *parent_node, void *prev_data, char *prev_instance);

inline int init_args_du_entry(struct dmctx *ctx, struct uci_section *s)
{
	struct software_module *args = &cur_software_module;
	args->softsection = s;
	return 0;
}

int get_deploymentunit_uuid(char *refparam, struct dmctx *ctx, char **value)
{
	struct software_module *softawreargs = &cur_software_module;

	dmuci_get_value_by_section_string(softawreargs->softsection, "uuid", value);
	
	return 0;
}

int get_deploymentunit_name(char *refparam, struct dmctx *ctx, char **value)
{
	struct software_module *softawreargs = &cur_software_module;

	dmuci_get_value_by_section_string(softawreargs->softsection, "name", value);
	return 0;
}

int get_deploymentunit_resolved(char *refparam, struct dmctx *ctx, char **value)
{
	struct software_module *softawreargs = &cur_software_module;

	dmuci_get_value_by_section_string(softawreargs->softsection, "resolved", value);
	return 0;
}

int get_deploymentunit_url(char *refparam, struct dmctx *ctx, char **value)
{
	struct software_module *softawreargs = &cur_software_module;

	dmuci_get_value_by_section_string(softawreargs->softsection, "url", value);
	return 0;
}

int get_deploymentunit_vendor(char *refparam, struct dmctx *ctx, char **value)
{
	struct software_module *softawreargs = &cur_software_module;

	dmuci_get_value_by_section_string(softawreargs->softsection, "vendor", value);
	return 0;
}

int get_deploymentunit_version(char *refparam, struct dmctx *ctx, char **value)
{
	struct software_module *softawreargs = &cur_software_module;

	dmuci_get_value_by_section_string(softawreargs->softsection, "version", value);
	return 0;
}

int get_deploymentunit_execution_env_ref(char *refparam, struct dmctx *ctx, char **value)
{
	struct software_module *softawreargs = &cur_software_module;

	dmuci_get_value_by_section_string(softawreargs->softsection, "execution_env_ref", value);
	return 0;
}

char *add_softwaremodules_deploymentunit(char *uuid, char*url, char *username, char *password, char *name, char *version)
{
	char *value;
	char *instance;
	struct uci_section *deploymentsection = NULL;
	char duname[16];
	
	
	uci_foreach_option_eq("dmmap", "deploymentunit", "UUID", uuid, deploymentsection) {
		dmuci_set_value_by_section(deploymentsection, "URL", url);
		dmuci_set_value_by_section(deploymentsection, "URL", url);
		dmuci_set_value_by_section(deploymentsection, "Name", name);
		dmuci_set_value_by_section(deploymentsection, "Version", version);
		dmuci_set_value_by_section(deploymentsection, "username", username);
		dmuci_set_value_by_section(deploymentsection, "password", password);
		dmuci_set_value_by_section(deploymentsection, "Resolved", "1");
		dmuci_get_value_by_section_string(deploymentsection, "duinstance", &instance);
		goto end;
	}
	instance = get_last_instance("dmmap", "deploymentunit", "duinstance");
	if (!instance)
		sprintf(duname, "du%d", 0);
	else
		sprintf(duname, "du%s", instance);
	dmuci_set_value("dmmap", duname, NULL, "deploymentunit");
	dmuci_set_value("dmmap", duname, "UUID", uuid);
	dmuci_set_value("dmmap", duname, "URL", url);
	dmuci_set_value("dmmap", duname, "Name", name);
	dmuci_set_value("dmmap", duname, "Version", version);
	dmuci_set_value("dmmap", duname, "username", username);
	dmuci_set_value("dmmap", duname, "password", password);
	dmuci_set_value("dmmap", duname, "Resolved", "1");
	instance = get_last_instance("dmmap", "deploymentunit", "duinstance");
	return instance;
end:
	return instance;
}

int update_softwaremodules_url(char *uuid, char *url)
{
	struct uci_section *s = NULL;
	uci_foreach_option_eq("dmmap", "deploymentunit", "UUID", uuid, s) {
		dmuci_set_value_by_section(s, "URL", url);
		return 1;
	}
	return 0;
}

char *get_softwaremodules_uuid(char *url)
{
	char *uuid;
	struct uci_section *s = NULL;

	uci_foreach_option_eq("dmmap", "deploymentunit", "url", url, s) {
		dmuci_get_value_by_section_string(s, "UUID", &uuid);
	
		return uuid;
	}
	return "";
}

char *get_softwaremodules_url(char *uuid)
{	
	char *url;
	struct uci_section *s = NULL;

	uci_foreach_option_eq("dmmap", "deploymentunit", "uuid", uuid, s) {
		dmuci_get_value_by_section_string(s, "URL", &url);
		return url;
	}
	return "";
}

char *get_softwaremodules_username(char *uuid)
{	
	char *url;
	struct uci_section *s = NULL;

	uci_foreach_option_eq("dmmap", "deploymentunit", "uuid", uuid, s) {
		dmuci_get_value_by_section_string(s, "username", &url);
		return url;
	}
	return "";
}

char *get_softwaremodules_pass(char *uuid)
{	
	char *url;
	struct uci_section *s = NULL;

	uci_foreach_option_eq("dmmap", "deploymentunit", "uuid", uuid, s) {
		dmuci_get_value_by_section_string(s, "password", &url);
		return url;
	}
	return "";
}

char *get_softwaremodules_instance(char *uuid)
{	
	char *url;
	struct uci_section *s = NULL;

	uci_foreach_option_eq("dmmap", "deploymentunit", "uuid", uuid, s) {
		dmuci_get_value_by_section_string(s, "duinstance", &url);
		return url;
	}
	return "";
}

char *get_softwaremodules_name(char *uuid)
{	
	char *name;
	struct uci_section *s = NULL;

	uci_foreach_option_eq("dmmap", "deploymentunit", "uuid", uuid, s) {
		dmuci_get_value_by_section_string(s, "Name", &name);
		return name;
	}
	return "";
}

char *get_softwaremodules_version(char *uuid)
{	
	char *version;
	struct uci_section *s = NULL;

	uci_foreach_option_eq("dmmap", "deploymentunit", "uuid", uuid, s) {
		dmuci_get_value_by_section_string(s, "Version", &version);
		return version;
	}
	return "";
}

/*************************************************************
 * ENTRY METHOD
/*************************************************************/
DMOBJ tSoftwareModulesObj[] = {
/* OBJ, permission, addobj, delobj, browseinstobj, finform, notification, nextobj, leaf*/
{"DeploymentUnit", &DMREAD, NULL, NULL, NULL, browsesoftwaremodules_deploymentunitInst, NULL, NULL, NULL, tDeploymentUnitParams, NULL},
{0}
};

DMLEAF tDeploymentUnitParams[] = {
/* PARAM, permission, type, getvlue, setvalue, forced_inform, notification*/
{"UUID", &DMREAD, DMT_STRING, get_deploymentunit_uuid, NULL, NULL, NULL},
{"Name", &DMREAD, DMT_STRING, get_deploymentunit_name, NULL, NULL, NULL},
{"Resolved", &DMREAD, DMT_BOOL, get_deploymentunit_resolved, NULL, NULL, NULL},
{"URL", &DMREAD, DMT_STRING, get_deploymentunit_url, NULL, NULL, NULL},
{"Vendor", &DMREAD, DMT_STRING, get_deploymentunit_vendor, NULL, NULL, NULL},
{"Version", &DMREAD, DMT_STRING, get_deploymentunit_version, NULL, NULL, NULL},
{"ExecutionEnvRef", &DMREAD, DMT_STRING, get_deploymentunit_execution_env_ref, NULL, NULL, NULL},
{0}
};


inline int browsesoftwaremodules_deploymentunitInst(struct dmctx *dmctx, DMNODE *parent_node, void *prev_data, char *prev_instance)
{
	char *idu = NULL, *idu_last = NULL;
	char *permission = "1";
	struct uci_section *s = NULL;

	uci_foreach_sections("dmmap", "deploymentunit", s) {
		init_args_du_entry(dmctx, s);
		idu = handle_update_instance(1, dmctx, &idu_last, update_instance_alias, 3, s, "duinstance", "duinstance_alias");
		if (DM_LINK_INST_OBJ(dmctx, parent_node, NULL, idu) == DM_STOP)
			break;
	}
	DM_CLEAN_ARGS(cur_software_module);
	return 0;
}


