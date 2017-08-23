var r = require('../index.js');
var chai = require('chai');
chai.should();

describe('Redis Addon', function() {
    var client;
    before(function () {
        client = new r.Client();
    });
    it("Calls Ping", function (done) {
        client.call("ping", function (err, reply) {
            if (err) return done(err);
            reply.should.equal('PONG');
            done();
        });
    });

    it("Sets and gets value", function (done) {
        client.set("test", 1, function (err, reply) {
            if (err) return done(err);
            reply.should.equal('OK');
            client.get("test", function (err, reply) {
                if (err) return done(err);
                reply.should.equal('1');
                done();    
            })
        });
    });

    it ("Increases value", function(done) {
        client.incr("test1", function (err, reply) {
            if (err) return done(err);
            reply.should.equal(1);
            client.incr("test1", function (err, reply) {
                if (err) return done(err);
                reply.should.equal(2);
                done();    
            })
        });
    });
});
