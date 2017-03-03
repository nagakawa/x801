const fs = require("fs");
const http = require('http');
const url = require('url');

const hostname = '127.0.0.1';
const port = 3000;

var contents = {};
var metadata = JSON.parse(fs.readFileSync("assets/index.json", "utf-8"))

function getContent(name, callback) {
  if (name in contents && contents[name] !== undefined)
    callback(null, contents[name]);
  else {
    fs.readFile(name, (err, data) => {
      if (err == null) contents[name] = data;
      callback(err, data);
    })
  }
}

const server = http.createServer((req, res) => {
  var parsedURL = url.parse(req.url, true);
  if (parsedURL.pathname == "/content") {
    console.log(parsedURL);
    getContent("assets/" + parsedURL.query.fname, (err, data) => {
      if (err) {
        res.setHeader('Content-Type', 'text/plain');
        res.statusCode = 404;
        res.end("Resource not found...\n");
      } else {
        res.setHeader('Content-Type', 'text/plain');
        res.statusCode = 200;
        res.end(data);
      }
    });
  } else if (parsedURL.pathname == "/version") {
    var fname = "assets/" + parsedURL.query.fname;
    if (fname in metadata) {
      res.setHeader('Content-Type', 'text/plain');
      res.statusCode = 200;
      res.end("" + metadata[fname].version);
    } else {
      res.setHeader('Content-Type', 'text/plain');
      res.statusCode = 404;
      res.end("Resource not found...\n");
    }
  } else {
    res.statusCode = 404;
    res.setHeader('Content-Type', 'text/plain');
    res.end('Oh shit! You fucked up!\n');
  }
});

server.listen(port, hostname, () => {
  console.log(`Server running at http://${hostname}:${port}/`);
});
