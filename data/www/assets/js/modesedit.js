//GET data/getmode.php?id=XX retorna modeType
//PUT data/savemode.php manda modeType
//GET data/modesensors.json?id=XX retorna array {"id" : "0" ,"name":"a", "disabled":0},
(function($) {
    $.fn.goTo = function() {
        $('html, body').animate({
            scrollTop: $(this).offset().top - 40 + 'px'
        }, 'fast');
        return this; // for chaining...
    }
})(jQuery);
var sab = 1 << 6;
var dom = 1 << 5;
var seg = 1 << 4;
var ter = 1 << 3;
var qua = 1 << 2;
var qui = 1 << 1;
var sex = 1 << 0;

var modeType = {
		autoStart : "",
		autoEnd : "",
		startTime : "",
		endTime : "",
		armsSystem : "",
		disarmsSystem : "",
		preAlarmTime : "",
		maxAlarms : "",
		disabledSensors : [],
		name : "",
		paused : false,
		active : false,
		activedays : 0
	}
var allSensors;
var backup;
$('.timepickeri').timepicker({showMeridian: false});

function getModes(id) {
	$.ajax({
	    url: 'data/getmode.json',
	    context : {id : id},
	    dataType: 'json',
	    type: 'get',
  		//data: { id: id },
	    contentType: 'application/json',
	    processData: false,
	    success: function( data, textStatus, jQxhr ){
	    	for (var i = 0; i < data.length; i++) {
	        loadMode(data[i].id, data[i]);
	    }
	    },
	    error: function( jqXhr, textStatus, errorThrown ){
	        console.log( errorThrown );
	        showMessage("error", "Data fetching failed. Server apeers to be down");
	    }
})}

function loadMode(id,mode) {
	console.log(mode);
	var modeItem;
	if($("#" + id).length == 0) {
		modeItem = $("#modeTemplate").clone(true);
		modeItem.attr("id", id);
		modeItem.appendTo($("#modeTemplate").closest(".container"));
		console.log("appended");
	}
    else {
		modeItem = $("#" + id);
    }
    if(mode.active == true) {
    	modeItem.attr("status", "active"); 
    } else if(mode.paused == true) {
    	modeItem.attr("status", "paused");
    } else {
    	modeItem.attr("status", "readOnly");
    }
    if(id == "new") {
    	modeItem.attr("status", "editable");
    }
	modeItem.find(".autoStart").prop("checked", mode.autoStart);
	if(mode.autoStart) {
		modeItem.find(".autoStart").closest(".mainBody").find(".panel-body").first().show();
	} else {
		modeItem.find(".autoStart").closest(".mainBody").find(".panel-body").first().hide();
	}
	modeItem.find(".autoEnd").prop("checked", mode.autoEnd);
	if(mode.autoEnd) {
		modeItem.find(".autoEnd").closest(".mainBody").find(".panel-body").first().show();
	} else {
		modeItem.find(".autoEnd").closest(".mainBody").find(".panel-body").first().hide();
	}
	modeItem.find(".startTime").timepicker('setTime', mode.startTime);
	modeItem.find(".endTime").timepicker('setTime', mode.endTime);
	modeItem.find(".armsSystem").prop("checked", mode.armsSystem);
	modeItem.find(".disarmsSystem").prop("checked", mode.disarmsSystem);
	modeItem.find(".preAlarmTime").val(mode.preAlarmTime);
	modeItem.find(".maxAlarms").val(mode.maxAlarms);
	modeItem.find(".modeNameLabel").text(mode.name);
	var hsab = (mode.activedays & sab) != 0;
	var hdom = (mode.activedays & dom) != 0;
	var hseg = (mode.activedays & seg) != 0;
	var hter = (mode.activedays & ter) != 0;
	var hqua = (mode.activedays & qua) != 0;
	var hqui = (mode.activedays & qui) != 0;
	var hsex = (mode.activedays & sex) != 0;
	modeItem.find(".repeat-sabado").prop("checked", hsab);
	modeItem.find(".repeat-domingo").prop("checked", hdom);
	modeItem.find(".repeat-segunda").prop("checked", hseg);
	modeItem.find(".repeat-terca").prop("checked", hter);
	modeItem.find(".repeat-quarta").prop("checked", hqua);
	modeItem.find(".repeat-quinta").prop("checked", hqui);
	modeItem.find(".repeat-sexta").prop("checked", hsex);
	loadModeSensors(id, mode.disabledSensors);
	modeItem.show();
	if(id == "new") {
		modeItem.goTo();
	}
}
function htmlToMode(id) {
var modeItem;
	var mode = Object.create(modeType);
	modeItem = $("#" + id);
	mode.autoStart = modeItem.find(".autoStart").prop("checked");
	mode.autoEnd = modeItem.find(".autoEnd").prop("checked");
	mode.startTime = modeItem.find(".startTime").val();
	mode.endTime = modeItem.find(".endTime").val();
	mode.armsSystem = modeItem.find(".armsSystem").prop("checked");
	mode.disarmsSystem = modeItem.find(".disarmsSystem").prop("checked");
	mode.preAlarmTime = modeItem.find(".preAlarmTime").val();
	mode.maxAlarms = modeItem.find(".maxAlarms").val();
	mode.disabledSensors = [];
	var sensors = modeItem.find(".sensorTableBody").find(":checked").closest("tr").each(function (index) {
		mode.disabledSensors.push($(this).attr("id"));
	});
	mode.name = modeItem.find(".modeNameInput").val();
	var days = 0;
	if(modeItem.find(".repeat-sabado").prop("checked") == true) {
		days = days + sab;
	}
	if(modeItem.find(".repeat-domingo").prop("checked") == true) {
		days = days + dom;
	}
	if(modeItem.find(".repeat-segunda").prop("checked") == true) {
		days = days + seg;
	}
	if(modeItem.find(".repeat-terca").prop("checked") == true) {
		days = days + ter;
	}
	if(modeItem.find(".repeat-quarta").prop("checked") == true) {
		days = days + qua;
	}
	if(modeItem.find(".repeat-quinta").prop("checked") == true) {
		days = days + qui;
	}
	if(modeItem.find(".repeat-sexta").prop("checked") == true) {
		days = days + sex;
	}
	mode.activedays = days;
	return mode;
}
function saveMode(id) {
	var mode = htmlToMode(id);
	$.ajax({
	    url: 'data/savemode.php',
	    context : {id : id},
	    dataType: 'json',
	    type: 'post',
	    contentType: 'application/json',
	    data: JSON.stringify( mode ),
	    processData: false,
	    success: function( data, textStatus, jQxhr ){
	    	if(data.result == "success") {
	    		var modeItem = $("#" + id);
	    		modeItem.find(".modeNameLabel").text(modeItem.find("modeNameInput").val());
	    		showMessage("success", "Saving Operation succeded!");
	    		setModeItemMode(this.id, "readOnly");
	    	}
	    	else {
	    		showMessage("error", "Ooops something went wrong while saving");
	    		loadMode(this.id, backup);
	    	}
	    },
	    error: function( jqXhr, textStatus, errorThrown ){
	        console.log( errorThrown );
	        showMessage("error", "Saving Operation failed. Server apeers to be down");
	        loadMode(this.id, backup);
	    }
});
}

function getAllSensors() {
	$.ajax({
  		url: 'data/sensorlist.json',
  		//data: { id: id },
  		dataType: 'json',
  		success: function( resp ) {
    		allSensors = resp;
    		getModes("all");
}
});
}

function loadModeSensors(id, disabledSensors) {
	var tr;
	var table = $("#" + id).find(".sensorTableBody");
	table.find("tr").remove();
	for (var i = 0; i < allSensors.length; i++) {
		tr = $("<tr id=sensor" + allSensors[i].id + ">");
		tr.append("<td>" + allSensors[i].name + "</td>");
		if((disabledSensors.indexOf(allSensors[i].id)) != -1) {
			tr.append('<td><input class="control-label editable" type="checkbox" value="" checked></td>');
		} else {
			tr.append('<td><input class="control-label editable" type="checkbox" value=""></td>');
		}
		table.append(tr);
	}
	setModeItemMode(id, $("#" + id).attr("status"));
}

function setModeItemMode(id, mode) {
	modeItem = $("#" + id);
	modeItem.attr("status", mode);
	if(mode != "paused") {
		modeItem.find(".pauseUnpauseMode").html('<span class="glyphicon glyphicon-pause"></span>Pause');
	} else {
		modeItem.find(".pauseUnpauseMode").html('<span class="glyphicon glyphicon-play"></span>Play');
	}
	if(mode == "editable") {
		modeItem.find(".editMode").html('<span class="glyphicon glyphicon-save"></span>Gravar');
	 	modeItem.find(".deleteMode").html('<span class="glyphicon glyphicon-thumbs-down"></span>Cancelar');
		modeItem.find(".pauseUnpauseMode").attr("disabled", true);
		modeItem.find(".sensorTable").find("*").show();
		modeItem.find(".modeNameLabel").hide();
		modeItem.find(".modeNameInput").removeClass("hidden");
		modeItem.find(".modeNameInput").val(modeItem.find(".modeNameLabel").text());
		backup = htmlToMode(id);
	} else {
		modeItem.find(".editMode").html('<span class="glyphicon glyphicon-edit"></span>Editar');
		modeItem.find(".deleteMode").html('<span class="glyphicon glyphicon-remove-sign"></span>Apagar');
		modeItem.find(".pauseUnpauseMode").attr("disabled", false);
		modeItem.find(".sensorTableBody").find("tr").hide();
		modeItem.find(".sensorTableBody").find("input:checked").parent("tr").show();
		modeItem.find(".sensorTable").find(".editable").hide();
		modeItem.find(".sensorTableBody").find(":checked").closest("tr").show();
		modeItem.find(".modeNameLabel").show();
		modeItem.find(".modeNameInput").addClass("hidden");
	}
	if(mode == "editable") {
		modeItem.find("input").prop("disabled", false);
	} else {
		modeItem.find("input").prop("disabled", true);
	}
	modeItem.removeClass("panel-primary");
	modeItem.removeClass("panel-default");
	modeItem.removeClass("panel-success");
	modeItem.removeClass("panel-warning");
	if(mode == "readOnly") {
		modeItem.addClass("panel-primary");
	} else if(mode == "editable") {
		modeItem.addClass("panel-warning");
	} else if(mode == "paused") {
		modeItem.addClass("panel-default");
	} else if(mode == "active") {
		modeItem.addClass("panel-success");
	}
}
$('#newMode').click(function(e) {
var mode = {
		name : "New Mode",
		autoStart : false,
		autoEnd : false,
		startTime : "",
		endTime : "",
		armsSystem : "",
		disarmsSystem : "",
		preAlarmTime : "",
		maxAlarms : "",
		disabledSensors : [],
		name : "",
		paused : false,
		active : false,
		activedays : 0
	}
	loadMode("new", mode);
});

$('.deleteMode').click(function(e) {
	var modeItem = $(this).closest(".modeItem");
	var id = modeItem.attr("id");
	var status = modeItem.attr("status");
	if(status == "editable") {
		$(".modeItem").each(function (index) {
			if($(this).attr("id") != id) {
				$(this).find("button").attr("disabled", false);
			}
		});
		if(id == "new") {
			modeItem.remove();
		} else {
			loadMode(id, backup);
			setModeItemMode(id, "readOnly");
		}
	} else {
		$("#confirm-forcedmode .modal-text").text("Tem a certeza que quer apagar o modo selecionado?");
	    $("#confirm-forcedmode .modal-title").text("Confirme operacao");
	    $("#confirm-forcedmode .btn-ok").attr('recordId', id);
	    $("#confirm-forcedmode .btn-ok").text("Sim");
	    $("#confirm-forcedmode .btn-default").attr('recordId', id);
		$("#confirm-forcedmode .btn-default").text("Nao");
	    $("#confirm-forcedmode").attr('type', "deleteMode");
	    $("#confirm-forcedmode").modal("show");
	}
});
$('.pauseUnpauseMode').click(function(e) {
	var modeItem = $(this).closest(".modeItem")
	var id = modeItem.attr("id");
	if(modeItem.attr("status") == "paused") {
		$("#confirm-forcedmode .modal-text").text("Tem a certeza que quer retirar o modo selecionado de pausa?");
		$("#confirm-forcedmode").attr('type', "unpause");
	}
	else {
		$("#confirm-forcedmode .modal-text").text("Tem a certeza que quer colocar o modo selecionado em pausa?");
		$("#confirm-forcedmode").attr('type', "pause");
	}
	$("#confirm-forcedmode .modal-title").text("Confirme operacao");
	$("#confirm-forcedmode .btn-ok").attr('recordId', id);
	$("#confirm-forcedmode .btn-ok").text("Sim");
	$("#confirm-forcedmode .btn-default").attr('recordId', id);
	$("#confirm-forcedmode .btn-default").text("Nao");
	$("#confirm-forcedmode").modal("show");
});
$('.editMode').click(function(e) {
	var modeItem = $(this).closest(".modeItem");
	var id = modeItem.attr("id");
	var status = modeItem.attr("status");
	if(status == "editable") {
		$(".modeItem").each(function (index) {
			if($(this).attr("id") != id) {
				$(this).find("button").attr("disabled", false);
			}
		});
		console.log("start ajax call for saving");
		saveMode(id);
	}
	else {
		$(".modeItem").each(function (index) {
			if($(this).attr("id") != id) {
				$(this).find("button").attr("disabled", true);
			}
		});
		setModeItemMode(id, "editable");
	}
	console.log("ID= "+ id);
});

$(document).ready(function(){
	$("#serveralert").hide();
	getAllSensors();
});

function showMessage(type, text) {
	var boldText;
	var classTxt;
	if(type == "warning") {
		boldText = "Warning";
		classTxt = alert-warning;
	}
	else if(type == "error") {
		boldText = "Error";
		classTxt = "alert-danger";
	}
	else if(type == "info") {
		boldText = "Information";
		classTxt = "alert-info"
	}
	else if(type == "success") {
		boldText = "Success"
		classTxt = "alert-success";
	}
	var html = $('<div class="alert ' + classTxt + '"><strong>' + boldText + '</strong>' + '  ' + text + '</div>');
	$(".maincontainer").prepend(html);
	$('html, body').animate({scrollTop: '0px'}, 500);
	html.delay(5000).fadeOut(500);
	setTimeout(function() {
  		html.remove();
		}, 10000);
}

$('#confirm-forcedmode').on('click', '.btn-ok', function(e) {
	var id = $(this).attr('recordId');
	var $modalDiv = $(e.delegateTarget);
	if($("#confirm-forcedmode").attr("type") == "deleteMode") {
		$modalDiv.addClass('loading');
		$.post('data/deletemode.php', {id : id}, function() {
			$modalDiv.modal('hide').removeClass('loading');
			showMessage("success", "Modo removido com sucesso");
			$("#" + id).remove();
		})
		.fail(function() {
			$modalDiv.modal('hide').removeClass('loading');
    		showMessage("error", "erro ao remover modo" );
  		})
	} else if($("#confirm-forcedmode").attr("type") == "pause" || $("#confirm-forcedmode").attr("type") == "unpause" ) {
		$modalDiv.addClass('loading');
		var op = $("#confirm-forcedmode").attr("type") == "pause";
		$.post('data/pausemode.php', {id : id, pause : op}, function() {
			$modalDiv.modal('hide').removeClass('loading');
			showMessage("success", "operacao concluida com sucesso");
			$("#" + id).remove();
		})
		.fail(function() {
			$modalDiv.modal('hide').removeClass('loading');
    		showMessage("error", "erro ao realizar operacao" );
  		})
	}
});