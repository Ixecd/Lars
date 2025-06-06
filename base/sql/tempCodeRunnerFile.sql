-- Active: 1717058667981@@127.0.0.1@3306
# 服务器调用状态表
use lars_dns;

drop table if exists `ServerCallStatus`;

create table `ServerCallStatus` (
    `modid` int unsigned not null comment '模块ID',
    `cmdid` int unsigned not null comment '指令ID' ,
    `ip` int unsigned not null comment '服务端ip',
    `port` int unsigned not null comment '服务端port',
    `caller` int unsigned not null comment '调用者',
    `succ_cnt` int unsigned not null comment '成功次数',
    `err_cnt` int unsigned not null  comment '失败次数',
    `ts` bigint unsigned not null comment '记录时间',
    `overload` int unsigned not null comment '是否过载',
    primary key(`modid`, `cmdid`, `ip`, `port`, `caller`),
    key `mlb_index` (`modid`, `cmdid`, `ip`, `port`, `caller`)
) engine=InnoDB default charset=utf8;
