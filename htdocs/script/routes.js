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

var route_id;

//0 if not showing graphs, 1 if showing
var toggle = 0;

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

var driveTime = 0;
var transitTime = 0;



function getCommuteTime(departLocation, arriveLocation, modeOfTransport) {    
    var queryURL = 
    "https://maps.googleapis.com/maps/api/distancematrix/json?units=imperial&origins=" + departLocation + "&destinations=" + arriveLocation + "&mode=" + modeOfTransport + "&key=" + keys.timeRoute;
    console.log(queryURL);
    
    
    $.ajax({
        url: queryURL,
        dataType: 'json',
        async: false,
        success: function(data) {
            callback(data, modeOfTransport);
        }
    });
    
}

function callback(data, modeOfTransport) {
    if (modeOfTransport == "driving") {
        driveTime = data.rows[0].elements[0].duration.value;
    } else if (modeOfTransport == "transit") {
        transitTime = data.rows[0].elements[0].duration.value;
    }    
}   

var sols = [];
var generation = 0;

var JSON_result;

var pathLine;
var points = [];

function gotFinishedRoute(result) {

    // Invalidate the timer and dismiss loading
    clearTimeout(saying_timer);
    
    //console.log(result);
    
    // Parse the JSON 
    JSON_result = JSON.parse(result);
    
    if (JSON.stringify(JSON_result.solutions) == "[[]]") {
        $(".dash").hide();
    }
    
    console.log(JSON_result);
    

    points = [];
    for (var i = 0; i < JSON_result.evaluated.length; i++) {
        points.push({lat: JSON_result.evaluated[i][1], lng:
                     JSON_result.evaluated[i][0]});

    }
    
    var time1 = fancyTimeFormat(JSON_result.timeForCurve);
    var length = fancyLengthFormat(JSON_result.distance);
    
    var elevations = [];
    var groundElevations = [];
    var speeds = [];
    
    for (var i = 0; i < JSON_result.elevations.length; i++) {
        elevations.push({x: JSON_result.elevations[i][0],
                         y: JSON_result.elevations[i][1]});
    }
    for (var i = 0; i < JSON_result.groundElevations.length; i++) {
        groundElevations.push({x: JSON_result.groundElevations[i][0],
                               y: JSON_result.groundElevations[i][1]});
    }
    for (var i = 0; i < JSON_result.speeds.length; i++) {
        speeds.push({x: JSON_result.speeds[i][0],
                     y: JSON_result.speeds[i][1]});
    }
    
    
    var chart = makeLineGraph("graph", "Distance (m)", "Elevation (m)")
    var chart1 = makeLineGraph("graph-1", "Distance (m)", "Speed (m/s)")
            
            chart.data.datasets[0] = {
                data: elevations,
                label: "Track",
                backgroundColor: "transparent",
                borderColor: gradient("graph", 1),
                pointRadius: 0,
                pointHitRadius: 0
            };
    
    
            chart.data.datasets[1] = {
                data:groundElevations,
                label: "Terrain",
                backgroundColor: "transparent",
                borderColor: gradient("graph", 2),
                pointRadius: 0,
                pointHitRadius: 0
            };
                
            chart.update()
            
            chart1.data.datasets[0] = {
                data: speeds,
                backgroundColor: "transparent",
                borderColor: gradient("graph", 1),
                pointRadius: 0,
                pointHitRadius: 0
                };
    
            chart1.options.legend.display = false;
            chart1.update()
    
    
    pathLine = new google.maps.Polyline({
        path: points,
        geodesic: true,
        strokeColor: '#FF0000',
        strokeOpacity: 1.0,
        strokeWeight: 4
    });
    
    var chart2 = makeBarGraph("graph-2",  ["Hyperloop", "Car", "Transit"], "Time of Travel")
    
    let start_lat = origin["lat"]
    let start_lng = origin["lng"]
    let dest_lat = destination["lat"]
    let dest_lng = destination["lng"]
    let start = start_lat + "," + start_lng
    let end = dest_lat + "," + dest_lng
    
    getCommuteTime(start, end, "driving");
    getCommuteTime(start, end, "transit");

        
    chart2.data.datasets[0] = {

        label: 'Travel Time',
        data: [JSON_result.timeForCurve, driveTime, transitTime],
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
    $('#map-container').css({"display": "flex", "opacity" : "0"})
    $('#overlay').css({"overflow-y": "scroll"})

    document.getElementById("time").innerHTML = time1;
    document.getElementById("distance").innerHTML = length;    
    
    numGenerations = JSON_result.solutions.length;
        
    for (var i = 0; i < numGenerations; i++) {
        sols[i] = [];
    }
    
    for (var i = 0; i < JSON_result.solutions.length; i++) {
        for (var j = 0; j < JSON_result.solutions[i].length; j++) {
            sols[i].push({lat: JSON_result.solutions[i][j][1],
                               lng: JSON_result.solutions[i][j][0]});
        }
    }
    
    updateMap();
}

function fadeOverlay() {
    
    $('#overlay').css({"opacity": 0})
    $('#map-container').css({"display": "flex", "opacity" : "100"})
    
    setTimeout(hideOverlay, 800)
    
    zoomToRoute();
    
}

function showRoute() {
    
    generation = numGenerations - 1;
    $('#left-map-graphs').css({"display": "flex", "opacity" : "0"})
    $('#right-map-graphs').css({"display": "flex", "opacity" : "0"})
    $('#dashboard-container').css({"opacity" : "0"});
    $('#dashboard-container').prop('disabled', true);
    $('#see-stats-container').css({"opacity" : "100"});
    fadeOverlay();
    updateMap();
    
}

function hideOverlay() {
    
     $('#overlay').css({"display": "none"})

}

function showStats() {
    $('#overlay').show()
    $('#overlay').css({"opacity": 100})
    $('#map-container').css({"display": "flex", "opacity" : "0"})
    $('#info-container').css({"display": "flex", "opacity" : "100"})
    
    pathLine.setMap(null);
    
    pathLine = new google.maps.Polyline({
        path: points,
        geodesic: true,
        strokeColor: '#FF0000',
        strokeOpacity: 1.0,
        strokeWeight: 4
    });
    
    setTimeout(function(){
        pathLine.setMap(map);
    }, 1500);
    
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
            } else {
                succ(result);
            } 

        }

    }

} 

function doc_keyUp(e) {

    //determine if the button is 'z'
    if (e.keyCode == 90) {
        zoomToRoute();
        console.log("z");
    } else if (e.keyCode == 39 && generation < numGenerations - 2) { //right arrow
        generation += 1;
        updateMap();
        addToGraphs();
        console.log("right");
    } else if (e.keyCode == 37 && generation > 0) { // left arrow
        generation += -1;
        updateMap();
        removeFromGraphs();
        console.log("left");   
    } else {
        updateMap();
    }
}
// register the handler 
document.addEventListener('keydown', doc_keyUp, false);

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

    var ret = "";

    if (hrs > 0) {
        ret += "" + hrs  + " hrs";
    }

    ret += "" + mins + " mins " + "" + secs.toFixed(0) + " secs";
    return ret;
}

function fancyLengthFormat(length) {   
    var km = ~~(length / 1000);
    var m = ~~(length % 1000)


    var ret = "";


    ret += "" + km.toFixed(2)  + "km ";

    return ret;
}

function startVisualization() {
    
    fadeOverlay();
    generation = 0;
    updateMap();
    initGraphs();
    $('#dashboard-container').css({ "opacity" : "100"});
    $('#dashboard-container').prop('disabled', false);
    $('#see-stats-container').css({ "opacity" : "100"});

}



var totalFitChart;
var trackFitChart;
var curveFitChart;
var gradeFitChart;
var lengthFitChart;


function initGraphs() {    
    
    totalFitChart = makeLineGraph("tot-fit-graph", "Generation", "Cost");
    trackFitChart = makeLineGraph("track-fit-graph", "Generation", "Cost");
    curveFitChart = makeLineGraph("curve-fit-graph", "Generation", "Cost");
    gradeFitChart = makeLineGraph("grade-fit-graph", "Generation", "Cost");
    lengthFitChart = makeLineGraph("length-fit-graph", "Generation", "Cost");

    
    var totals = [];

    for (var i = 0; i < generation; i++) {
        totals.push({x: i,
                     y: JSON_result.totalFitness[i]});
    }    
    

    
    totalFitChart.data.datasets[0] = {
        data: totals,
        label: "Cost",
        backgroundColor: "#191919",
        borderColor: gradient("graph", 1),
        pointRadius: 0,
        pointHitRadius: 0
    }
    
    totalFitChart.update();
    
    var tracks = [];

    for (var i = 0; i < generation; i++) {
        tracks.push({x: i,
                     y: JSON_result.trackFitness[i]});
    }    
    
    trackFitChart.data.datasets[0] = {
        data: tracks,
        label: "Cost",
        backgroundColor: "#191919",
        borderColor: gradient("graph", 2),
        pointRadius: 0,
        pointHitRadius: 0
    }
    
    trackFitChart.update();
    
    var curves = [];

    for (var i = 0; i < generation; i++) {
        curves.push({x: i,
                     y: JSON_result.curveFitness[i]});
    }    
    
    curveFitChart.data.datasets[0] = {
        data: curves,
        label: "Cost",
        backgroundColor: "#191919",
        borderColor: gradient("graph", 2),
        pointRadius: 0,
        pointHitRadius: 0
    }
    
    curveFitChart.update();
    
    var grades = [];

    for (var i = 0; i < generation; i++) {
        grades.push({x: i,
                     y: JSON_result.gradeFitness[i]});
    }    
    
    gradeFitChart.data.datasets[0] = {
        data: grades,
        label: "Cost",
        backgroundColor: "#191919",
        borderColor: gradient("graph", 2),
        pointRadius: 0,
        pointHitRadius: 0
    }
    
    gradeFitChart.update();
    
    var lengths = [];

    for (var i = 0; i < generation; i++) {
        lengths.push({x: i,
                     y: JSON_result.lengthFitness[i]});
    }    
    
    lengthFitChart.data.datasets[0] = {
        data: lengths,
        label: "Cost",
        backgroundColor: "#191919",
        borderColor: gradient("graph", 2),
        pointRadius: 0,
        pointHitRadius: 0
    }
    
    
    lengthFitChart.update();
    
}

function addToGraphs() {
    totalFitChart.data.datasets.forEach((dataset) => {
        dataset.data.push({x: generation,
                           y: JSON_result.totalFitness[generation]});
    });
    totalFitChart.update();
    
    trackFitChart.data.datasets.forEach((dataset) => {
        dataset.data.push({x: generation,
                           y: JSON_result.trackFitness[generation]});
    });
    trackFitChart.update();
    
    curveFitChart.data.datasets.forEach((dataset) => {
        dataset.data.push({x: generation,
                           y: JSON_result.curveFitness[generation]});
    });
    curveFitChart.update();
    
    gradeFitChart.data.datasets.forEach((dataset) => {
        dataset.data.push({x: generation,
                           y: JSON_result.gradeFitness[generation]});
    });
    gradeFitChart.update();
    
    lengthFitChart.data.datasets.forEach((dataset) => {
        dataset.data.push({x: generation,
                           y: JSON_result.lengthFitness[generation]});
    });
    lengthFitChart.update();
}

function removeFromGraphs() {
    totalFitChart.data.datasets.forEach((dataset) => {
        dataset.data.pop();
    });
    totalFitChart.update();
    
    trackFitChart.data.datasets.forEach((dataset) => {
        dataset.data.pop();
    });
    trackFitChart.update();
    
    curveFitChart.data.datasets.forEach((dataset) => {
        dataset.data.pop();
    });
    curveFitChart.update();
    
    gradeFitChart.data.datasets.forEach((dataset) => {
        dataset.data.pop();
    });
    gradeFitChart.update();
    
    lengthFitChart.data.datasets.forEach((dataset) => {
        dataset.data.pop();
    });
    lengthFitChart.update();
}

function updateMap() {
    
    pathLine.setMap(null);
    
    var newPath;
    
    if (JSON.stringify(JSON_result.solutions) == "[[]]") {
        newPath = points;
    } else {
        newPath = sols[generation];
    }
    
    pathLine = new google.maps.Polyline({
        path: newPath,
        geodesic: true,
        strokeColor: '#FF0000',
        strokeOpacity: 1.0,
        strokeWeight: 4
    });
        
    origin = JSON.parse(localStorage.getItem("origin"))
    destination = JSON.parse(localStorage.getItem("destination"))
    
    let start_lat = origin["lat"]
    let start_lng = origin["lng"]
    let dest_lat = destination["lat"]
    let dest_lng = destination["lng"]
    let start = start_lat + "," + start_lng
    let end = dest_lat + "," + dest_lng
    
    pathLine.setMap(map);
}


function showGraphs() {
    if (toggle == 0) {
        $('#left-map-graphs').css({"display": "flex", "opacity" : "100"})
        $('#right-map-graphs').css({"display": "flex", "opacity" : "100"})
        toggle = 1;
        $("see-graphs-text").update("Hide Graphs");
    } else {
        $('#left-map-graphs').css({"display": "flex", "opacity" : "0"})
        $('#right-map-graphs').css({"display": "flex", "opacity" : "0"})
        toggle = 0;
        $("see-graphs-text").update("Show Graphs");

    }
}























