// String -> Void
// Smoothly scrolls to a div with the id that is passed in 
function smoothScrollToDiv(div) {
    
    var rowpos = $("#" + div).offset().top - 80;
    $("body").animate({ scrollTop: rowpos }, 600);
    
}