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
	window.setInterval(fetchAndUpdateVideoTable, 5000);
	
	// Send initial commands to server to get the current state:
	sendCommandToServer('recording');
	sendCommandToServer('deterrents');
	fetchAndUpdateVideoTable();
	
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


function fetchAndUpdateVideoTable() {
	console.log('calling this function periodically!\n');
	$.get('/api/videos', function(videoFiles) {
		console.log("videoFiles is: ", videoFiles, '\n');
		if(JSON.stringify(videoFiles) !== JSON.stringify(currentFilePathList)) {
			currentFilePathList = videoFiles;
			console.log('Updated video file list:', currentFilePathList, '\n');
			// update the HTML table
			const tableBody = $('#videoTableBody');
			tableBody.empty(); // Clear the table before updating

			videoFiles.forEach(video => {
				const timestamp = new Date(parseFloat(video.timestamp) * 1000).toLocaleString();
				const row = `
					<tr>
						<td>${video.filename}</td>
						<td>${timestamp}</td>
						<td><button type="button" onclick="playVideo('${video.filename}', '${video.timestamp}')">Play</button></td>
					</tr>
				`;
				tableBody.append(row);
			});
		}
	}).fail(function() {
		console.error('Failed to fetch video files\n');
	});
}

function playVideo(filename, timestamp) {
	const videoPlayer = document.getElementById('videoPlayer');
	const videoSource = document.getElementById('videoSource');
	videoSource.src = `/videos/${filename}`;
	videoPlayer.load();
	videoPlayer.play();

	// update the timestamp display
	const videoPlayerTimestamp = document.getElementById('videoPlayerTimestamp');
	const formattedTimestamp = new Date(parseFloat(timestamp) * 1000).toLocaleString();
	videoPlayerTimestamp.innerHTML = `Playback: ${formattedTimestamp}`;
}

let currentFilePathList = [];
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
	
	socket.on('purrhibition-error', errorHandler);
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