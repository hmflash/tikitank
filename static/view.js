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
	var mode = true; // true = treads, false = barrel
	var canvas = document.getElementById('canvas');
	var width = 300;
	var height = 50;
	var ctx = canvas.getContext('2d');
	var img = ctx.createImageData(width, height);

	ws = new WebSocket('ws://' + location.host);
	ws.binaryType = 'arraybuffer';

	ws.onmessage = function(event) {
		var g = {
			offset: [20, 5],
			img: img,
			colors: new Uint8ClampedArray(event.data),
			i: 0
		};

		if (mode) {
			renderTreads(g);
		} else {
			renderBarrel(g, 18);
			renderBarrel(g, 20);
			renderBarrel(g, 22);
			ctx.putImageData(img, 0, 0);
		}

		mode = !mode;
	}

	ws.onclose = function(event) {
		ctx.fillStyle = "black";
		ctx.fillRect(0, 0, width, height);
	}
}

function renderTreads(g) {
	var vertices = [
		[  80,  0 ],
		[   0,  0 ],
		[   0, 10 ],  //  10 px
		[  44, 40 ],  //  46 px
		[ 208, 40 ],  // 164 px
		[ 225, 25 ],  //  20 px
		[ 225, 15 ],  //  10 px
		[ 208,  0 ],  //  20 px
		[  80,  0 ]   // 128 px
	];

	renderPolygon(g, vertices);
}

function renderBarrel(g, y) {
	var vertices = [
		[ 156, y ],
		[  80, y ]
	];

	g.i = 0;

	renderPolygon(g, vertices);
}

function renderPolygon(g, vertices) {
	var prev = vertices.shift();
	for (var i = 0; i < vertices.length; i++) {
		var cur = vertices[i];
		line(g,
			prev[0]+g.offset[0], prev[1]+g.offset[1], 
			cur[0]+g.offset[0], cur[1]+g.offset[1]
		);
		prev = cur;
	}
	// console.log('i: ', g.i/3, colors.length/3);
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
	g.img.data[i+1] = getColor(g);  // G
	g.img.data[i+0] = getColor(g);  // R
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
