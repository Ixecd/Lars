-- Active: 1717058667981@@127.0.0.1@3306
drop database if exists lars_dns;
create database lars_dns;
use lars_dns;

drop table if exists `RouteData`;
create table `RouteData` (
    `id` int unsigned not null AUTO_INCREMENT,
    `modid` int unsigned not null,
    `cmdid` int unsigned not null,
    `serverip` int unsigned not null,
    `serverport` int unsigned not null,
    PRIMARY KEY (`id`)
) engine=InnoDB AUTO_INCREMENT= 65432 DEFAULT CHARSET=utf8;

drop table if exists `RouteVersion`;
create table RouteVersion (
    `id` int unsigned not null AUTO_INCREMENT,
    `version` int unsigned not null,
    PRIMARY KEY(`id`)
);

insert into RouteVersion(version) values(0);

drop table if exists `RouteChange`;
create table RouteChange (
    `id` int unsigned not null AUTO_INCREMENT,
    `modid` int unsigned not null,
    `cmdid` int unsigned not null,
    `version` bigint unsigned not null,
    PRIMARY KEY (`id`)
);


insert into RouteData values(1, 1, 2, 1114, 19);