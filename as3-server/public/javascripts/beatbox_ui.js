"use strict";
// Client-side interactions with the browser for web interface

// Make connection to server when web page is fully loaded.
var socket = io.connect();
// var volume = 50;
// var tempo = 50;
var communicationsTimeout = null;
$(document).ready(function() {
	setupServerMessageHandlers(socket);
	
	// Setup a repeating function (every 1s)
	window.setInterval(function() {sendCommandToServer('read-uptime')}, 1000);
	window.setInterval(function() {sendCommandToServer('recording')}, 1000);
	window.setInterval(function() {sendCommandToServer('deterrents')}, 1000);
	
	// Start off by "polling" the volume, mode, and tempo:
	sendCommandToServer('recording');
	sendCommandToServer('deterrents');
	
	// Setup the button clicks:
	$('#recordingOff').click(function() {
		sendCommandToServer('recording', "0");
	});
	$('#recordingOn').click(function() {
		sendCommandToServer('recording', "1");
	});

	$('#deterrentsOff').click(function() {
		sendCommandToServer('deterrents', "0");
	});

	$('#deterrentsOn').click(function() {
		sendCommandToServer('deterrents', "1");
	});

	$('#stop').click(function() {
		console.log("Terminating program");
		sendCommandToServer('stop', "0");
	});
});

var hideErrorTimeout;
function setupServerMessageHandlers(socket) {
	// Hide error display:
	$('#error-box').hide(); 
	
	socket.on('recording-reply', function(message) {
		console.log("Receive Reply: recording-reply " + message);
		var name = "Unknown!";
		switch(Number(message)) {
			case 0: name = "Off"; break;
			case 1: name = "On"; break;
		}
		$('#recordingId').text(name);
		clearServerTimeout();
	});

	socket.on('deterrents-reply', function(message) {
		console.log("Receive Reply: deterrents-reply " + message);
		var name = "Unknown!";
		switch(Number(message)) {
			case 0: name = "Off"; break;
			case 1: name = "On"; break;
		}
		$('#deterrentsId').text(name);
		clearServerTimeout();
	});

	
	socket.on('uptime-reply', function(message) {
		var times = message.split(" ");
		var seconds = Number(times[0]);
		
		var hours = Math.floor(seconds/60/60);
		var minutes = Math.floor((seconds / 60) % 60);
		seconds = Math.floor(seconds % 60);
		
		var display = "Device up for: " + hours + ":" + minutes + ":" + seconds + "(H:M:S)";
		
		$('#status').html(display);
		clearServerTimeout();
	});
	
	socket.on('beatbox-error', errorHandler);
}

function sendCommandToServer(command, options) {
	if (communicationsTimeout == null) {
		communicationsTimeout = setTimeout(errorHandler, 1000, 
				"ERROR: Unable to communicate to HTTP server. Is nodeJS server running?");
	}
	socket.emit(command, options);
}
function clearServerTimeout() {
	clearTimeout(communicationsTimeout);
	communicationsTimeout = null;
}

function errorHandler(message) {
	console.log("ERROR Handler: " + message);
	// Make linefeeds into <br> tag.
//	message = replaceAll(message, "\n", "<br/>");
	
	$('#error-text').html(message);	
	$('#error-box').show();
	
	// Hide it after a few seconds:
	window.clearTimeout(hideErrorTimeout);
	hideErrorTimeout = window.setTimeout(function() {$('#error-box').hide();}, 5000);
	clearServerTimeout();
}