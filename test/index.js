var r = require('../index.js');
var chai = require('chai');
chai.should();

describe('Redis Addon', function() {
    var client;
    before(function () {
        client = new r.Client({host: '127.0.0.1', port: 6379});
    });
    it("Calls Ping", function (done) {
        client.call("ping", function (err, reply) {
            if (err) return done(err);
            reply.should.equal('PONG');
            done();
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
