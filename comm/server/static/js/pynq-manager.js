/*
This script provides a PYNQ manager
*/

var PynqManager = (function(){
    "use strict";

    // Constructor
    var PynqManager = function(clients){

        if (!(this instanceof PynqManager)){
            return new PynqManager(clients);
        }

        this.clients = clients;
    }

    var _p = PynqManager.prototype;

    // Status check for clients
    _p.getStatus = function(){

        for (var key in this.clients){
            (function(_key, _client){
                var statusObj = $(".client-status-row[data-cname='"+_key+"']").find(".client-status-value").eq(0)
                $.ajax({
                    type: "POST",
                    url: "/status",
                    dataType: "json",
                    data: {
                        "client": _client
                    }
                }).done(function(res){
                    var answer = res["status"];
                    var message = "";

                    if (answer) {
                        message = answer;
                    }else{
                        message = "Illegal response: " + res;
                    }
                    statusObj.text(message);
                }).fail(function(){
                    statusObj.text("Connection error");
                });
            })(key, this.clients[key]);
        }

    }

    _p.sendQuestion = function(qname, after=null){

        $("#solving-question-name").text(qname);
        $("#solving-question-status").text('Processing');

        $.ajax({
            type: "POST",
            dataType: "json",
            url: "/start",
            data: {
                "qname": qname
            }
        }).done(function(res){
            var answer = res;
            $("#solving-question-status").text(answer["status"]);
            $("#solved-result").text(answer["answer"]["answer"]);
            $("#solved-client").text(answer["answer"]["client"]);

            if (after !== null){
                after();
            }
        });

    }

    _p.sendStop = function(){

        console.log("Not implemented!");

    }

    return PynqManager;
})();
