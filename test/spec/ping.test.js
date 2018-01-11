var r = require('../../index.js');
var chai = require('chai');
chai.should();

describe('Basic Ping Test', function() {
    var client;
    before(function () {
        client = new r.Client({
            host: process.env.REDIS_HOST || 'localhost', 
            port: process.env.REDIS_PORT || 6379
        });
    });
    it("Calls Ping", function (done) {
        client.call("ping", function (err, reply) {
            if (err) return done(err);
            reply.should.equal('PONG');
            done();
        });
    });
});
