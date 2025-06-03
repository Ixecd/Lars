USE lars_dns;

INSERT INTO RouteData(modid, cmdid, serverip, serverport) VALUES(1, 1, 3232235953, 8885);
INSERT INTO RouteData(modid, cmdid, serverip, serverport) VALUES(1, 2, 3232235954, 8886);
INSERT INTO RouteData(modid, cmdid, serverip, serverport) VALUES(1, 2, 3232235955, 8887);
INSERT INTO RouteData(modid, cmdid, serverip, serverport) VALUES(1, 2, 3232235956, 8888);

UPDATE RouteVersion SET version = UNIX_TIMESTAMP(NOW()) WHERE id = 1;
USE lars_dns;

UPDATE RouteVersion SET version = UNIX_TIMESTAMP(NOW()) WHERE id = 1;
