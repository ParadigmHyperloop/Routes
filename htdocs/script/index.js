window.onload = function() {

    origin      = new google.maps.places.Autocomplete((document.getElementById('origin')),      {types: ['geocode']})
    destination = new google.maps.places.Autocomplete((document.getElementById('destination')), {types: ['geocode']})

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
    
}

function compute() {
        
    localStorage.setItem("origin",      JSON.stringify(origin.getPlace().geometry.location));
    localStorage.setItem("destination", JSON.stringify(destination.getPlace().geometry.location));

    window.location.href = "route.html";
        
    
}

function supports_html5_storage() {
  try {
    return 'localStorage' in window && window['localStorage'] !== null;
  } catch (e) {
    return false;
  }
}