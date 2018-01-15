var Benchmark = require('benchmark');
var addon = require('../index.js');
var jsRedis = require('redis');
var fastRedis = require('redis-fast-driver');

console.log("STARTING BENCHMARK");
var nativeClient = new addon.Client({
    host: process.env.REDIS_HOST || 'localhost',
    port: process.env.REDIS_PORT || 6379
}); 
var jsClient = jsRedis.createClient({
    host: process.env.REDIS_HOST || 'localhost',
    port: process.env.REDIS_PORT || 6379
});
var fastClient = new fastRedis({
    host: process.env.REDIS_HOST || 'localhost',
    port: process.env.REDIS_PORT || 6379
});
var suite = new Benchmark.Suite;
var opts = {defer: true}

console.log("STARTING PING SUITE");
suite
    .add('NodeRedisAddon#Call', (def) => nativeClient.call("PING", () => def.resolve()), opts)
    .add('FastRedis#Call', (def) => fastClient.rawCall(["PING"], () => def.resolve()), opts)
    .add('JSRedis#Call', (def) => jsClient.send_command("PING", () => def.resolve()), opts)
    .on('cycle',  (event) =>  console.log(String(event.target)))
    .on('complete', function () { 
        console.log('Fastest is ' + this.filter('fastest').map('name'));
        process.exit();
    }).run();