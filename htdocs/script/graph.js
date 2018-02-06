function makeLineGraph(canvas, xAxis, yAxis) {
    
    var ctx = document.getElementById(canvas).getContext('2d');
            
    return new Chart(ctx, {
        type: 'scatter',

        options: {

            scales: {
                xAxes: [{

                    ticks: {
                        fontFamily: "Rubik",
                        fontSize: 13,
                        fontColor: "#fff"
                    },

                    scaleLabel: {

                        display: true,
                        labelString: xAxis,
                        fontFamily: "Rubik",
                        fontSize: 15,
                        fontColor: "#fff"
                    }

                }],

                yAxes: [{

                    ticks: {
                        fontFamily: "Rubik",
                        fontSize: 13,
                        fontColor: "#fff"
                    },

                    scaleLabel: {

                        display: true,
                        labelString: yAxis,
                        fontFamily: "Rubik",
                        fontSize: 15,
                        fontColor: "#fff"
                    }

                }]
            },

            legend: {
                display: false,
            }

        }


    });
    
}

function makeBarGraph(canvas, labels, yAxis) {
    
    var ctx = document.getElementById("graph-2").getContext('2d');
    return new Chart(ctx, {

        type: 'bar',

        data: {

            labels: labels
            
        },

        options: {

            scales: {

                xAxes: [{

                    ticks: {
                        fontFamily: "Rubik",
                        fontSize: 13,
                        fontColor: "#fff"
                    },

                    scaleLabel: {

                        display: true,
                        labelString: yAxis,
                        fontFamily: "Rubik",
                        fontSize: 15,
                        fontColor: "#fff"
                    }

                }],

                yAxes: [{

                    ticks: {

                        fontFamily: "Rubik",
                        fontSize: 13,
                        fontColor: "#fff",
                        beginAtZero:true
                    },

                    scaleLabel: {

                        display: true,
                        labelString: "Time (m)",
                        fontFamily: "Rubik",
                        fontSize: 15,
                        fontColor: "#fff"

                    }

                }]

            },

            legend: {

                display: false,

            }

        }
    });
    
}

function gradientW(canvas, width, height) {
    
    var ctx = document.getElementById(canvas).getContext('2d');
    
    var gradient = ctx.createLinearGradient(0, height, width, 0);
    gradient.addColorStop(0, '#FF5A43');   
    gradient.addColorStop(1, '#C00040');
           
    return gradient;
    
}

function gradient(canvas) {
    return gradientW(canvas, $("#" + canvas).width(), $("#" + canvas).height());
}
