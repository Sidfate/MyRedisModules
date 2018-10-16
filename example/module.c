#include "../redismodule.h"
#include "../rmutil/util.h"
#include "../rmutil/strings.h"
#include "../rmutil/test_util.h"

/*
* Create a random uuid
*
*/
char *random_uuid(char buf[37])
{
  const char *c = "89ab";
  char *p = buf;
  int n;

  for (n = 0; n < 16; ++n) {
    int b = rand() % 255;

    switch (n) {
    case 6:
      sprintf(p, "4%x", b % 15);
      break;
    case 8:
      sprintf(p, "%c%x", c[rand() % strlen(c)],b % 15);
      break;
    default:
      sprintf(p, "%02x", b);
      break;
    }

    p += 2;

    switch (n) {
    case 3:
    case 5:
    case 7:
    case 9:
      *p++ = '-';
      break;
    }
  }

  *p = 0;

  return buf;
}

/*
* MY.LDEL <key> <index>
*
* Delete list element by the index
*/
int LDELCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{

  // we need EXACTLY 3 arguments
  if (argc != 3)
  {
    return RedisModule_WrongArity(ctx);
  }
  RedisModule_AutoMemory(ctx);

  // open the key and make sure it's indeed a LIST and not empty
  RedisModuleKey *key =
      RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
  if (RedisModule_KeyType(key) != REDISMODULE_KEYTYPE_LIST &&
      RedisModule_KeyType(key) != REDISMODULE_KEYTYPE_EMPTY)
  {
    return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
  }

  char guid[37];
  random_uuid(guid);
  // get the current value of the hash element
  RedisModuleCallReply *rep =
      RedisModule_Call(ctx, "LSET", "ssc", argv[1], argv[2], guid);
  RMUTIL_ASSERT_NOERROR(ctx, rep);

  // set the new value of the element
  RedisModuleCallReply *srep =
      RedisModule_Call(ctx, "LREM", "scc", argv[1], "0", guid);
  RMUTIL_ASSERT_NOERROR(ctx, srep);

  // forward the HGET reply to the client
  RedisModule_ReplyWithCallReply(ctx, rep);
  return REDISMODULE_OK;
}

/*
* MY.HGETSET <key> <element> <value>
* Atomically set a value in a HASH key to <value> and return its value before
* the HSET.
*
* Basically atomic HGET + HSET
*/
int HGetSetCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{

  // we need EXACTLY 4 arguments
  if (argc != 4)
  {
    return RedisModule_WrongArity(ctx);
  }
  RedisModule_AutoMemory(ctx);

  // open the key and make sure it's indeed a HASH and not empty
  RedisModuleKey *key =
      RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
  if (RedisModule_KeyType(key) != REDISMODULE_KEYTYPE_HASH &&
      RedisModule_KeyType(key) != REDISMODULE_KEYTYPE_EMPTY)
  {
    return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
  }

  // get the current value of the hash element
  RedisModuleCallReply *rep =
      RedisModule_Call(ctx, "HGET", "ss", argv[1], argv[2]);
  RMUTIL_ASSERT_NOERROR(ctx, rep);

  // set the new value of the element
  RedisModuleCallReply *srep =
      RedisModule_Call(ctx, "HSET", "sss", argv[1], argv[2], argv[3]);
  RMUTIL_ASSERT_NOERROR(ctx, srep);

  // if the value was null before - we just return null
  if (RedisModule_CallReplyType(rep) == REDISMODULE_REPLY_NULL)
  {
    RedisModule_ReplyWithNull(ctx);
    return REDISMODULE_OK;
  }

  // forward the HGET reply to the client
  RedisModule_ReplyWithCallReply(ctx, rep);
  return REDISMODULE_OK;
}

// Test the the LDEL command
int testLdel(RedisModuleCtx *ctx)
{
  RedisModuleCallReply *r =
      RedisModule_Call(ctx, "rpush", "mylist", "test1");
  r = RedisModule_Call(ctx, "rpush", "mylist", "test2");
  r = RedisModule_Call(ctx, "rpush", "mylist", "test1");

  r = RedisModule_Call(ctx, "my.ldel", "mylist", "0");
  RMUtil_Assert(RedisModule_CallReplyType(r) != REDISMODULE_REPLY_ERROR);
  // RMUtil_AssertReplyEquals(r, "OK");

  return 0;
}

// test the HGETSET command
int testHgetSet(RedisModuleCtx *ctx)
{
  RedisModuleCallReply *r =
      RedisModule_Call(ctx, "my.hgetset", "ccc", "foo", "bar", "baz");
  RMUtil_Assert(RedisModule_CallReplyType(r) != REDISMODULE_REPLY_ERROR);

  r = RedisModule_Call(ctx, "my.hgetset", "ccc", "foo", "bar", "bag");
  RMUtil_Assert(RedisModule_CallReplyType(r) == REDISMODULE_REPLY_STRING);
  RMUtil_AssertReplyEquals(r, "baz");
  r = RedisModule_Call(ctx, "my.hgetset", "ccc", "foo", "bar", "bang");
  RMUtil_AssertReplyEquals(r, "bag");
  return 0;
}

// Unit test entry point for the module
int TestModule(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
{
  RedisModule_AutoMemory(ctx);

  RMUtil_Test(testLdel);
  RMUtil_Test(testHgetSet);

  RedisModule_ReplyWithSimpleString(ctx, "PASS");
  return REDISMODULE_OK;
}

int RedisModule_OnLoad(RedisModuleCtx *ctx)
{

  if (RedisModule_Init(ctx, "my", 1, REDISMODULE_APIVER_1) ==
      REDISMODULE_ERR)
  {
    return REDISMODULE_ERR;
  }

  // register example.hgetset - using the shortened utility registration macro
  RMUtil_RegisterWriteCmd(ctx, "my.hgetset", HGetSetCommand);

  // register the example.ldel
  RMUtil_RegisterWriteCmd(ctx, "my.ldel", LDELCommand);

  // register the unit test
  RMUtil_RegisterWriteCmd(ctx, "my.test", TestModule);

  return REDISMODULE_OK;
}
