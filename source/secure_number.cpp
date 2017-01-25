/*
 * Copyright (c) 2016, ARM Limited, All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "uvisor-lib/uvisor-lib.h"
#include "mbed.h"
#include "rtos.h"
#include "main-hw.h"

struct box_context {
    uint32_t secret_number;
    int trusted_id;
    int previous_box_caller;
    int caller_id;
    RawSerial * pc;
};

static const UvisorBoxAclItem acl[] = {
};

static void number_store_main(const void *);
static uint32_t get_number(void);
static int set_number(uint32_t number);

/* Box configuration */
UVISOR_BOX_NAMESPACE(NULL);
UVISOR_BOX_HEAPSIZE(3072);
UVISOR_BOX_MAIN(number_store_main, osPriorityNormal, UVISOR_BOX_STACK_SIZE);
UVISOR_BOX_CONFIG(box_number_store, acl, UVISOR_BOX_STACK_SIZE, box_context);

/* Gateways */
UVISOR_BOX_RPC_GATEWAY_SYNC (box_number_store, secure_number_get_number, get_number, uint32_t, void);
UVISOR_BOX_RPC_GATEWAY_ASYNC(box_number_store, secure_number_set_number, set_number, int, uint32_t);

static int get_caller_id()
{
    if (uvisor_ctx->caller_id != uvisor_ctx->previous_box_caller) {

        led_blue = LED_ON;
        Thread::wait(100);
        led_blue = LED_OFF;

        uvisor_ctx->previous_box_caller = uvisor_ctx->caller_id;
    }

    return uvisor_ctx->caller_id;
}

static uint32_t get_number(void)
{
    led_green = LED_ON;
    Thread::wait(100);
    led_green = LED_OFF;

    return uvisor_ctx->secret_number;
}

static int set_number(uint32_t number)
{
    const int id = get_caller_id();

    /* Cache the name verification result. This allows future checks to replace
     * a relatively more expensive string compare with a cheaper integer
     * comparison. */
    if (uvisor_ctx->trusted_id == -1) {
        char name[UVISOR_MAX_BOX_NAMESPACE_LENGTH];
        memset(name, 0, sizeof(name));
        uvisor_box_namespace(id, name, sizeof(name));
        /* We only trust client a. */
        static const char * trusted_namespace = "client_a";
        if (memcmp(name, trusted_namespace, sizeof(*trusted_namespace)) == 0) {
            uvisor_ctx->trusted_id = id;
            uvisor_ctx->pc->printf("Trusted client a has box id %u\r\n", id);
        } else {
            return 1;
        }
    }
    if (uvisor_ctx->trusted_id != id) {
        /* This box is not allowed to write to the secret number. */
        return 1;
    }

    /* Let's pretend this action takes 50ms */
    led_red = LED_ON;
    Thread::wait(100);
    led_red = LED_OFF;

    uvisor_ctx->secret_number = number;
    return 0;
}

static void number_store_main(const void *)
{
    /* Allocate serial port to ensure that code in this secure box won't touch
     * the handle in the default security context when printing. */
    uvisor_ctx->pc = new RawSerial(USBTX, USBRX);
    if (!uvisor_ctx->pc) {
        return;
    }

    /* Today we only allow client a to write to the number. */
    uvisor_ctx->trusted_id = -1;

    /* The list of functions we are interested in handling RPC requests for */
    static const TFN_Ptr my_fn_array[] = {
        (TFN_Ptr) get_number,
        (TFN_Ptr) set_number
    };

    while (1) {
        int status;

        /* NOTE: This serializes all access to the number store! */
        status = rpc_fncall_waitfor(my_fn_array, 2, &uvisor_ctx->caller_id, UVISOR_WAIT_FOREVER);

        if (status) {
            uvisor_ctx->pc->printf("Failure is not an option.\r\n");
            uvisor_error(USER_NOT_ALLOWED);
        }
    }
}
