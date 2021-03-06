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
#include "cwmp.h"
#include "diagnostic.h" 
#include "ubus.h"
#include "dmcwmp.h"
#include "dmuci.h"
#include "dmubus.h"
#include "dmcommon.h"
#include "softwaremodules.h"

struct software_module cur_software_module = {0};
inline int entry_softwaremodules_deploymentunit_instance(struct dmctx *ctx, char *idu);

inline int init_args_du_entry(struct dmctx *ctx, struct uci_section *s)
{
	struct software_module *args = &cur_software_module;
	ctx->args = (void *)args;
	args->softsection = s;
	return 0;
}

int get_deploymentunit_uuid(char *refparam, struct dmctx *ctx, char **value)
{
	struct software_module *softawreargs = (struct software_module *)ctx->args;

	dmuci_get_value_by_section_string(softawreargs->softsection, "uuid", value);
	
	return 0;
}

int get_deploymentunit_name(char *refparam, struct dmctx *ctx, char **value)
{
	struct software_module *softawreargs = (struct software_module *)ctx->args;

	dmuci_get_value_by_section_string(softawreargs->softsection, "name", value);
	return 0;
}

int get_deploymentunit_resolved(char *refparam, struct dmctx *ctx, char **value)
{
	struct software_module *softawreargs = (struct software_module *)ctx->args;

	dmuci_get_value_by_section_string(softawreargs->softsection, "resolved", value);
	return 0;
}

int get_deploymentunit_url(char *refparam, struct dmctx *ctx, char **value)
{
	struct software_module *softawreargs = (struct software_module *)ctx->args;

	dmuci_get_value_by_section_string(softawreargs->softsection, "url", value);
	return 0;
}

int get_deploymentunit_vendor(char *refparam, struct dmctx *ctx, char **value)
{
	struct software_module *softawreargs = (struct software_module *)ctx->args;

	dmuci_get_value_by_section_string(softawreargs->softsection, "vendor", value);
	return 0;
}

int get_deploymentunit_version(char *refparam, struct dmctx *ctx, char **value)
{
	struct software_module *softawreargs = (struct software_module *)ctx->args;

	dmuci_get_value_by_section_string(softawreargs->softsection, "version", value);
	return 0;
}

int get_deploymentunit_execution_env_ref(char *refparam, struct dmctx *ctx, char **value)
{
	struct software_module *softawreargs = (struct software_module *)ctx->args;

	dmuci_get_value_by_section_string(softawreargs->softsection, "execution_env_ref", value);
	return 0;
}

char *add_softwaremodules_deploymentunit(char *uuid, char*url, char *username, char *password, char *name, char *version)
{
	char *value;
	char *instance;
	struct uci_section *deploymentsection = NULL;
	char duname[16];
	
	
	uci_path_foreach_option_eq(icwmpd, "dmmap", "deploymentunit", "UUID", uuid, deploymentsection) {
		DMUCI_SET_VALUE_BY_SECTION(icwmpd, deploymentsection, "URL", url);
		DMUCI_SET_VALUE_BY_SECTION(icwmpd,deploymentsection, "URL", url);
		DMUCI_SET_VALUE_BY_SECTION(icwmpd,deploymentsection, "Name", name);
		DMUCI_SET_VALUE_BY_SECTION(icwmpd,deploymentsection, "Version", version);
		DMUCI_SET_VALUE_BY_SECTION(icwmpd,deploymentsection, "username", username);
		DMUCI_SET_VALUE_BY_SECTION(icwmpd,deploymentsection, "password", password);
		DMUCI_SET_VALUE_BY_SECTION(icwmpd,deploymentsection, "Resolved", "1");
		dmuci_get_value_by_section_string(deploymentsection, "duinstance", &instance);
		goto end;
	}
	instance = get_last_instance(DMMAP, "deploymentunit", "duinstance");
	if (!instance)
		sprintf(duname, "du%d", 0);
	else
		sprintf(duname, "du%s", instance);
	DMUCI_SET_VALUE(icwmpd, "dmmap", duname, NULL, "deploymentunit");
	DMUCI_SET_VALUE(icwmpd, "dmmap", duname, "UUID", uuid);
	DMUCI_SET_VALUE(icwmpd, "dmmap", duname, "URL", url);
	DMUCI_SET_VALUE(icwmpd, "dmmap", duname, "Name", name);
	DMUCI_SET_VALUE(icwmpd, "dmmap", duname, "Version", version);
	DMUCI_SET_VALUE(icwmpd, "dmmap", duname, "username", username);
	DMUCI_SET_VALUE(icwmpd, "dmmap", duname, "password", password);
	DMUCI_SET_VALUE(icwmpd, "dmmap", duname, "Resolved", "1");
	instance = get_last_instance(DMMAP, "deploymentunit", "duinstance");
	return instance;
end:
	return instance;
}

int update_softwaremodules_url(char *uuid, char *url)
{
	struct uci_section *s = NULL;
	uci_path_foreach_option_eq(icwmpd, "dmmap", "deploymentunit", "UUID", uuid, s) {
		DMUCI_SET_VALUE_BY_SECTION(icwmpd, s, "URL", url);
		return 1;
	}
	return 0;
}

char *get_softwaremodules_uuid(char *url)
{
	char *uuid;
	struct uci_section *s = NULL;

	uci_path_foreach_option_eq(icwmpd, "dmmap", "deploymentunit", "url", url, s) {
		dmuci_get_value_by_section_string(s, "UUID", &uuid);
	
		return uuid;
	}
	return "";
}

char *get_softwaremodules_url(char *uuid)
{	
	char *url;
	struct uci_section *s = NULL;

	uci_path_foreach_option_eq(icwmpd, "dmmap", "deploymentunit", "uuid", uuid, s) {
		dmuci_get_value_by_section_string(s, "URL", &url);
		return url;
	}
	return "";
}

char *get_softwaremodules_username(char *uuid)
{	
	char *url;
	struct uci_section *s = NULL;

	uci_path_foreach_option_eq(icwmpd, "dmmap", "deploymentunit", "uuid", uuid, s) {
		dmuci_get_value_by_section_string(s, "username", &url);
		return url;
	}
	return "";
}

char *get_softwaremodules_pass(char *uuid)
{	
	char *url;
	struct uci_section *s = NULL;

	uci_path_foreach_option_eq(icwmpd, "dmmap", "deploymentunit", "uuid", uuid, s) {
		dmuci_get_value_by_section_string(s, "password", &url);
		return url;
	}
	return "";
}

char *get_softwaremodules_instance(char *uuid)
{	
	char *url;
	struct uci_section *s = NULL;

	uci_path_foreach_option_eq(icwmpd, "dmmap", "deploymentunit", "uuid", uuid, s) {
		dmuci_get_value_by_section_string(s, "duinstance", &url);
		return url;
	}
	return "";
}

char *get_softwaremodules_name(char *uuid)
{	
	char *name;
	struct uci_section *s = NULL;

	uci_path_foreach_option_eq(icwmpd, "dmmap", "deploymentunit", "uuid", uuid, s) {
		dmuci_get_value_by_section_string(s, "Name", &name);
		return name;
	}
	return "";
}

char *get_softwaremodules_version(char *uuid)
{	
	char *version;
	struct uci_section *s = NULL;

	uci_path_foreach_option_eq(icwmpd, "dmmap", "deploymentunit", "uuid", uuid, s) {
		dmuci_get_value_by_section_string(s, "Version", &version);
		return version;
	}
	return "";
}

inline int entry_softwaremodules_deploymentunit(struct dmctx *ctx)
{
	char *idu = NULL, *idu_last = NULL;
	char *permission = "1";
	struct uci_section *s = NULL;

	uci_path_foreach_sections(icwmpd, "dmmap", "deploymentunit", s) {
		init_args_du_entry(ctx, s);
		idu = handle_update_instance(1, ctx, &idu_last, update_instance_alias_icwmpd, 3, s, "duinstance", "duinstance_alias");
		SUBENTRY(entry_softwaremodules_deploymentunit_instance, ctx, idu);
	}
	return 0;
}

inline int entry_softwaremodules_deploymentunit_instance(struct dmctx *ctx, char *idu)
{	
	IF_MATCH(ctx, DMROOT"SoftwareModules.DeploymentUnit.%s.", idu) {
		DMOBJECT(DMROOT"SoftwareModules.DeploymentUnit.%s.", ctx, "0", 1, NULL, NULL, NULL, idu);
		DMPARAM("UUID", ctx, "0", get_deploymentunit_uuid, NULL, NULL, 0, 1, UNDEF, NULL);
		DMPARAM("Name", ctx, "0", get_deploymentunit_name, NULL, NULL, 0, 1, UNDEF, NULL);
		DMPARAM("Resolved", ctx, "0", get_deploymentunit_resolved, NULL, "xsd:boolean", 0, 1, UNDEF, NULL);
		DMPARAM("URL", ctx, "0", get_deploymentunit_url, NULL, NULL, 0, 1, UNDEF, NULL);
		DMPARAM("Vendor", ctx, "0", get_deploymentunit_vendor, NULL, NULL, 0, 1, UNDEF, NULL);
		DMPARAM("Version", ctx, "0", get_deploymentunit_version, NULL, NULL, 0, 1, UNDEF, NULL);
		DMPARAM("ExecutionEnvRef", ctx, "0", get_deploymentunit_execution_env_ref, NULL, NULL, 0, 1, UNDEF, NULL);			  
		return 0;
	}
	return FAULT_9005;
}

int entry_method_root_software_modules(struct dmctx *ctx)
{
	IF_MATCH(ctx, DMROOT"SoftwareModules.") {
		DMOBJECT(DMROOT"SoftwareModules.", ctx, "0", 1, NULL, NULL, NULL);
		DMOBJECT(DMROOT"SoftwareModules.DeploymentUnit.", ctx, "0", 1, NULL, NULL, NULL);
		SUBENTRY(entry_softwaremodules_deploymentunit, ctx);
		return 0;
	}
	return FAULT_9005;
}

