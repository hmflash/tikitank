"use strict";

// Global Vars

var cw;
var global = {
	manualToggle: false,
	screenSaverToggle: true,
	treads: {
		idle: false
	},
	barrel: {
		idle: false
	},
	panels: {
		idle: false
	}
};

$(document).ready(function () {
	cw = Raphael.colorwheel($("#rightContainer .colorwheel")[0], 300);
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

	$('.button.manualToggle').click(function() {
		var value = !global.manualToggle;
		setSetting({ manualToggle: value ? 1 : 0 });
	});

	$('.button.idleInterval').click(function() {
		setSetting({ idleInterval: $('#idleInterval').val() });
	});

	$('.button.screenSaverToggle').click(function() {
		var value = !global.screenSaverToggle;
		setSetting({ screenSaverToggle: value ? 1 : 0 });
	});

	$('.button.alpha').click(function() {
		setSetting({ alpha: $('#alpha').val() });
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

	$('.button.effect.idle').click(function() {
		var channel = $(this).attr('data-channel');
		var value = global[channel].idle;
		setEffect({ kind: channel, screen_saver: !value });
	});

	$('.button.resetDefaults').click(function() {
		resetDefaults();
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

function displaySettings(data) {
	$(".brightness .button").removeClass('active');
	$(".brightness .button:nth-child(" + data.brightness + ")").addClass('active');
	$("#manualTick").val(data.manualTick);
	$("#idleInterval").val(data.idleInterval);
	$("#alpha").val(data.alpha);
	global.manualToggle = data.manualToggle == 1;
	$("#manualToggle").text(data.manualToggle ? 'ON' : 'OFF');
	global.screenSaverToggle = data.screenSaverToggle == 1;
	$("#screenSaverToggle").text(data.screenSaverToggle ? 'ON' : 'OFF');
}

function displayActiveEffect(kind, data) {
	if (data.arg_desc == null) {
		data.arg_desc = "n/a"
	}

	$("#" + kind + "ArgumentDescription").html(data.arg_desc);
	$("#" + kind + "ArgumentValue").val(data.argument);
	$("#" + kind + "ActiveEffect").text(data.name);

	global[kind].idle = data.screen_saver;
	var idleButton = $(".button.effect.idle." + kind);
	if (data.screen_saver) {
		idleButton.text("[X] SSAVER");
	} else {
		idleButton.text("[_] SSAVER");
	}

	if (kind == "panels") {
		for (var i = 0; i < data.color.length; i++) {
			var panel = $('.panel[data-id=' + i + ']');
			var raw = data.color[i];
			panel.css({ fill: intToRgb(raw) });
		}
	} else {
		cw.color(intToRgb(data.color));
	}
}

function displayEffects(data) {
	displayEffect("panels", data.panels);
	displayEffect("treads", data.treads);
	displayEffect("barrel", data.barrel);
}

function getEffects() {
	$.getJSON("/api/effects", function (data, status) {
		displayEffects(data);
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

function resetDefaults() {
	$.post("/api/reset", null, function (data, status) { 
		displaySettings(data); 
		getEffects();
	});
}
