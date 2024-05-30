-- Active: 1717058667981@@127.0.0.1@3306
# 服务器调用状态表
use lars_dns;

drop table if exists `ServerCallStatus`;

create table `ServerCallStatus` (
    `modid` int(11) not null comment '模块ID',
    `cmdid` int(11) not null comment '指令ID' ,
    `ip` int(11) not null comment '服务器ip',
    `port` int(11) not null comment '服务器port',
    `caller` int(11) not null comment '调用者',
    `succ_cnt` int(11) not null comment '成功次数',
    `err_cnt` int(11) not null  comment '失败次数',
    `ts` bigint not null comment '记录时间',
    `overload` int(11) not null comment '是否过载',
    primary key(`modid`, `cmdid`, `ip`, `port`, `caller`),
    key `mlb_index` (`modid`, `cmdid`, `ip`, `port`, `caller`)
) engine=InnoDB default charset=utf8;


