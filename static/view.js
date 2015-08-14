"use strict";

var ws;
var showPreview = false;

$(document).ready(function () {
	$('#treadsPreviewButton').click(function() {
		if (showPreview)
			stopPreview();
		else
			startPreview();

		showPreview = !showPreview;
		$('#canvas').toggle(showPreview);
	});
});

function startPreview() {
	var canvas = document.getElementById('canvas');
	var width = 300;
	var height = 50;
	var ctx = canvas.getContext('2d');
	var img = ctx.createImageData(width, height);

	ws = new WebSocket('ws://' + location.host);
	ws.binaryType = 'arraybuffer';
	
	ws.onmessage = function(event) {
		var colors = new Uint8ClampedArray(event.data);

		var g = {
			img: img,
			colors: colors,
			i: 0
		};

		var origin = [ 0, 0];
		var offset = [20, 5];

		var vertices = [
			[   0, 10 ],  //  10 px
			[  44, 34 ],  //  45 px
			[ 210, 34 ],  // 165 px
			[ 226, 22 ],  //  20 px
			[ 226, 12 ],  //  10 px
			[ 210,  0 ],  //  20 px
			[   0,  0 ]   // 210 px (14*5 + 14*10)
		];

		var prev = origin;
		for (var i = 0; i < vertices.length; i++) {
			var cur = vertices[i];
			line(g,
				prev[0]+offset[0], prev[1]+offset[1], 
				cur[0]+offset[0], cur[1]+offset[1]
			);
			prev = cur;
		}

		ctx.putImageData(img, 0, 0);
	}

	ws.onclose = function(event) {
		ctx.fillStyle = "black";
		ctx.fillRect(0, 0, width, height);
	}
}

function stopPreview() {
	ws.close();
}

function getColor(g) {
	if (g.i < g.colors.byteLength) {
		var i = g.i;
		g.i += 1;
		return (g.colors[i] & 0x7f) * 2;
	}
	return 0x00;
}

function plot(g, x, y) {
	var i = y*g.img.width*4 + x*4;
	g.img.data[i+0] = getColor(g);  // R
	g.img.data[i+1] = getColor(g);  // G
	g.img.data[i+2] = getColor(g);  // B
	g.img.data[i+3] = 0xff;         // A
}

function line(g, x0, y0, x1, y1) {
	var dx = Math.abs(x1 - x0);
	var sx = x0 < x1 ? 1 : -1;
	var dy = Math.abs(y1 - y0);
	var sy = y0 < y1 ? 1 : -1;
	var err = (dx > dy ? dx : -dy)/2;

	while (true) {
		plot(g, x0, y0);

		if (x0 === x1 && y0 === y1)
			break;

		var e2 = err;

		if (e2 > -dx) {
			err -= dy;
			x0 += sx;
		}

		if (e2 < dy) {
			err += dx;
			y0 += sy;
		}
	}
}
