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


    // Get panels effects
    getEffectsList("panels");
    getEffectsList("treads");
    getEffectsList("barrel");

    getActiveEffects();

    getSettings();

    // Global Vars
    var treadsSensorDriveValue = false;
    var treadsScreenSaverValue = false;
    var barrelScreenSaverValue = false;
    var panelsScreenSaverValue = false;
});


function getEffectsList(api)
{
    console.log("Getting list of effects");
    // Get panels effects
    $.getJSON("/api/" + api + "/effects", function (data, status) {
        console.log(data);
        displayEffectsList(data, api);
    });
}

function displayEffectsList(effects, api) {
    var html = "<ul>";

    for (var i = 0, len = effects.length; i < len; i++) {

        var name = effects[i].name;
        if (effects[i].isScreenSaver) name += "*";
        html += "<li onClick=\"selectEffect('" + effects[i].id + "','" + api + "')\">" + name + "</li>";
        console.log(effects[i]);
    }

    html += "</ul>";

    $("#" + api + "EffectsList").html(html);    
    //return html;
}

function displayActiveEffectData(data, api)
{
    if (data.argumentDescription == null)
        data.argumentDescription = "n/a"

    $("#" + api + "ArgumentDescription").html(data.argumentDescription);
    $("#" + api + "ArgumentValue").val(data.argument);
    $("#" + api + "ActiveEffect").text(data.name);
    
    if (data.isScreenSaver) {
        $("#" + api + "ScreenSaverButton").text("[X] SSAVER");
    }
    else {
        $("#" + api + "ScreenSaverButton").text("[_] SSAVER");
    }

    if (api == "treads") {
        if (data.isSensorDriven) {            
            $("#treadsAutoButton").text("[X] AUTOMATIC");
        }
        else {            
            $("#treadsAutoButton").text("[_] AUTOMATIC");
        }
        treadsSensorDriveValue = !data.isSensorDriven;
        treadsScreenSaverValue = !data.isScreenSaver;
    }
    else if (api == "barrel") {
        barrelScreenSaverValue = !data.isScreenSaver;
    }
    else if (api == "panels") {
        panelsScreenSaverValue = !data.isScreenSaver;
    }   
}


function getActiveEffects()
{
    console.log("Getting active effects");

    $.getJSON("/api/panels/effect", function (data, status) {
        displayActiveEffectData(data, "panels");
        console.log(data);
    });

    $.getJSON("/api/treads/effect", function (data, status) {
        displayActiveEffectData(data, "treads");
        console.log(data);
    });

    $.getJSON("/api/barrel/effect", function (data, status) {
        displayActiveEffectData(data, "barrel");
        console.log(data);
    });
}

function selectEffect(id, api) {
    $.post("/api/" + api + "/effect",
    {
        id: id
    },
    function (data, status) {
        displayActiveEffectData(data, api);
    });
}

function setEffectParameters(api, color, arg) {
    var clr = color.toString();

    $.post("/api/" + api + "/effect",
     {
         color: clr,
         argument: arg
     },
     function (data, status) {
         getActiveEffects();
     });
}

function setEffectColor(api, color) {
    var clr = color.toString();

    $.post("/api/" + api + "/effect",
     {
         color: clr,
     },
     function (data, status) {
         getActiveEffects();
     });
}

function setEffectArgument(api, arg) {
    $.post("/api/" + api + "/effect",
     {
         argument: arg
     },
     function (data, status) {
         getActiveEffects();
     });
}

function setEffectSensorDrive(api, arg) {
    $.post("/api/" + api + "/effect",
     {
         isSensorDriven: arg
     },
     function (data, status) {
         getActiveEffects();
     });
}

function setEffectScreenSaver(api, arg) {
    $.post("/api/" + api + "/effect",
     {
         isScreenSaver: arg
     },
     function (data, status) {
         getActiveEffects();
         getEffectsList(api);
     });
}


function getSettings() {
    console.log("Getting settings");
    // Get panels effects
    $.getJSON("/settings", function (data, status) {
        console.log(data);
        displaySettings(data);
    });
}

function displaySettings(data) {
    $("#settingsDmxBrightness").val(data.dmxBrightness);
    $("#settingsManualTick").val(data.manualTick);
    $("#settingsScreenSaverInterval").val(data.idleInterval);
}

function setDmxBrightness(arg) {
    $.post("/settings",
     {
         dmxBrightness: arg
     },
     function (data, status) {
         displaySettings(data);
     });
}

function setManualTick(arg) {
    $.post("/settings",
     {
         manualTick: arg
     },
     function (data, status) {
         displaySettings(data);
     });
}

function setScreenSaverInterval(arg) {
    $.post("/settings",
     {
         idleInterval: arg
     },
     function (data, status) {
         displaySettings(data);
     });
}
