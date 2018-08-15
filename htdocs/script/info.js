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
}
//
//document.getElementById('button').addEventListener('click', changeVisibility(), null);
//function changeVisibility()
//{
//    document.getElementById('divToHide').style.display = "none";
//    document.getElementById('divToShow').style.display = "block";
//}