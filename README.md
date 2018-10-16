### MyRedisModules

> This repository is forked from [RedisModulesSDK](https://github.com/RedisLabs/RedisModulesSDK).

The Redis 4.x has the feature of [Modules](https://redis.io/topics/modules-intro).Modules allow us to implement new Redis commands at a speed and with features similar to what can be done inside the core itself.

In this repository, I implement some customized commands in file `example/module.c`.

### Platform

* Redis version >= 4.0.0
* Linux | MacOS
* C

### Startup

1. Make the `module.so`.

```shell
$ git clone git@github.com:Sidfate/MyRedisModules.git
$ cd ./MyRedisModules/example
$ make all
```

2. Start redis server with `module.so`.

```shell
$ /path/to/redis-server --loadmodule ./module.so
```

3. Test for command

```shell
$ redis-cli
127.0.0.1:6379> MY.TEST
PASS
```

### Commands

1. MY.HGETSET

    > This command extends from [RedisModulesSDK](https://github.com/RedisLabs/RedisModulesSDK/blob/master/example/module.c).


    Atomically set a value in a HASH key to <value> and return its value before the HSET.

    Usage: 
    ```
    MY.HGETSET <key> <element> <value>
    ```

    Test: 
    ```shell
    $ redis-cli
    127.0.0.1:6379> MY.HGETSET foo bar baz
    (nil)
    127.0.0.1:6379> MY.HGETSET foo bar vaz
    "baz"
    127.0.0.1:6379> RPUSH mylist "test1"
    (integer) 1
    127.0.0.1:6379> RPUSH mylist "test2"
    (integer) 10
    127.0.0.1:6379> RPUSH mylist "test2"
    (integer) 10
    127.0.0.1:6379> MY.TEST
    PASS
    ```

2. MY.LDEL

    Delete list element by the index

    Usage:
    ```
    MY.LDEL <key> <index>
    ```

    Test: 
    ```shell
    $ redis-cli
    127.0.0.1:6379> RPUSH mylist "test1"
    (integer) 1
    127.0.0.1:6379> RPUSH mylist "test2"
    (integer) 2
    127.0.0.1:6379> RPUSH mylist "test2"
    (integer) 3
    127.0.0.1:6379> LRANGE mylist 0 -1
    1) "test1"
    2) "test2"
    3) "test3"
    127.0.0.1:6379> MY.LDEL mylist 0
    OK
    127.0.0.1:6379> LRANGE mylist 0 -1
    1) "test2"
    2) "test3"
    ```


### License
MIT