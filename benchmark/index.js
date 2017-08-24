var Benchmark = require('benchmark');
var nativeRedis = require('../index.js');
var jsRedis = require('redis');
var fastRedis = require('redis-fast-driver');

var nativeClient = new nativeRedis.Client();
var jsClient = jsRedis.createClient();
var fastClient = new fastRedis({});
var suite = new Benchmark.Suite;
var opts = {defer: true}

suite
    .add('NativeRedis#Call', (def) => nativeClient.call("PING", () => def.resolve()), opts)
    .add('FastRedis#Call', (def) => fastClient.rawCall(["PING"], () => def.resolve()), opts)
    .add('JSRedis#Call', (def) => jsClient.send_command("PING", () => def.resolve()), opts)
    .on('cycle',  (event) =>  console.log(String(event.target)))
    .on('complete', function () { 
        console.log('Fastest is ' + this.filter('fastest').map('name'));
        process.exit();
    }).run();