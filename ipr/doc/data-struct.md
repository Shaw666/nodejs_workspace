
SESSION.runtime
===============
```
    session.runtime = {
            profile : <JProfile>,             // 基本信息
            areas   : <JAreas>,             // 报警分区信息
            [zones] : <JZones>,             // 报警防区信息
            [flags] : <JFlags>,             // 设备状态标记
    }
```


基本参数设置
------------

```
<JProfile> = {
    tid     : <tid>,     // 设备唯一ID
    brand   : <string>,  // 品牌
    model   : <string>,  // 型号
    ver     : <string>,  // 版本
    [sn]    : <string>,  // 产品序列号
}
```

报警分区状态
------------

```
<JAreas> = {
    <GG> : <JArea>,     // GG = 分区号, 对应cid消息中的分区号
    ...
}
<JArea> = {
    [stat]   : <string>,
    [t_stat] : <string>,
}
```


报警防区状态
------------

```
    <JZones> = {
        <ZZZ> : <JZone>,
        ...
    }

    <JZone>  = {
        [stat]   : <string>,
        [t_stat] : <string>,
    }
```

其它设备状态
------------

```
<JFlags> = {
    <name> : <value>,
    ...
}
```

| flag name    | value             | memo         |
| ------------ | ----------------- | ------------ |
| panel-bus    | null, true, false | 报警主机总线 |
| low-battery  | null, true, false | 电池电压低   |
| ac-fail      | null, true, false | 无交流电     |
| line-fail    | null, true, false | 电话线故障   |
| storage-full | null, true, false | 存储设备已满 |
| storage-fail | null, true, false | 存储设备故障 |

* null 表示状态未知


CID_EVENT
=========

```
CID_EVENT =  {
    daemon  : <Daemon>,
    session : <Session>,
    cid     : <string>,
    eid     : <string>,
    time    : <Date>,
    time_src: "dev" | "server"
}

```


EVENT_LOG
=========

* LOCATION  : /var/conwin/event.q
* DELIMITER : ";"

```
   line = <TIME>;<LINE>;<CID>;<EID>;<PROTOCOL>;<PROTOCOL PORT>;<IP>;<SN>|<TID>|<CALLER ID>
```


SYSTEM_LOG
==========


