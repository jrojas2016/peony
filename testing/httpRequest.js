var http = require('http');

//The url we want is: 'http://peony-curie.herokuapp.com/getPushNotifications'
var options = {
	host: 'peony-curie.herokuapp.com',
	path: '/getPushNotifications',
	method: 'GET'
};

callback = function(response) {
	var str = '';
	console.log(response.statusCode);
	response.setEncoding('utf8');
	//another chunk of data has been recieved, so append it to `str`
	response.on('data', function (chunk) {
		console.log('New data arriving!');
		str += chunk;
	});

	//the whole response has been recieved, so we just print it out here
	response.on('end', function () {
		//Send to Genuino
		console.log(str);
		console.log('End of request reached!');
	});

}

var req = http.request(options, callback);

req.on('error', function(e) {
	console.log(e.message);
});

req.end();

