window.onload = function() {

    origin      = new google.maps.places.Autocomplete((document.getElementById('origin')),      {types: ['geocode']});
    destination = new google.maps.places.Autocomplete((document.getElementById('destination')), {types: ['geocode']});
    
}

function compute() {
    
    let start_lat = origin.getPlace().geometry.location.lat()
    let start_lng = origin.getPlace().geometry.location.lng()
    let dest_lat = destination.getPlace().geometry.location.lat()
    let dest_lng = destination.getPlace().geometry.location.lng()
    let start = start_lng+ "," + start_lat
    let end = dest_lng + "," + dest_lat

    window.location.href = "route.html?start=" + start + "&dest=" + end;
    
}