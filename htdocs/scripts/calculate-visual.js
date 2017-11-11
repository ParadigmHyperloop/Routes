function genieIntro() {
	
	// Set a delay to show the big title
	setTimeout(showBigTitle, 700);

	
}

function showBigTitle() {
	
	$("#big-title").animate({opacity: '1'}, {duration: 1000});
	setTimeout(showGenie, 700);
	
}

function showGenie() {
	
	$("#genie-name").animate({opacity: '1'}, {duration: 1000});
	setTimeout(showSubtitle, 700);
	
}

function showSubtitle() {
	
	$("#little-title").animate({opacity: '1'}, {duration: 1000});
	setTimeout(showGene, 700);
	
}

function showGene() {
	
	$("#genetic").animate({opacity: '1'}, {duration: 1000});
	setTimeout(showRest, 700);
	
}

function showRest() {
	
	$("#sub-rest").animate({opacity: '1'}, {duration: 1000});
	setTimeout(showWhereTo, 1200);
	
}

function showWhereTo() {
	
	$("#where-to").animate({opacity: '1'}, {duration: 1000});
	
}