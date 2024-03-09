# C_HTTP_server
HTTP server written in C

# BUGS:
- handle socket closing -> refreshing page causes creation of second (unwanted) socket which becomes valid socket afer another refresh
  - this process repeats and always leaves at least one useless socket
  - removing critical sections causes this effect to happen when directly connecting with port too: investigate it
    - after adding them back i still does the same thing
- only six connections can be created - idk why