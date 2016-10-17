#include "asyn_router.h"
#include "../../general/mac_frame.h"
#include "../../general/mac_package.h"
asyn_router_cb_t asyn_router_cb;
static asyn_info_t info;
static mac_state_e get_state(void)
{
	return *info.state;
}
static void set_state(mac_state_e state)
{
	*info.state = state;
}

static bool_t run(void)
{
	return TRUE;
}
static bool_t stop(void)
{
	set_state(WORK_DOWN);
	return TRUE;
}
static bool_t router_send(sbuf_t *sbuf)
{
	DBG_ASSERT(sbuf != NULL __DBG_LINE);
	switch (sbuf->primtype)
	{
	case N2M_DATA_REQUEST:
	{
#if NODE_TYPE == NODE_TYPE_COLLECTOR
		south_sbuf_fill(sbuf);
#else
		mac_data_fill_package(sbuf);
#endif
	}
	break;
	default:
		return FALSE;
	}
	return TRUE;
}

static asyn_info_t* get(void)
{
	return &info;
}

const struct asyn_router_t asyn_router =
{
	run,
	stop,
	router_send,
	get,
};
