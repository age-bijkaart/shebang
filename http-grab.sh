#!./shebang phantomjs $< $1
var args = require('system').args;
if (args.length == 2) 
  url = args[1];
else {
  console.log("usage: " + args[0] + " url");
  phantom.exit(1);
}

var page = require('webpage').create();
page.open(url, function (status) { 
  if (status=='success') {
    console.log(page.content); 
    phantom.exit(0); 
  }
  else
    phantom.exit(2);
});
