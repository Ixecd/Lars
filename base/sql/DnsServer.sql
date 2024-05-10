-- Active: 1713606375979@@127.0.0.1@3306@qc
drop database if exists lars_dns;
create database lars_dns;
use lars_dns;

drop table if exists `RouteData`;
create table `RouteData` (
    `id` int(10) unsigned not null AUTO_INCREMENT,
    `modid` int(10) unsigned not null,
    `cmdid` int(10) unsigned not null,
    `serverip` int(10) unsigned not null,
    `serverport` int(10) unsigned not null,
    PRIMARY KEY (`id`)
) engine=InnoDB AUTO_INCREMENT=116064 DEFAULT CHARSET=utf8;

drop table if exists `RouteVersion`;
create table RouteVersion (
    `id` int(10) unsigned not null AUTO_INCREMENT,
    `version` int(10) unsigned not null,
    PRIMARY KEY(`id`)
);

insert into RouteVersion(version) values(0);

drop table if exists `RouteChange`;
create table RouteChange (
    `id` int(10) unsigned not null AUTO_INCREMENT,
    `modid` int(10) unsigned not null,
    `cmdid` int(10) unsigned not null,
    `version` bigint(20) unsigned not null,
    PRIMARY KEY (`id`)
);