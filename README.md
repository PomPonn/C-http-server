# C_server
simple, single-threaded windows http server written in C

## TODO:
-  fix: whats going on with create_server function? test variant on profiling branch is exactly the same and works just fine
- fix: for some reason when client requests favicon.ico and server responds with 404 then client still keeps connection alive (sometimes closes). It probably should be closed.
