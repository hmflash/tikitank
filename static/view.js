"use strict";

// $(document).ready(function () {
// 	var canvas = document.getElementById('canvas');
// 	var width = 250;
// 	var height = 250;
// 	var ctx = canvas.getContext('2d');

// 	var img = ctx.createImageData(width, height);

// 	var ws = new WebSocket('ws://' + location.host);
// 	ws.binaryType = 'arraybuffer';
	
// 	ws.onopen = function(event) {
// 		console.log('onopen');
// 	};

// 	ws.onmessage = function(event) {
// 		var view = new Uint8ClampedArray(event.data);
// 		for (var i = 0; i < view.byteLength / 3; i++) {
// 			img.data[i*4+0] = view[i*3+1];
// 			img.data[i*4+1] = view[i*3+0];
// 			img.data[i*4+2] = view[i*3+2];
// 			img.data[i*4+3] = 0xff;
// 		}
	
// 		ctx.putImageData(img, 0, 0);
// 	}
// });

$(document).ready(function () {
	var canvas = document.getElementById('canvas');
	var width = 480;
	var height = 1;
	var ctx = canvas.getContext('2d');

	var pcanvas = document.createElement('canvas');
	pcanvas.width = width;
	pcanvas.height = height;
	var pctx = pcanvas.getContext('2d');
	var img = pctx.createImageData(width, height);

	var ws = new WebSocket('ws://' + location.host);
	ws.binaryType = 'arraybuffer';
	
	ws.onopen = function(event) {
		console.log('onopen');
	};

	ws.onmessage = function(event) {
		var view = new Uint8ClampedArray(event.data);
		for (var i = 0; i < view.byteLength / 3; i++) {
			img.data[i*4+0] = view[i*3+1];
			img.data[i*4+1] = view[i*3+0];
			img.data[i*4+2] = view[i*3+2];
			img.data[i*4+3] = 0xff;
		}
		pctx.putImageData(img, 0, 0);

		var pattern = ctx.createPattern(pcanvas, 'repeat');

		var origin = [10, 10];

		var vertices = [
			[0, 10],
			[50, 25],
			[250, 25],
			[265, 15],
			[265, 5],
			[250, 0],
			[0, 0]		
		];

		ctx.beginPath();
		ctx.strokeStyle = pattern;
		ctx.moveTo(origin[0], origin[1]);
		for (var i = 0; i < vertices.length; i++) {
			ctx.lineTo(vertices[i][0] + origin[0], vertices[i][1] + origin[1]);
		}
		ctx.stroke();
	}
});
