
$(document).ready(function(){
    setInterval(function(){getstatus();}, 1000);
});

function getstatus() {
$.getJSON("data/getsystemstatus.json",
    function (json) {
        $("#heap").val(json.heap);
        $("#analog").val(json.analog);
        $("#gpio").val(json.gpio);
        $("#fstotalbytes").val(json.fstotalbytes);
        $("#usedBytes").val(json.fsusedbytes);
        $("#blockSize").val(json.fsblocksize);
        $("#pageSize").val(json.fspagesize);

        }
    );
}