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
#include <stdint.h>
#include <assert.h>

/* Create ACLs for main box. */
MAIN_ACL(g_main_acl);

/* Register privleged system hooks. */
UVISOR_EXTERN void SVC_Handler(void);
UVISOR_EXTERN void PendSV_Handler(void);
UVISOR_EXTERN void SysTick_Handler(void);
extern "C" uint32_t rt_suspend(void);

UVISOR_SET_PRIV_SYS_HOOKS(SVC_Handler, PendSV_Handler, SysTick_Handler, rt_suspend, __uvisor_semaphore_post);

/* Enable uVisor. */
UVISOR_SET_MODE_ACL(UVISOR_ENABLED, g_main_acl);

DigitalOut led_red(LED1);
DigitalOut led_green(LED2);
DigitalOut led_blue(LED3);

static uint32_t get_a_number()
{
    static uint32_t number = 425;
    return (number -= 400UL);
}

static void main_async_runner(void)
{
    while (1) {
        uvisor_rpc_result_t result;
        const uint32_t number = get_a_number();
        result = secure_number_set_number(number);

        // ...Do stuff asynchronously here...

        /* Wait for a non-error result synchronously. */
        while (1) {
            int status;
            /* TODO typesafe return codes */
            uint32_t ret;
            status = rpc_fncall_wait(result, UVISOR_WAIT_FOREVER, &ret);
            printf("%c: %s '0x%08x'\r\n",
                   (char) uvisor_box_id_self() + '0',
                   (ret == 0) ? "Wrote" :
                                "Permission denied. This client cannot write the secure number",
                   (unsigned int) number);
            if (!status) {
                break;
            }
        }

        Thread::wait(13000);
    }
}

static void main_sync_runner(void)
{
    while (1) {
        /* Synchronous access to the number. */
        const uint32_t number = secure_number_get_number();
        printf("%c: Read '0x%08x'\r\n", (char) uvisor_box_id_self() + '0', (unsigned int) number);

        Thread::wait(11000);
    }
}

int main(void)
{
    printf("\r\n***** uVisor secure number store example *****\r\n");
    led_red = LED_OFF;
    led_blue = LED_OFF;
    led_green = LED_OFF;

    /* Startup a few RPC runners. */
    /* Note: The stack must be at least 1kB since threads will use printf. */
    Thread sync(osPriorityNormal, 1024, NULL);
    sync.start(main_sync_runner);
    Thread async(osPriorityNormal, 1024, NULL);
    async.start(main_async_runner);

    size_t count = 0;

    while (1)
    {
        /* Spin forever. */
        ++count;
    }

    return 0;
}
