var sayings = ["Adding that [Musk-y] smell", 
               "Proving [Darwin] right", 
               "Counting the [1's] and 0's",
               "[Elon-gating] the track", 
               "Tasting [victory]",
               "[Spinning] up the fans",
               "[404] route not found",
               "Dropping [production] tables",
               "[Mmmm]... bacon",
               "Finding [{] a friend",
               "[Re-thinking] decisions",
               "Forgetting something [really] important",
               "[Sucking] out the air",
               "Making [more] problems",
               "Hey look a [butterfly!]",
               "Swizzling some [vectors]",
               "Printing [\"Hello World\"]",
               "Finding the [missing] semicolon",
               "[Flipping] a few bits"]
var saying_timer
var current_saying


window.onload = function() {
    
    particleground(document.getElementById('background'), {
        
      minSpeedX: 0.2,
      maxSpeedX: 0.3,
      minSpeedY: 0.2,
      maxSpeedY: 0.3,
      density: 63000, 
      curvedLines: false,
      parallax: false,
      proximity: 250, 
      particleRadius: 4,
        
    }) 
    
    origin = JSON.parse(localStorage.getItem("origin"))
    destination = JSON.parse(localStorage.getItem("destination"))
    
    let start_lat = origin["lat"]
    let start_lng = origin["lng"]
    let dest_lat = destination["lat"]
    let dest_lng = destination["lng"]
    let start = start_lng+ "," + start_lat
    let end = dest_lng + "," + dest_lat
    
    $.ajax(getComputeRequest(start, end, handleIdentifier))
    zoomToRoute();
    
    initSayings()
    saying_timer = window.setTimeout(changeSaying, 500)
    
}

var index = 0
var loadedIndex = sayings.length
var offset = 0

function initSayings() {
    
    for (var i = 0; i < loadedIndex; i++) 
        $('#sayings').append("<div class=\"saying-row\">" + sayings[i].replace("[", "<div class=\"grad-text\">").replace("]", "</div>") + "</div>")

    $('#sayings').animate({scrollTop: 1}, 0)
    
}

var acc = 0;
function changeSaying() {
    
    offset = $('#sayings').children('div').get(index).offsetHeight
    $('#sayings').css({ "-webkit-transform" : "translate3d(0,-" + (acc + offset) + "px,0)"});
    acc += offset;
    index++
    
    $('#sayings').append("<div class=\"saying-row\">" + sayings[loadedIndex % sayings.length].replace("[", "<div class=\"grad-text\">").replace("]", "</div>") + "</div>")
    loadedIndex++
    
    saying_timer = window.setTimeout(changeSaying, 2000)

}
            
var ident = "";
function handleIdentifier(result) {
    
    // Save the identifier
    ident = result;

    // Set a timer 
    setTimeout(checkCompleted, 1000);
}

function checkCompleted() {

    $.ajax(getCheckRequest(ident, gotFinishedRoute, gotInProgress));

}

function gotInProgress() {

    // Set a timer 
    setTimeout(checkCompleted, 1000);

}



function gotFinishedRoute(result) {

    // Invalidate the timer and dismiss loading
    clearTimeout(saying_timer);

    // Parse the JSON 
    var JSON_result = JSON.parse(result);

    var points = [];
    for (var i = 0; i < JSON_result.evaluated.length; i++) {
        points.push({lat: JSON_result.evaluated[i][1], lng:
                     JSON_result.evaluated[i][0]});

    }
    
    var time1 = fancyTimeFormat(JSON_result.timeForCurve);
    var length = fancyLengthFormat(JSON_result.distance);
    
    var elevations = [];
    var speeds = [];
    
    for (var i = 0; i < JSON_result.elevations.length; i++) {
        elevations.push({x: JSON_result.elevations[i][0],
                         y: JSON_result.elevations[i][1]});
    }
    for (var i = 0; i < JSON_result.speeds.length; i++) {
        speeds.push({x: JSON_result.speeds[i][0],
                     y: JSON_result.speeds[i][1]});
    }
    
    var chart = makeLineGraph("graph", "Distance (m)", "Elevation (m)")
            var chart1 = makeLineGraph("graph-1", "Distance (m)", "Speed (m/s)")
            
            chart.data.datasets[0] = {
                data: elevations,
                backgroundColor: "transparent",
                borderColor: gradient("graph"),
                pointRadius: 0,
                pointHitRadius: 0
            };
            
            chart.update()
            
            chart1.data.datasets[0] = {
                data: speeds,
                
                backgroundColor: "transparent",
                borderColor: gradient("graph"),
                pointRadius: 0,
                pointHitRadius: 0
                };
    
    chart1.update()
    
    
    var pathLine = new google.maps.Polyline({
        path: points,
        geodesic: true,
        strokeColor: '#FF0000',
        strokeOpacity: 1.0,
        strokeWeight: 4
    });
    
    var chart2 = makeBarGraph("graph-2",  ["Hyperloop", "Car", "Transit"], "Time of Travel")
            
    chart2.data.datasets[0] = {

        label: 'Travel Time',
        data: [190, 2940, 8460],
        backgroundColor: [
            gradientW("graph-2", 250, 50),
            "#3c3c3c",
            "#3c3c3c",
            "#3c3c3c"
        ],
    }
            
  chart2.update()

    pathLine.setMap(map)
    $('#sayings-container').hide()
    $('#info-container').css({"display": "flex", "opacity" : "100"})
    $('#overlay').css({"overflow-y": "scroll"})
    
    document.getElementById("time").innerHTML = time1;
    document.getElementById("distance").innerHTML = length;    
}

function fadeOverlay() {
    
    $('#overlay').css({"opacity": 0})
    setTimeout(hideOverlay, 1000)
    
}

function hideOverlay() {
    
     $('#overlay').css({"display": "none"})

}

function getComputeRequest(start, dest, succ) {

    // Return the dictonary with the parameters filled in
    return {

        type: "GET",
        url: config.url + ":8080/compute?start=" + start + "&dest=" + dest,

        contentType: "text/plain",

        headers: { "Access-Control-Allow-Origin" : "*", },
        success: function(result) { succ(result) }
    }

}

function getCheckRequest(_ident, succ, fail) {

    return {

        type: "GET",
        url: config.url + ":8080/retrieve?id=" + _ident,

        contentType: "text/plain",

        headers: { "Access-Control-Allow-Origin" : "*", },

        success: function(result) { 

            if (result == "false"){
                console.log("failed to retrieve data from server" + ident)
                fail();
            }
            else succ(result);

        }

    }

}

function zoomToRoute() {
    
    let bounds = new google.maps.LatLngBounds();
    bounds.extend(origin)
    bounds.extend(destination)
    
    map.fitBounds(bounds);
    map.setCenter(bounds.getCenter());    
    
}

function fancyTimeFormat(time) {   
    // Hours, minutes and seconds
    var hrs = ~~(time / 3600);
    var mins = ~~((time % 3600) / 60);
    var secs = time % 60;

    // Output like "1:01" or "4:03:59" or "123:03:59"
    var ret = "";

    if (hrs > 0) {
        ret += "" + hrs  + " hrs";
    }

    ret += "" + mins + " mins " + "" + secs.toFixed(0) + " secs";
    return ret;
}

function fancyLengthFormat(length) {   
    // Hours, minutes and seconds
    var km = ~~(length / 1000);
    var m = ~~(length % 1000)


    // Output like "1:01" or "4:03:59" or "123:03:59"
    var ret = "";

    if (km > 0) {
        ret += "" + km  + "km ";
    }

    ret += "" + m + "m";
    return ret;
}