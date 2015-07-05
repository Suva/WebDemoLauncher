# WebDemoLauncher

Absolutely minimal HTTP server to provide simpler means to launch Web Browser demos locally. Get the binaries from [RELEASES PAGE](https://github.com/Suva/WebDemoLauncher/releases).

## Usage

Copy executable file into the same folder where index.html is. When starting this program, it creates a web server and opens the URL in client default browser.

## Shutting down

You can close the program normally, but there are easier way for automatic shutdown. Just include a magic script /wdl.js: &lt;script src="/wdl.js"&gt;&lt;/script&gt;, this will register onunload handler that will close the server when browser is closed. If you would rather manually use the methods, check out their descriptions below.

### Force shutdown from remote

Send get request to /shutdown, this will immediately terminate the program. With jquery it can be done like this: $.get("/shutdown");

### Monitor window with heartbeat

You can send get requests to /heartbeat. After the first request to /heartbeat, the server starts a timer that will terminate program if no more heartbeats are sent in 3 seconds. This will effectively close the server after window is closed.
