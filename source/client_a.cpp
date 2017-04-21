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
};

static const UvisorBoxAclItem acl[] = {
};

static void client_a_main(void *);

/* Box configuration
 * This box has a smaller interrupt and main thread stack sizes as we do nothing
 * special in them. */
UVISOR_BOX_NAMESPACE("client_a");
UVISOR_BOX_HEAPSIZE(3072);
UVISOR_BOX_MAIN(client_a_main, osPriorityNormal, 512);
UVISOR_BOX_CONFIG(secure_number_client_a, acl, 512, box_context);

/* FIXME: The guard is needed for backwards-compatibility reasons. Remove it
 *        when mbed OS is updated. */
#ifdef __uvisor_ctx
#define uvisor_ctx ((box_context *) __uvisor_ctx)
#endif

static uint32_t get_a_number()
{
    /* Such random. Many secure. Much bits. Wow. */
    return (uvisor_ctx->number -= 500UL);
}

static void box_async_runner(void)
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
            shared_pc.printf("client_a: Attempt to write  0x%08X (%s)\r\n",
                             (unsigned int) number, (ret == 0) ? "granted" : "denied");
            /* FIXME: Add better error handling. */
            if (!status) {
                break;
            }
        }

        Thread::wait(5000);
    }
}

static void box_sync_runner(void)
{
    while (1) {
        /* Synchronous access to the number. */
        const uint32_t number = secure_number_get_number();
        shared_pc.printf("client_a: Attempt to read : 0x%08X (granted)\r\n", (unsigned int) number);

        Thread::wait(7000);
    }
}

static void client_a_main(void *)
{
    /* Create new threads. */
    /* Note: The stack must be at least 1kB since threads will use printf. */
    Thread sync(osPriorityNormal, 1024, NULL);
    sync.start(box_sync_runner);
    Thread async(osPriorityNormal, 1024, NULL);
    async.start(box_async_runner);

    size_t count = 0;
    while (1) {
        /* Spin forever. */
        ++count;
    }
}
