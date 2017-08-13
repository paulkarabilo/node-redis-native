let r = require('./index.js');

let c = new r.Client();

c.call("ping", function(err, res) {console.log(res); });

c.call("SET test 1", function(err, res) {
    c.call("GET test", function(err, res) {
        console.log(res);
    });
});

c.set("test1", "1", function (err, res) {
    c.get("test1", function(err, res) {
        console.log(res);
    });
});