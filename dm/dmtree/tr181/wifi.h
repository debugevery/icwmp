#ifndef __WIFI_H
#define __WIFI_H
#include <libubox/blobmsg_json.h>
#include <json/json.h>

struct wifi_radio_args
{
	struct uci_section *wifi_radio_sec;
};

struct wifi_ssid_args
{
	struct uci_section *wifi_ssid_sec;
	char *wiface;
	char *linker;
};
int entry_method_root_Wifi(struct dmctx *ctx);

#endif