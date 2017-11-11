var sayings = ["Adding that Musk-y smell", 
               "Elon-gating the track", 
               "Proving Darwin right", 
               "Counting the 1's and 0's",
               "Wishing upon a star",
               "Praying to Gaben",
               "Tasting victory",
               "404 track not found",
               "Mmmm... bacon",
               "Sucking out the air",
               "Hey look a butterfly!",
               "Conjuring Genie"];

var saying_timer;
var current_saying;
var map = null;

window.onload = onLoad()

function onLoad(){
    $("#loading_blob").hide(); 
	genieIntro();
}

function initMap() {
    var uluru = {lat: 34.0522, lng: -118.2437};
    map = new google.maps.Map(document.getElementById('map-holder'), {
        zoom: 3,
        center: uluru,
        disableDefaultUI: true,
        styles: [
            {
                "featureType": "poi.business",
                "stylers": [
                    {
                        "visibility": "off"
                    }
                ]
            },
            {
                "featureType": "poi.park",
                "elementType": "labels.text",
                "stylers": [
                    {
                        "visibility": "off"
                    }
                ]
            },
            {
                "featureType": "road",
                "stylers": [
                    {
                        "visibility": "off"
                    }
                ]
            }
        ]                
    });

    initAutocomplete();

}

function initAutocomplete(){
    autocomplete = new google.maps.places.Autocomplete((document.getElementById('autocomplete')),
                                                       {types: ['geocode']});
    autocomplete2 = new google.maps.places.Autocomplete((document.getElementById('autocomplete2')),
                                                        {types: ['geocode']});


}

function openNav(){
    $("#overlay").css("width","100%");
}

function changeSaying() {

    var new_saying;
    while (true) {

        new_saying = Math.floor((Math.random() * sayings.length));
        if (new_saying != current_saying)
            break;

    }

    current_saying = new_saying;

    $("#saying").html(sayings[new_saying]);

    saying_timer = window.setTimeout(changeSaying, 2000);

}

function getComputeRequest(start, dest, succ) {

    // Return the dictonary with the parameters filled in
    console.log("http://hyperroutes.duckdns.org:8080/compute?start=" + start + "&dest=" + dest)
    return {

        type: "GET",
        url: "http://hyperroutes.duckdns.org:8080/compute?start=" + start + "&dest=" + dest,

        contentType: "text/plain",

        headers: { "Access-Control-Allow-Origin" : "*", },

        success: function(result) { succ(result) }
    }

}

function getCheckRequest(_ident, succ, fail) {

    return {

        type: "GET",
        url: "http://hyperroutes.duckdns.org:8080/retrieve?id=" + _ident,

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

function gotFinishedRoute(result) {

    // Invalidate the timer and dismiss loading
    clearTimeout(saying_timer);
    $("#loading_blob").hide();

    // Parse the JSON 
    var JSON_result = JSON.parse(result);

    var points = [];
    for (var i = 0; i < JSON_result.evaluated.length; i++) {
        points.push({lat: JSON_result.evaluated[i][1], lng:
                     JSON_result.evaluated[i][0]})

    }

    var pathLine = new google.maps.Polyline({
        path: points,
        geodesic: true,
        strokeColor: '#FF0000',
        strokeOpacity: 1.0,
        strokeWeight: 4
    });

    pathLine.setMap(map);

}

function gotInProgress() {

    // Set a timer 
    setTimeout(checkCompleted, 1000);

}

function checkCompleted() {

    $.ajax(getCheckRequest(ident, gotFinishedRoute, gotInProgress));

}

var ident = "";
function handleIdentifier(result) {
    // Save the identifier
    ident = result;

    // Set a timer 
    setTimeout(checkCompleted, 1000);
}

function push() {
//    switch to map view
    zoomToLocation();
    $("#loading_blob").show();
    hideOverlay();
    
//    format start and end for the api call
    let s_lat = autocomplete.getPlace().geometry.location.lat()
    let s_lng = autocomplete.getPlace().geometry.location.lng()
    let e_lat = autocomplete2.getPlace().geometry.location.lat()
    let e_lng = autocomplete2.getPlace().geometry.location.lng()
    let s = s_lng + "," + s_lat
    let e = e_lng + "," + e_lat

    $.ajax(getComputeRequest(s,e,handleIdentifier));

    changeSaying();
}

function hideOverlay(){
    $("#overlay").css("width","0%");
}

function zoomToLocation(){
    var bounds = new google.maps.LatLngBounds();
    bounds.extend(autocomplete.getPlace().geometry.location)
    bounds.extend(autocomplete2.getPlace().geometry.location)
    map.fitBounds(bounds);
    map.setCenter(bounds.getCenter());    
}