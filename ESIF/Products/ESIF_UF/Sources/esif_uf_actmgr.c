/******************************************************************************
** Copyright (c) 2013-2015 Intel Corporation All Rights Reserved
**
** Licensed under the Apache License, Version 2.0 (the "License"); you may not
** use this file except in compliance with the License.
**
** You may obtain a copy of the License at
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
** WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**
** See the License for the specific language governing permissions and
** limitations under the License.
**
******************************************************************************/

#define ESIF_TRACE_ID	ESIF_TRACEMODULE_ACTION

#include "esif_uf.h"		/* Upper Framework */
#include "esif_uf_actmgr.h"	/* Action Manager */

#ifdef ESIF_ATTR_OS_WINDOWS
//
// The Windows banned-API check header must be included after all other headers, or issues can be identified
// against Windows SDK/DDK included headers which we have no control over.
//
#define _SDL_BANNED_RECOMMENDED
#include "win\banned.h"
#endif

/* Friend */
EsifActMgr g_actMgr = {0};

static EsifActPtr GetActionFromName(
	EsifActMgr *THIS,
	EsifString lib_name
	)
{
	u8 i = 0;
	EsifActPtr a_action_ptr = NULL;

	for (i = 0; i < ESIF_MAX_ACTIONS; i++) {
		a_action_ptr = &THIS->fEnrtries[i];

		if (NULL == a_action_ptr->fLibNamePtr) {
			continue;
		}

		if (!strcmp(a_action_ptr->fLibNamePtr, lib_name)) {
			return a_action_ptr;
		}
	}
	return NULL;
}


/*
    ACTION MANAGER
 */
#define ACT_VERSION "x1.0.0.1"
#define GUID 0
#define OS "ALL"
#define OS_WINDOWS "WINDOWS"

static EsifActTypePtr GetActionType(
	EsifActMgrPtr THIS,
	UInt8 type
	)
{
	EsifActTypePtr found_ptr = NULL;
	struct esif_link_list_node *curr_ptr = NULL;
	if (THIS->fActTypes == NULL) {
		goto exit;
	}
	curr_ptr = THIS->fActTypes->head_ptr;

	while (curr_ptr) {
		EsifActTypePtr cur_actiontype_ptr = (EsifActTypePtr)curr_ptr->data_ptr;
		if (cur_actiontype_ptr != NULL) {
			if (type == cur_actiontype_ptr->fType) {
				found_ptr = cur_actiontype_ptr;
				break;
			}
		}
		curr_ptr = curr_ptr->next_ptr;
	}
exit:
	return found_ptr;
}


/* Insert Action Into List */
static eEsifError AddAction(
	EsifActMgrPtr THIS,
	EsifActTypePtr actionPtr
	)
{
	ESIF_TRACE_DEBUG("Item %p", actionPtr);
	return  esif_link_list_add_at_back(THIS->fActTypes, (void *)actionPtr);
}


static eEsifError RemoveAction(
	EsifActMgrPtr THIS,
	EsifActTypePtr type
	)
{
	UNREFERENCED_PARAMETER(THIS);
	UNREFERENCED_PARAMETER(type);

	return ESIF_E_NOT_IMPLEMENTED;
}


eEsifError EsifActMgrInit()
{
	eEsifError rc = ESIF_OK;

	ESIF_TRACE_ENTRY_INFO();

	esif_ccb_lock_init(&g_actMgr.fLock);

	g_actMgr.fActTypes = esif_link_list_create();
	if (NULL == g_actMgr.fActTypes) {
		rc = ESIF_E_NO_MEMORY;
		goto exit;
	}

	g_actMgr.GetActType     = GetActionType;
	g_actMgr.GetActFromName = GetActionFromName;
	g_actMgr.AddActType     = AddAction;
	g_actMgr.RemoveActType  = RemoveAction;

	/* Action manager must be initialized */
	EsifActInit();
exit:
	ESIF_TRACE_EXIT_INFO_W_STATUS(rc);
	return rc;
}


void EsifActMgrExit()
{
	u8 i = 0;
	EsifActPtr a_act_ptr = NULL;

	ESIF_TRACE_ENTRY_INFO();

	/* Call before destroying action manager */
	EsifActExit();

	if (NULL != g_actMgr.fActTypes) {
		esif_link_list_destroy(g_actMgr.fActTypes);
		g_actMgr.fActTypes = NULL;  //set to null so that it will be caught if mid-execution
	}

	esif_ccb_read_lock(&g_actMgr.fLock);
	for (i = 0; i < ESIF_MAX_ACTIONS; i++) {
		a_act_ptr = &g_actMgr.fEnrtries[i];
		esif_ccb_free(a_act_ptr->fLibNamePtr);
		esif_ccb_library_unload(a_act_ptr->fLibHandle);
		esif_ccb_memset(a_act_ptr, 0, sizeof(*a_act_ptr));
	}
	esif_ccb_read_unlock(&g_actMgr.fLock);

	esif_ccb_lock_uninit(&g_actMgr.fLock);

	ESIF_TRACE_EXIT_INFO();
}


/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
