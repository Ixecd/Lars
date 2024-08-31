use lars_dns;

set @time =  UNIX_TIMESTAMP(NOW());

insert into `RouteData`(modid, cmdid, serverip,serverport) values(1, 1, 323223593, 9999);

update `RouteVersion` set version = @time where id = 1;

insert into `RouteChange`(modid, cmdid, version) values(1,1,@time);

-- insert into `RouteChange`(modid, cmdid, version) values(1,2, 1000);
