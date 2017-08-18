let r = require('./index.js');

let c = new r.Client();

c.call("ping", (err, res) => console.log(res));

c.call("SET test 1", (err, res) => c.call("GET test", (err, res) => console.log(res)));

c.set("test1", 1, (err, res) => c.get("test1", (err, res) => console.log(res)));