var r = require('../../index.js');
var chai = require('chai');
chai.should();

describe('Basic INC test', function() {
    var client;
    before(function () {
        client = new r.Client({
            host: process.env.REDIS_HOST || 'localhost', 
            port: process.env.REDIS_PORT || 6379
        });
    });

    it ("Increases value", function(done) {
        client.call("incr test1", function (err, reply) {
            if (err) return done(err);
            reply.should.equal(1);
            client.call("incr test1", function (err, reply) {
                if (err) return done(err);
                reply.should.equal(2);
                done();    
            })
        });
    });
});
