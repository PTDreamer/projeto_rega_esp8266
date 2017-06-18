
$(document).ready(function(){
    setInterval(function(){getlog();}, 1000);
});

function getlog() {
    $('.panel-body').load("data/log.html");
}