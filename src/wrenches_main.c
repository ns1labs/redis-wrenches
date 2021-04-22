/*
 * Redis-Wrenches ~ Copyright 2021 NSONE, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <redismodule.h>

#include <rmutil/logging.h>
#include <rmutil/util.h>


int RW_hmgetall(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (argc < 2) {
        return RedisModule_WrongArity(ctx);
    }

    RedisModule_AutoMemory(ctx);

    // Prepare the response array. Following this call we must add exactly
    // `argc - 1` elements to the response. Those elements can be any valid
    // Redis reply type. In our case they are nested arrays.
    //
    // N.B. according to module.c docs, this will always return OK.
    RedisModule_ReplyWithArray(ctx, argc - 1);

    int reply_type;
    for (int i = 1; i < argc; i++) {
        RedisModuleCallReply *reply = RedisModule_Call(ctx, "HGETALL", "s", argv[i]);

        reply_type = RedisModule_CallReplyType(reply);
        if (reply_type == REDISMODULE_REPLY_ARRAY) {
            RedisModule_ReplyWithCallReply(ctx, reply);
        } else {
            if (reply_type == REDISMODULE_REPLY_ERROR) {
                RM_LOG_NOTICE(ctx, "coerced HGETALL to empty array");
            }

#if REDIS_VERSION >= 6
            RedisModule_ReplyWithEmptyArray(ctx);
#else
            // ReplyWithEmptyArray is new in v6.
            RedisModule_ReplyWithArray(ctx, 0);
#endif
        }
    }

    return REDISMODULE_OK;
}

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    // Clears unused args warnings.
    (void) argv;
    (void) argc;

    if (RedisModule_Init(ctx, "rw", 1, REDISMODULE_APIVER_1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    RMUtil_RegisterReadCmd(ctx, "rw.hmgetall", RW_hmgetall);

    return REDISMODULE_OK;
}
