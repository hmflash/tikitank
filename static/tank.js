"use strict";

// Global Vars
var treadsSensorDriveValue = false;
var treadsScreenSaverValue = false;
var barrelScreenSaverValue = false;
var panelsScreenSaverValue = false;

$(document).ready(function () {
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
		if (effects[i].isScreenSaver) name += "*";
		html += "<li onClick=\"selectEffect('" + kind + "', " + i + ")\">" + name + "</li>";
	}

	html += "</ul>";

	$("#" + kind + "EffectsList").html(html);    
}

function displayActiveEffect(kind, data) {
	if (data.argumentDescription == null) {
		data.argumentDescription = "n/a"
	}

	$("#" + kind + "ArgumentDescription").html(data.argumentDescription);
	$("#" + kind + "ArgumentValue").val(data.argument);
	$("#" + kind + "ActiveEffect").text(data.name);

	// TODO: display active colors
	
	if (kind == "treads") {
		if (data.isSensorDriven) {            
			$("#treadsAutoButton").text("[X] AUTOMATIC");
		} else {            
			$("#treadsAutoButton").text("[_] AUTOMATIC");
		}

		if (data.isScreenSaver) {
			$("#" + kind + "ScreenSaverButton").text("[X] SSAVER");
		} else {
			$("#" + kind + "ScreenSaverButton").text("[_] SSAVER");
		}

		treadsSensorDriveValue = data.isSensorDriven;
		treadsScreenSaverValue = data.isScreenSaver;
	} else if (kind == "barrel") {
		if (data.isScreenSaver) {
			$("#" + kind + "ScreenSaverButton").text("[X] SSAVER");
		} else {
			$("#" + kind + "ScreenSaverButton").text("[_] SSAVER");
		}

		barrelScreenSaverValue = data.isScreenSaver;
	} else if (kind == "panels") {
		panelsScreenSaverValue = data.isScreenSaver;
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

function setEffectParameters(api, color, arg) {
	setEffect({ kind: api, color: color.toString(), argument: arg });
}

function setEffectColor(api, color) {
	setEffect({ kind: api, color: color.toString() });
}

function setEffectArgument(api, arg) {
	setEffect({ kind: api, argument: arg });
}

function setEffectSensorDrive(api, arg) {
	setEffect({ kind: api, isSensorDriven: arg });
}

function setEffectScreenSaver(api, arg) {
	setEffect({ kind: api, isScreenSaver: arg });
}

function displaySettings(data) {
	$("#settingsDmxBrightness").val(data.dmxBrightness);
	$("#settingsManualTick").val(data.manualTick);
	$("#settingsScreenSaverInterval").val(data.idleInterval);
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

function setDmxBrightness(arg) {
	setSetting({ dmxBrightness: arg });
}

function setManualTick(arg) {
	setSetting({ manualTick: arg });
}

function setScreenSaverInterval(arg) {
	setSetting({ idleInterval: arg });
}
