<!--
 * @Author: running-code-pp 3320996652@qq.com
 * @Date: 2025-10-25 11:44:04
 * @LastEditors: running-code-pp 3320996652@qq.com
 * @LastEditTime: 2025-10-25 12:32:56
 * @FilePath: \note\中间件\kong.md
 * @Description: 
 * @Copyright: Copyright (c) 2025 by ${git_name}, All Rights Reserved.
-->
# 部署
## windows
- 安装docker
```bash
docker network create kong-net
docker run -d -p 5432:5432 --name kong-postgresql-13 --network=kong-net -v C:\ProgramDatas\Docker\Containers\kong-database-13:/var/lib/postgresql/data -e "POSTGRES_USER=kong" -e "POSTGRES_DB=kong" -e "POSTGRES_PASSWORD=kongpass" postgres:13

docker run --rm --network=kong-net -e "KONG_DATABASE=postgres" -e "KONG_PG_HOST=kong-postgresql-13" -e "KONG_PG_PASSWORD=kongpass" -e "KONG_PASSWORD=test" kong/kong-gateway:3.6.1.0 kong migrations bootstrap

docker run -d -p 8000:8000 -p 8443:8443 -p 8001:8001 -p 8444:8444 -p 8002:8002 -p 8445:8445 -p 8003:8003 -p 8004:8004 --name kong-gateway-3.6.1.0 --network=kong-net -e "KONG_DATABASE=postgres" -e "KONG_PG_HOST=kong-postgresql-13" -e "KONG_PG_USER=kong" -e "KONG_PG_PASSWORD=kongpass" -e "KONG_PROXY_ACCESS_LOG=/dev/stdout" -e "KONG_ADMIN_ACCESS_LOG=/dev/stdout" -e "KONG_PROXY_ERROR_LOG=/dev/stderr" -e "KONG_ADMIN_ERROR_LOG=/dev/stderr" -e "KONG_ADMIN_LISTEN=0.0.0.0:8001" -e "KONG_ADMIN_GUI_URL=http://localhost:8002" -e KONG_LICENSE_DATA kong/kong-gateway:3.6.1.0

docker run -d -p 5433:5432 --name konga-postgresql-9.6 --network=kong-net --privileged=true -v C:\ProgramDatas\Docker\Containers\konga-postgresql-9.6:/var/lib/postgresql/data -e "POSTGRES_USER=konga" -e "POSTGRES_DB=konga" -e "POSTGRES_PASSWORD=12345" postgres:9.6

docker run --rm --network=kong-net pantsel/konga:0.14.9 -c prepare -a postgres -u postgres://konga:12345@konga-postgresql-9.6:5432/konga

docker run -d -p 1337:1337 --net=kong-net -e "DB_ADAPTER=postgres" -e "DB_HOST=konga-postgresql-9.6" -e "DB_PORT=5432" -e "DB_USER=konga" -e "DB_PASSWORD=12345" -e "DB_DATABASE=konga" -e "NODE_ENV=production" --name konga-0.14.9 pantsel/konga:0.14.9
```


打开浏览器，访问 http://localhost:1337 地址，第一次登录需要初始化用户信息。连接 Kong Gateway ，如果 Kong Gateway 和 KongaA 在同一个 Docker 需要用 http://host.docker.internal:8001 地址。否则，使用 http://127.0.0.1:8001/ 则会在 KongA 容器内部，找到自己的 8001 端口，无法连接到 Kong Gateway，这一点日志也有体现。
