"use strict";

// Global Vars

var global = {
	treads: {
		auto: false,
		idle: false
	},
	barrel: {
		auto: false,
		idle: false
	},
	panels: {
		auto: false,
		idle: false
	}
};

$(document).ready(function () {
	window.cw = Raphael.colorwheel($("#rightContainer .colorwheel")[0], 300);
	cw.input($("#rightContainer input")[0]);
	cw.color("#FF0000");
	window.scrollTo(0, 1);

	$('#panelsTab').show();
	$('#treadsTab').hide();
	$('#barrelTab').hide();
	$('#settingsTab').hide();

	$('div.optPanels').click(function () {
		$('#panelsTab').show();
		$('#treadsTab').hide();
		$('#barrelTab').hide();
		$('#settingsTab').hide();
		$('div.optPanels').addClass("selectTab");
		$('div.optTreads').removeClass("selectTab");
		$('div.optBarrel').removeClass("selectTab");
		$('div.optSettings').removeClass("selectTab");
	});

	$('div.optTreads').click(function () {
		$('#panelsTab').hide();
		$('#treadsTab').show();
		$('#barrelTab').hide();
		$('#settingsTab').hide();
		$('div.optPanels').removeClass("selectTab");
		$('div.optTreads').addClass("selectTab");
		$('div.optBarrel').removeClass("selectTab");
		$('div.optSettings').removeClass("selectTab");
	});

	$('div.optBarrel').click(function () {
		$('#panelsTab').hide();
		$('#treadsTab').hide();
		$('#barrelTab').show();
		$('#settingsTab').hide();
		$('div.optPanels').removeClass("selectTab");
		$('div.optTreads').removeClass("selectTab");
		$('div.optBarrel').addClass("selectTab");
		$('div.optSettings').removeClass("selectTab");
	});

	$('div.optSettings').click(function () {
		$('#panelsTab').hide();
		$('#treadsTab').hide();
		$('#barrelTab').hide();
		$('#settingsTab').show();
		$('div.optPanels').removeClass("selectTab");
		$('div.optTreads').removeClass("selectTab");
		$('div.optBarrel').removeClass("selectTab");
		$('div.optSettings').addClass("selectTab");
	});

	$('.panel').click(function () {
		var index = $(this).attr('data-id');
		setEffect({ 
			kind: 'panels', 
			color: cw.color().hex.toString(), 
			argument: index 
		});
	});

	$('.colorOption').click(function() {
		cw.color($(this).attr('data-color'));
	});

	$('.brightness .button').click(function() {
		setSetting({ brightness: $(this).text() });
	});

	$('.button.manualTick').click(function() {
		setSetting({ manualTick: $('#manualTick').val() });
	});

	$('.button.idleInterval').click(function() {
		setSetting({ idleInterval: $('#idleInterval').val() });
	});

	$('.button.effect.argument').click(function() {
		var channel = $(this).attr('data-channel');
		var value = $('#' + channel + 'ArgumentValue').val();
		setEffect({ kind: channel, argument: value });
	});

	$('.button.effect.color').click(function() {
		var channel = $(this).attr('data-channel');
		setEffect({ kind: channel, color: cw.color().hex.toString() });
	});

	$('.button.effect.auto').click(function() {
		var channel = $(this).attr('data-channel');
		var value = global[channel].auto;
		setEffect({ kind: channel, sensor_driven: !value });
	});

	$('.button.effect.idle').click(function() {
		var channel = $(this).attr('data-channel');
		var value = global[channel].idle;
		setEffect({ kind: channel, screen_saver: !value });
	});

	getEffects();
	getSettings();
});

function displayEffect(kind, data) {
	displayEffectsList(kind, data.all);
	displayActiveEffect(kind, data.all[data.active]);
}

function displayEffectsList(kind, effects) {
	var html = "<ul>";

	for (var i = 0, len = effects.length; i < len; i++) {
		var name = effects[i].name;
		if (effects[i].screen_saver) 
			name += "*";
		html += "<li onClick=\"selectEffect('" + kind + "', " + i + ")\">" + name + "</li>";
	}

	html += "</ul>";

	$("#" + kind + "EffectsList").html(html);    
}

function intToRgb(raw) {
	return 'rgb(' + [
		(raw >> 16) & 0xff,
		(raw >> 8) & 0xff,
		raw & 0xff
	].join(',') + ')';
}

function displayActiveEffect(kind, data) {
	if (data.arg_desc == null) {
		data.arg_desc = "n/a"
	}

	$("#" + kind + "ArgumentDescription").html(data.arg_desc);
	$("#" + kind + "ArgumentValue").val(data.argument);
	$("#" + kind + "ActiveEffect").text(data.name);

	// TODO: display active colors

	var autoButton = $(".button.effect.auto." + kind);
	global[kind].auto = data.sensor_driven;
	if (data.sensor_driven) {
		autoButton.text("[X] AUTOMATIC");
	} else {            
		autoButton.text("[_] AUTOMATIC");
	}

	global[kind].idle = data.screen_saver;
	var idleButton = $(".button.effect.idle." + kind);
	if (data.screen_saver) {
		idleButton.text("[X] SSAVER");
	} else {
		idleButton.text("[_] SSAVER");
	}

	if (kind == "treads") {
	} else if (kind == "barrel") {
	} else if (kind == "panels") {
		for (var i = 0; i < data.color.length; i++) {
			var panel = $('.panel[data-id=' + i + ']');
			var raw = data.color[i];
			panel.css({ fill: intToRgb(raw) });
		}
	}   
}

function getEffects() {
	$.getJSON("/api/effects", function (data, status) {
		displayEffect("panels", data.panels);
		displayEffect("treads", data.treads);
		displayEffect("barrel", data.barrel);
	});
}

function setEffect(obj) {
	$.post("/api/effects", obj, function (data, status) { 
		displayEffect(obj.kind, data[obj.kind]); 
	});
}

function selectEffect(api, idx) {
	setEffect({ kind: api, active: idx });
}

function displaySettings(data) {
	$(".brightness .button").removeClass('active');
	$(".brightness .button:nth-child(" + data.brightness + ")").addClass('active');
	$("#manualTick").val(data.manualTick);
	$("#idleInterval").val(data.idleInterval);
}

function getSettings() {
	$.getJSON("/api/settings", function (data, status) {
		displaySettings(data);
	});
}

function setSetting(obj) {
	$.post("/api/settings", obj, function (data, status) { 
		displaySettings(data); 
	});
}
