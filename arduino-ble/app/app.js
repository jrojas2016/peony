// JavaScript code for the Arduino BLE example app.

/**
 * The BLE plugin is loaded asynchronously so the ble
 * variable is set in the onDeviceReady handler.
 */
var ble = null;

$(document).ready( function()
{
	// Adding event listeners for GPS buttons
	app.watchPosition();
});

/**
 * Application object that holds data and functions used by the app.
 */
var app =
{
	// Discovered devices.
	knownDevices: {},

	// Reference to the device we are connecting to.
	connectee: null,

	// Handle to the connected device.
	deviceHandle: null,

	// Handles to characteristics and descriptor for reading and
	// writing data from/to the Arduino using the BLE shield.
	characteristicRead: null,
	characteristicWrite: null,
	descriptorNotification: null,

	// Data that is plotted on the canvas.
	dataPoints: [],

	initialize: function()
	{
		document.addEventListener(
			'deviceready',
			function() { evothings.scriptsLoaded(app.onDeviceReady) },
			false);
	},

	displayStatus: function(status)
	{
		if(document.getElementById('status').innerHTML == status)
			return;
		console.log('Status: '+status);
		document.getElementById('status').innerHTML = status
	},

	// Called when device plugin functions are ready for use.
	onDeviceReady: function()
	{
		ble = evothings.ble; // Evothings BLE plugin

		app.startScan();
	},

	startScan: function()
	{
		app.displayStatus('Scanning...');
		evothings.ble.startScan(
			function(deviceInfo)
			{
				if (app.knownDevices[deviceInfo.address])
				{
					return;
				}
				console.log('found device: ' + deviceInfo.name);
				app.knownDevices[deviceInfo.address] = deviceInfo;
				if (deviceInfo.name == 'ARDUINO 101-3E4E' && !app.connectee)
				{
					console.log('Found Peony');
					connectee = deviceInfo;
					console.log('Peony Address: ' + deviceInfo.address);
					app.connect(deviceInfo.address);
				}
			},
			function(errorCode)
			{
				app.displayStatus('startScan error: ' + errorCode);
			});
	},

	connect: function(address)
	{
		evothings.ble.stopScan();
		app.displayStatus('Connecting...');
		evothings.ble.connect(
			address,
			function(connectInfo)
			{
				// console.log("Device Handle: " + connectInfo.deviceHandle);
				if (connectInfo.state == 2) // Connected
				{
					app.deviceHandle = connectInfo.deviceHandle;
					app.getServices(connectInfo.deviceHandle);
				}
				else
				{
					app.displayStatus('Disconnected');
				}
			},
			function(errorCode)
			{
				app.displayStatus('connect error: ' + errorCode);
			});
	},

	on: function()
	{
		app.write(
			'writeCharacteristic',
			app.deviceHandle,
			app.characteristicWrite,
			new Uint8Array([1])); // 1 = on
	},

	off: function()
	{
		app.write(
			'writeCharacteristic',
			app.deviceHandle,
			app.characteristicWrite,
			new Uint8Array([0])); // 0 = off
	},

	sendSMSAlert: function (position) {
		var toNum = document.getElementById('contact-num').value;
		var longitude = position.coords.longitude;
		var latitude = position.coords.latitude;
		$.ajax({ type: "GET",
			 url: 'http://peony-curie.herokuapp.com/sendSMSAlert',   
			 async: false,
			 dataType: "json",
			 data: {to_num: toNum, lat: latitude, long: longitude},
			 complete : function(xhr, text)
			 {
				console.log("SMS Sent Successfully!");
			 },
			 error: function(err){
				console.log("SMS Alert Failed: " + err);
			 }
		});
	},

	sendLocation: function () {
		var options = {
			enableHighAccuracy: true,
			maximumAge: 3600000
		}
		
		var watchID = navigator.geolocation.getCurrentPosition(onSuccess, onError, options);

		function onSuccess(position) {
			app.sendSMSAlert(position)
		};

		function onError(error) {
			alert('code: '    + error.code    + '\n' + 'message: ' + error.message + '\n');
		}
	},

	watchPosition: function () {
		var options = {
			maximumAge: 3600000,
			timeout: 60000,
			enableHighAccuracy: true,
		}

		var watchID = navigator.geolocation.watchPosition(onSuccess, onError, options);

		function onSuccess(position) {

			document.getElementById('location').src = "https://www.google.com/maps/embed/v1/place?key=AIzaSyDReMHfmJ6dNucdWrJTcDZEOROs_mApZzc&q=" + 
			position.coords.latitude + "," + position.coords.longitude;
		};

		function onError(error) {
			alert('code: '    + error.code    + '\n' +'message: ' + error.message + '\n');
		}
	},

	write: function(writeFunc, deviceHandle, handle, value)
	{
		if (handle)
		{
			ble[writeFunc](
				deviceHandle,
				handle,
				value,
				function()
				{
					console.log(writeFunc + ': ' + handle + ' success.');
				},
				function(errorCode)
				{
					console.log(writeFunc + ': ' + handle + ' error: ' + errorCode);
				});
		}
	},

	startReading: function(deviceHandle)
	{
		app.displayStatus('Enabling notifications...');

		// Turn notifications on.
		app.write(
			'writeDescriptor',
			deviceHandle,
			app.descriptorNotification,
			new Uint8Array([1,0]));

		// Start reading notifications.
		evothings.ble.enableNotification(
			deviceHandle,
			app.characteristicRead,
			function(data)
			{
				app.displayStatus('Active');
				app.changeButtonState(data);
			},
			function(errorCode)
			{
				app.displayStatus('enableNotification error: ' + errorCode);
			});
	},

	changeButtonState: function (dataValue) {
		var parsedData = new DataView(dataValue);	//Bluetooth sends a byte array; converting to int
		if (!parsedData.getInt8(0)) {            
			console.log("Button State: " + parsedData.getInt8(0));
			app.sendLocation();
			document.getElementById('buttonstate').className = "green";
		}
		else {
			console.log("Button state: " + parsedData.getInt8(0));
			document.getElementById('buttonstate').className = "black";
		}
	},

	getServices: function(deviceHandle)
	{
		app.displayStatus('Reading services...');

		evothings.ble.readAllServiceData(deviceHandle, function(services)
		{
			// Find handles for characteristics and descriptor needed.
			for (var si in services)
			{
				var service = services[si];

				for (var ci in service.characteristics)
				{
					var characteristic = service.characteristics[ci];

					if (characteristic.uuid == '19b10012-e8f2-537e-4f6c-d104768a1214')
					{
						app.characteristicRead = characteristic.handle;
					}
					else if (characteristic.uuid == '19b10011-e8f2-537e-4f6c-d104768a1214')
					{
						app.characteristicWrite = characteristic.handle;
					}

					for (var di in characteristic.descriptors)
					{
						var descriptor = characteristic.descriptors[di];

						if (characteristic.uuid == '19b10012-e8f2-537e-4f6c-d104768a1214' &&
							descriptor.uuid == '00002902-0000-1000-8000-00805f9b34fb')
						{
							app.descriptorNotification = descriptor.handle;
						}
					}
				}
			}

			if (app.characteristicRead && app.characteristicWrite && app.descriptorNotification)
			{
				console.log('RX/TX services found.');
				app.startReading(deviceHandle);
			}
			else
			{
				app.displayStatus('ERROR: RX/TX services not found!');
			}
		},
		function(errorCode)
		{
			app.displayStatus('readAllServiceData error: ' + errorCode);
		});
	},

	openBrowser: function(url)
	{
		window.open(url, '_system', 'location=yes')
	}
};
// End of app object.

// Initialise app.
app.initialize();
