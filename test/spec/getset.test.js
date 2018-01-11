var r = require('../../index.js');
var chai = require('chai');
chai.should();

describe('Basic Get/Set TEst', function() {
    var client;
    before(function () {
        client = new r.Client({
            host: process.env.REDIS_HOST || 'localhost', 
            port: process.env.REDIS_PORT || 6379
        });
    });
    it("Calls set and get", function (done) {
        client.call("set test 1", function (err, reply) {
            if (err) return done(err);
            reply.should.equal('OK');
            client.call("get test", function (err, reply) {
                if (err) return done(err);
                reply.should.equal('1');
                done();    
            })
        });
    });
});
