var r = require('../../index.js');
var chai = require('chai');
chai.should();

describe('Basic Connect Test', function() {
    it("Connects to existing server", function (done) {
        var client = new r.Client({
            host: process.env.REDIS_HOST || 'localhost', 
            port: process.env.REDIS_PORT || 6379,
            onConnect: function (reply) {
                reply.should.equal(0);
                done();
            }
        });
    });
    it("Fails to connect to nonexisting server", function (done) {
        var client = new r.Client({
            host: 'nowhere', 
            port: 9999,
            onError: function (reply) {
                reply.should.equal('Name does not resolve');
                done();
            }
        });
    });
    it("Disconnects from existing server on quit command", function (done) {
        var client = new r.Client({
            host: process.env.REDIS_HOST || 'localhost', 
            port: process.env.REDIS_PORT || 6379,
            onConnect: function() {
                client.call('quit', function (err, reply) {
                    console.log(err, reply);
                });
            },
            onDisconnect: function (reply) {
                // Status is REDIS_ERR, according to comments in redis code
                // it is what should happen on quit
                reply.should.equal(-1);
                done();
            }
        });
    });
});
