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


	// Global Vars
	var treadsSensorDriveValue = false;
	var treadsScreenSaverValue = false;
	var barrelScreenSaverValue = false;
	var panelsScreenSaverValue = false;

	getEffects();
	getSettings();
});

function displayEffect(data) {
	displayEffectsList(data);
	displayActiveEffect(data.kind, data.active);
}

function displayEffectsList(data) {
	var html = "<ul>";

	for (var i = 0, len = data.effects.length; i < len; i++) {
		console.log(data.effects[i]);
		var name = data.effects[i].name;
		if (effects[i].isScreenSaver) name += "*";
		html += "<li onClick=\"selectEffect('" + effects[i].id + "','" + data.kind + "')\">" + name + "</li>";
	}

	html += "</ul>";

	$("#" + data.kind + "EffectsList").html(html);    
}

function displayActiveEffect(kind, data) {
	if (data.argumentDescription == null) {
		data.argumentDescription = "n/a"
	}

	$("#" + data.kind + "ArgumentDescription").html(data.argumentDescription);
	$("#" + data.kind + "ArgumentValue").val(data.argument);
	$("#" + data.kind + "ActiveEffect").text(data.name);
	
	if (data.isScreenSaver) {
		$("#" + data.kind + "ScreenSaverButton").text("[X] SSAVER");
	} else {
		$("#" + data.kind + "ScreenSaverButton").html("[&nbsp;] SSAVER");
	}

	if (data.kind == "treads") {
		if (data.isSensorDriven) {            
			$("#treadsAutoButton").text("[X] AUTOMATIC");
		} else {            
			$("#treadsAutoButton").text("[_] AUTOMATIC");
		}
		treadsSensorDriveValue = !data.isSensorDriven;
		treadsScreenSaverValue = !data.isScreenSaver;
	} else if (data.kind == "barrel") {
		barrelScreenSaverValue = !data.isScreenSaver;
	} else if (data.kind == "panels") {
		panelsScreenSaverValue = !data.isScreenSaver;
	}   
}

function getEffects() {
	$.getJSON("/api/effects", function (data, status) {
		console.log(data);
		displayEffect(data.panels);
		displayEffect(data.treads);
		displayEffect(data.barrel);
	});
}

function setEffect(obj) {
	$.post("/api/effects", obj, function (data, status) { 
		displayEffect(data[obj.kind]); 
	});
}

function selectEffect(id, api) {
	setEffect({ kind: api, id: id });
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
	console.log("Getting settings");
	// Get panels effects
	$.getJSON("/api/settings", function (data, status) {
		console.log(data);
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
