var r = require('./index');

var c = new r.Client({
    onConnect: function() {
        c.disconnect();
    },
    onDisconnect: function (reply) {
        console.log(reply);
    }
});