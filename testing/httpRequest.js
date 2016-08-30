var EventEmitter = require("events").EventEmitter;
var str = new EventEmitter();
var res = '';

function getNotifications(pHost, pPath) {
	// var http = require('http');
	var http = new http();
	//The url we want is: 'http://peony-curie.herokuapp.com/getPushNotifications'
	var options = {
		host: pHost,
		path: pPath,
		method: 'GET'
	};

	callback = function(response) {
		// var str = '';
		console.log(response.statusCode);
		response.setEncoding('utf8');
		console.log(response.data)
		//another chunk of data has been recieved, so append it to `str`
		response.on('data', function (chunk) {
			console.log('New data arriving!');
			str.data += chunk;
		});

		//the whole response has been recieved, so we just print it out here
		response.on('end', function () {
			//Send to Genuino
			console.log(str.data);
			console.log('End of request reached!');
			str.emit('update')
			// return str;
		});
	}

	var req = http.request(options, callback);
	req.end();
}

var notif = getNotifications('peony-curie.herokuapp.com', '/getPushNotifications')
str.on('update', function () {
    console.log(str.data); // HOORAY! THIS WORKS!
    res = str.data;
});

