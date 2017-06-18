
$(document).ready(function(){
    getSchedules();
    setInterval(function(){periodicEvents();}, 1000);
});
function getSchedules() {
    $.ajax({
        url: "data/schedules.json",
        dataType: 'json',
        type: 'get',
        contentType: 'application/json',
        processData: false,
        success: function( data, textStatus, jQxhr ){
            console.log(data.pump_delay);
            $("#pumpdelay").val(data.pump_delay);
            $.each(data.schedules, function(schedule) {
                console.log(this.zone);
                var newRow = $("#rowtemplate").clone(true);
                newRow.removeAttr("id");
                newRow.appendTo($("#tablebody"));
                if(this.zone == "A") {
                    newRow.find(".zone").val("Zona A");
                } else if(this.zone == "B") {
                    newRow.find(".zone").val("Zona B");
                }
                newRow.find(".timepickeri").timepicker({showMeridian: false});
                newRow.find(".timepickeri").timepicker('setTime',this.hour + ":" + this.minute);
                newRow.find(".duration").val(this.duration);
                console.log(this.hour + ":" + this.minute);
                newRow.show();
            });
        },
        error: function( jqXhr, textStatus, errorThrown ){
            console.log( errorThrown );
            showMessage("error", "Data fetching failed. Server apeers to be down");
        }
    })
}
function saveSchedules(data) {
    $.ajax({
        url: "data/schedules.json",
        dataType: 'json',
        type: 'post',
        data: JSON.stringify(data),
        contentType: 'application/json',
        processData: false,
        success: function( data, textStatus, jQxhr ){
            if(data.status == "success") {
                showMessage("success", "Data saved successfully");
            } else {
                showMessage("error", "An error ocorred while saving data");
            }
        },
        error: function( jqXhr, textStatus, errorThrown ){
            console.log( errorThrown );
            showMessage("error", "Data saving failed. Server apeers to be down");
        }
    })
}
$('#newline').click(function(e) {
    var newRow = $("#rowtemplate").clone(true);
    newRow.appendTo($("#tablebody"));
    newRow.find(".timepickeri").timepicker({showMeridian: false});
    newRow.removeAttr("id");
    newRow.show();
});
$('.delete').click(function(e) {
    $(this).closest("tr").remove();
});

///TEST BUTTONS
$('#zonaa').click(function(e) {
    handleOutputClicks($(this), "A");
});
$('#zonab').click(function(e) {
    handleOutputClicks($(this), "B");
});
$('#pump').click(function(e) {
    handleOutputClicks($(this), "pump");
});
    
function handleOutputClicks(button, zone) {
    var data = {};
    if(button.attr('status') == "on") {
        data.zone = zone;
        data.state = "off";
    } else {
        data.zone = zone;
        data.state = "on";
    }
    $.ajax({
        dataType: "json",
        url: "data/getoutputs.json",
        data: data
    });
}
$('#save').click(function(e) {
    var hasError = false;
    var sch = {
        "schedules" : [],
        "pump_delay" : 0
        };
    $("#tablebody").find(".timepicker").each(function (index) {
        console.log("save");
        if($(this).closest("tr").attr("id") != "rowtemplate") {
            var regexp = /([01][0-9]|[02][0-3]):[0-5][0-9]/;
            var correct = ($(this).val().search(regexp) >= 0) ? true : false;
            var correct = true;
            if(!correct) {
                hasError = true;
                $(this).closest(".time").addClass("has-error");
            } else {
                $(this).closest(".time").removeClass("has-error");
            }
        }
    });
    $("#tablebody").find(".duration").each(function (index) {
        console.log("save");
        if($(this).closest("tr").attr("id") != "rowtemplate") {
            var correct = (($(this).val() >= 1) && ($(this).val() < 60)) ? true : false;
            console.log(correct);
            if(!correct) {
                hasError = true;
                $(this).closest(".durationparent").addClass("has-error");
            } else {
                $(this).closest(".durationparent").removeClass("has-error");
            }
        }
    });
    if(hasError) {
        showMessage("error", "Existem erros nos dados que introduziu")
    } else {
        sch.pump_delay = $("#pumpdelay").val();
        sch.schedules = [];
        $("#tablebody").find("tr").each(function (index) {
            if($(this).attr("id") != "rowtemplate") {
                var record = {
                            "hour" : 0,
                            "minute" : 0,
                            "duration" : 0,
                            "zone" : "A"};
                record.hour = $(this).find(".timepicker").val().split(":")[0];
                record.minute = $(this).find(".timepicker").val().split(":")[1];
                record.duration = $(this).find(".duration").val();
                if($(this).find(".zone").val() == "Zona A") {
                    record.zone = "A";
                } else {
                    record.zone = "B";
                }
                sch.schedules.push(record);
            }
        });
        saveSchedules(sch);
    }
});

function showMessage(type, text) {
    var boldText;
    var classTxt;
    if(type == "warning") {
        boldText = "Warning";
        classTxt = alert-warning;
    } else if(type == "error") {
        boldText = "Error";
        classTxt = "alert-danger";
    } else if(type == "info") {
        boldText = "Information";
        classTxt = "alert-info"
    } else if(type == "success") {
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
function periodicEvents() {
    getTime();
    getOutputs();
}
function getTime() {
    $.getJSON("data/gettime.json",
        function (json) {
            $("#timedisplay").text(json.hour + ":" + json.minute + ":" + json.seconds);
        });
}
function getOutputs() {
    $.getJSON("data/getoutputs.json", function (json) {
        if(json.zoneA == "on") {
            $("#zonaa").text("Desligar Zona A");
            $("#zonaa").attr("status", "on");
        } else if(json.zoneA == "off") {
            $("#zonaa").text("Ligar Zona A");
            $("#zonaa").attr("status", "off");
        }
        if(json.zoneB == "on") {
            $("#zonab").text("Desligar Zona B");
            $("#zonab").attr("status", "on");
        } else if(json.zoneB == "off") {
            $("#zonab").text("Ligar Zona B");
            $("#zonab").attr("status", "off");
        }
        if(json.pump == "on") {
            $("#pump").text("Desligar Bomba");
            $("#pump").attr("status", "on");
        } else if(json.zoneA == "off") {
            $("#pump").text("Ligar Bomba");
            $("#pump").attr("status", "off");
        }
    });
}