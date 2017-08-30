# Native node redis addon (experimental, WIP)

Currently supports only node v8.x.

### Usage

node-gyp should be installed

```sh
    $ npm install git+https://github.com/paulkarabilo/node-redis-native.git
```

```javascript
    var r = require('node-redis-native');
    var client = new r.Client({
        host: '127.0.0.1', //optional, default is 'localhost'
        port: 6378, //optional, default is 6379
        onError: function () {}
    });

    client.call("PING", function (err, reply) { console.log(reply); }); //PONG
```
