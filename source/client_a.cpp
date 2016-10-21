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
#include "secure_number.h"

struct box_context {
    uint32_t number;
    RawSerial * pc;
};

static const UvisorBoxAclItem acl[] = {
};

static void client_a_main(const void *);

/* Box configuration */
UVISOR_BOX_NAMESPACE("client_a");
UVISOR_BOX_HEAPSIZE(8192);
UVISOR_BOX_MAIN(client_a_main, osPriorityNormal, UVISOR_BOX_STACK_SIZE);
UVISOR_BOX_CONFIG(secure_number_client_a, acl, UVISOR_BOX_STACK_SIZE, box_context);

static uint32_t get_a_number()
{
    /* Such random. Many secure. Much bits. Wow. */
    return (uvisor_ctx->number -= 500UL);
}

static void box_async_runner(const void *)
{
    while (1) {
        uvisor_rpc_result_t result;
        const uint32_t number = get_a_number();
        result = secure_number_set_number(number);

        /* Simulating some workload here. */
        Thread::wait(10);

        /* Wait for a non-error result synchronously. */
        while (1) {
            uint32_t ret;
            int status = rpc_fncall_wait(result, UVISOR_WAIT_FOREVER, &ret);
            uvisor_ctx->pc->printf("%c: %s '0x%08x'\r\n", (char) uvisor_box_id_self() + '0', (ret == 0) ? "Wrote" : "Failed to write", (unsigned int) number);
            /* FIXME: Add better error handling. */
            if (!status) {
                break;
            }
        }

        Thread::wait(5000);
    }
}

static void box_sync_runner(const void *)
{
    while (1) {
        /* Synchronous access to the number. */
        const uint32_t number = secure_number_get_number();
        uvisor_ctx->pc->printf("%c: Read '0x%08x'\r\n", (char) uvisor_box_id_self() + '0', (unsigned int) number);

        Thread::wait(7000);
    }
}

static void client_a_main(const void *)
{
    /* Allocate serial port to ensure that code in this secure box won't touch
     * the handle in the default security context when printing. */
    uvisor_ctx->pc = new RawSerial(USBTX, USBRX);
    if (!uvisor_ctx->pc) {
        return;
    }

    srand(uvisor_box_id_self());
    new Thread(box_sync_runner, NULL);
    new Thread(box_async_runner, NULL);
}
