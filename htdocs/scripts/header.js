// When we scroll, add a drop shadow to the nav bar
$(window).on('scroll',function(){

    // Round because the code I copied from stackoverflow did
    var stop = Math.round($(window).scrollTop());

    // Check if we are passed 20pt, and if we are add the drop shadow
    if (stop > 20.0) 
        $('nav').addClass('nav-scrolled');
    else $('nav').removeClass('nav-scrolled');

});

// Void -> Void
// Goes to the route calculation screen
function goToRoutes() {
    
    window.location.href="route.html";
    
}