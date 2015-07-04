# WebDemoLauncher

Absolutely minimal HTTP server to provide simpler means to launch Web Browser demos locally.

# Usage

Copy executable file into the same folder where index.html is. When starting this program, it creates a web server and opens the URL in client default browser.

# Shutting down

You can close the program normally, but there are two ways of automatic shutdown.

## Force shutdown from remote

Send get request to /shutdown, this will immediately terminate the program. With jquery it can be done like this: $.get("/shutdown");

## Monitor window with heartbeat

You can send get requests to /heartbeat. After the first request to /heartbeat, the server starts a timer that will terminate program if no more heartbeats are sent in 3 seconds.
