function makeLineGraph(canvas) {
    
    var ctx = document.getElementById(canvas).getContext('2d');
            
    return new Chart(ctx, {
        type: 'scatter',

        options: {

            scales: {
                xAxes: [{

                    ticks: {
                        fontFamily: "Rubik",
                        fontSize: 13
                    },

                    scaleLabel: {

                        display: true,
                        labelString: "Distance (m)",
                        fontFamily: "Rubik",
                        fontSize: 15
                    }

                }],

                yAxes: [{

                    ticks: {
                        fontFamily: "Rubik",
                        fontSize: 13
                    },

                    scaleLabel: {

                        display: true,
                        labelString: "Elevation (m)",
                        fontFamily: "Rubik",
                        fontSize: 15
                    }

                }]
            },

            legend: {
                display: false,
            }

        }


    });
    
}

function gradient(canvas) {
    
    var ctx = document.getElementById(canvas).getContext('2d');
    
    var gradient = ctx.createLinearGradient(0, 0, $("#" + canvas).width(), $("#" + canvas).height());
    gradient.addColorStop(0, '#FF5A43');   
    gradient.addColorStop(1, '#C00040');
           
    return gradient;
    
}