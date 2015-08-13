"use strict";

$(document).ready(function () {
	var canvas = document.getElementById('canvas');
	var width = 500;
	var height = 500;
	var ctx = canvas.getContext('2d');
	var img = ctx.createImageData(width, height);

	var ws = new WebSocket('ws://' + location.host);
	ws.binaryType = 'arraybuffer';
	
	ws.onmessage = function(event) {
		var colors = new Uint8ClampedArray(event.data);

		var g = {
			img: img,
			colors: colors,
			i: 0
		};

		var origin = [0, 0];
		var offset = [20, 20];

		var vertices = [
			[0   , 10 ],
			[50  , 25 ],
			[250 , 25 ],
			[265 , 15 ],
			[265 , 5  ],
			[250 , 0  ],
			[0   , 0  ]		
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
});

function plot(g, x, y) {
	var j = y*g.img.width*4 + x*4;
	var i = g.i % g.colors.byteLength;

	g.img.data[j+0] = g.colors[i+0] & 0x7f * 2;  // R
	g.img.data[j+1] = g.colors[i+1] & 0x7f * 2;  // G
	g.img.data[j+2] = g.colors[i+2] & 0x7f * 2;  // B
	g.img.data[j+3] = 0xff;                      // A

	g.i += 3;
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
