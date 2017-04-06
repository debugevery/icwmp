#ifndef __DMENTRY_H__
#define __DMENTRY_H__

#include "dmcwmp.h"
extern struct list_head head_package_change;

enum ctx_init_enum {
	CTX_INIT_ALL,
	CTX_INIT_SUB
};

int dm_ctx_init(struct dmctx *ctx, unsigned int amd_version, unsigned int instance_mode);
int dm_ctx_init_sub(struct dmctx *ctx, unsigned int amd_version, unsigned int instance_mode);
int dm_entry_param_method(struct dmctx *ctx, int cmd, char *inparam, char *arg1, char *arg2);
int dm_entry_apply(struct dmctx *ctx, int cmd, char *arg1, char *arg2);
int dm_entry_load_enabled_notify(unsigned int amd_version, int instance_mode);
int adm_entry_get_linker_param(struct dmctx *ctx, char *param, char *linker, char **value);
int adm_entry_get_linker_value(struct dmctx *ctx, char *param, char **value);
int dm_entry_restart_services();
int dm_ctx_clean(struct dmctx *ctx);
int dm_ctx_clean_sub(struct dmctx *ctx);
void dm_entry_cli(int argc, char** argv, unsigned int amd_version, unsigned int instance_mode);
void wepkey_cli(int argc, char** argv);
#endif
