/*
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	Copyright (C) 2012-2014 PIVA SOFTWARE (www.pivasoftware.com)
*		Author: Imen Bhiri <imen.bhiri@pivasoftware.com>
 *		Author: Feten Besbes <feten.besbes@pivasoftware.com>
 *		Author: Mohamed Kallel <mohamed.kallel@pivasoftware.com>
 *		Author: Anis Ellouze <anis.ellouze@pivasoftware.com>
 */

#include "dmcwmp.h"
#include "dmuci.h"

int get_pwr_mgmt_value_ethapd(char *refparam, struct dmctx *ctx, char **value)
{
	dmuci_get_option_value_string("power_mgmt", "power_mgmt", "ethapd", value);
	return 0;
}

int get_pwr_mgmt_value_eee(char *refparam, struct dmctx *ctx, char **value)
{
	dmuci_get_option_value_string("power_mgmt", "power_mgmt", "eee", value);
	return 0;
}

int get_pwr_nbr_interfaces_up(char *refparam, struct dmctx *ctx, char **value)
{
	*value = dmstrdup("TOCODE");
	return 0;
}

int get_pwr_nbr_interfaces_down(char *refparam, struct dmctx *ctx, char **value)
{
	*value = dmstrdup("TOCODE");
	return 0;
}

int set_power_mgmt_param_ethapd(char *refparam, struct dmctx *ctx, int action, char *value)
{
	dmuci_set_value("power_mgmt", "power_mgmt", "ethapd", value);
	return 0;
}

int set_power_mgmt_param_eee(char *refparam, struct dmctx *ctx, int action, char *value)
{
	dmuci_set_value("power_mgmt", "power_mgmt", "eee", value);
	return 0;
}

int entry_method_root_X_INTENO_SE_PowerManagement(struct dmctx *ctx) 
{
	IF_MATCH(ctx, DMROOT"X_INTENO_SE_PowerManagement.") {
		DMOBJECT(DMROOT"X_INTENO_SE_PowerManagement.", ctx, "0", 1, NULL, NULL, NULL);
		DMPARAM("EthernetAutoPowerDownEnable", ctx, "1", get_pwr_mgmt_value_ethapd, set_power_mgmt_param_ethapd, "xsd:boolean", 0, 1, UNDEF, NULL);
		DMPARAM("EnergyEfficientEthernetEnable", ctx, "1", get_pwr_mgmt_value_eee, set_power_mgmt_param_eee, "xsd:boolean", 0, 1, UNDEF, NULL);
		DMPARAM("NumberOfEthernetInterfacesPoweredUp", ctx, "0", get_pwr_nbr_interfaces_up, NULL, "xsd:unsignedInt", 0, 1, UNDEF, NULL);
		DMPARAM("NumberOfEthernetInterfacesPoweredDown", ctx, "0", get_pwr_nbr_interfaces_down, NULL, "xsd:unsignedInt", 0, 1, UNDEF, NULL);
		return 0;
	}
	return FAULT_9005;
}
