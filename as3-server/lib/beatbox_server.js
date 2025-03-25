"use strict";
/*
 * Respond to commands over a websocket to access the beat-box program
 */

var fs       = require('fs');
var socketio = require('socket.io');
var io; 
var dgram    = require('dgram');

exports.listen = function(server) {
	io = socketio.listen(server);
	io.set('log level 1');
	
	io.sockets.on('connection', function(socket) {
		handleCommand(socket);
	});
};

function handleCommand(socket) {
	console.log("Setting up socket handlers.");

	socket.on('read-uptime', function() {
		readAndSendFile(socket, '/proc/uptime', 'uptime-reply');
	});
	socket.on('mode', function(modeNumber) {
		console.log("Got mode command: " + modeNumber);
		relayToLocalPort(socket, "mode " + modeNumber, "mode-reply");
	});
	socket.on('volume', function(volumeNumber) {
		console.log("Got volume command: " + volumeNumber);
		relayToLocalPort(socket, "volume " + volumeNumber, "volume-reply");
	});
	socket.on('tempo', function(tempoNumber) {
		console.log("Got tempo command: " + tempoNumber);
		relayToLocalPort(socket, "tempo " + tempoNumber, "tempo-reply");
	});
	socket.on('play', function(songNumber) {
		console.log("Got play command: " + songNumber);
		relayToLocalPort(socket, "play " + songNumber, "play-reply");
	});
	socket.on('stop', function(notUsed) {
		console.log("Got stop command: ");
		relayToLocalPort(socket, "stop", "stop-reply");
	});
};

function readAndSendFile(socket, absPath, commandString) {
	fs.exists(absPath, function(exists) {
		if (exists) {
			fs.readFile(absPath, function(err, fileData) {
				if (err) {
					socket.emit("beatbox-error", 
							"ERROR: Unable to read file " + absPath);
				} else {
					// Don't send back empty files.
					if (fileData.length > 0) {
						socket.emit(commandString, fileData.toString('utf8'));;
					}
				}
			});
		} else {
			socket.emit("beatbox-error", 
					"ERROR: File " + absPath + " not found.");
		}
	});
}

function relayToLocalPort(socket, data, replyCommandName) {
	console.log('relaying to local port command: ' + data);
	
	// Info for connecting to the local process via UDP
	var PORT = 12345;	// Port of local application
	var HOST = '192.168.7.2';
	var buffer = new Buffer(data);

	// Send an error if we have not got a reply in a second
    var errorTimer = setTimeout(function() {
    	console.log("ERROR: No reply from local application.");
    	socket.emit("beatbox-error", "SERVER ERROR: No response from beat-box application. Is it running?");
    }, 1000);

	
	var client = dgram.createSocket('udp4');
	client.send(buffer, 0, buffer.length, PORT, HOST, function(err, bytes) {
	    if (err) 
	    	throw err;
	    console.log('UDP message sent to ' + HOST +':'+ PORT);
	});
	
	client.on('listening', function () {
	    var address = client.address();
	    console.log('UDP Client: listening on ' + address.address + ":" + address.port);
	});
	// Handle an incoming message over the UDP from the local application.
	client.on('message', function (message, remote) {
	    console.log("UDP Client: message Rx" + remote.address + ':' + remote.port +' - ' + message);
	    
	    var reply = message.toString('utf8')
	    socket.emit(replyCommandName, reply);
	    clearTimeout(errorTimer);
	    client.close();
	});
	
	client.on("UDP Client: close", function() {
	    console.log("closed");
	});
	client.on("UDP Client: error", function(err) {
	    console.log("error: ",err);
	});	
}