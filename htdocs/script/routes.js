var sayings = ["Adding that [Musk-y] smell", 
               "[Elon-gating] the track", 
               "Proving [Darwin] right", 
               "Counting the [1's] and 0's",
               "Wishing upon a [star]",
               "Praying to [Gaben]",
               "Tasting [victory]",
               "[404] track not found",
               "[Mmmm]... bacon",
               "[Sucking] out the air",
               "Hey look a [butterfly!]",
               "Funneling in [electrons]"]
var saying_timer
var current_saying


window.onload = function() {
    
    let query = window.location.href.slice(window.location.href.indexOf('?') + 1)
     $.ajax(getComputeRequest(query, handleIdentifier))
    changeSaying()
    
}

function changeSaying() {

    var new_saying;
    while (true) {

        new_saying = Math.floor((Math.random() * sayings.length))
        if (new_saying != current_saying)
            break;

    }

    current_saying = new_saying;

    $("#saying").html(sayings[new_saying].replace("[", "<div class=\"grad-text\">").replace("]", "</div>"))
    saying_timer = window.setTimeout(changeSaying, 2000)

}
            
var ident = "";
function handleIdentifier(result) {
    // Save the identifier
    ident = result;

    // Set a timer 
    //setTimeout(checkCompleted, 1000);
}

function getComputeRequest(query, succ) {

    // Return the dictonary with the parameters filled in
    return {

        type: "GET",
        url: config.url + ":8080/compute?" + query,

        contentType: "text/plain",

        headers: { "Access-Control-Allow-Origin" : "*", },
        success: function(result) { succ(result) }
    }

}